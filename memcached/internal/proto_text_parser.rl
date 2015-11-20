#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#include <module.h>

#include "memcached.h"
#include "constants.h"
#include "utils.h"
#include "error.h"

#include "proto_text.h"
#include "proto_text_parser.h"

%%{
	machine memcached_text_parser;
	write data;
}%%

static inline const char *
skip_line(const char *begin, const char *end)
{
	for (; begin < (end - 1) &&*begin != '\r' && *(begin + 1) != '\n'; ++begin);
	return begin;
}

int
memcached_text_parser(struct memcached_connection *con,
					  const char **p_ptr, const char *pe)
{
	const char *p = *p_ptr;
	int cs = 0;
	const char *s = NULL;
	bool done = false;

	struct memcached_text_request *req = &con->request;
	memset(req, 0, sizeof(struct memcached_text_request));

	%%{
		action key_start {
			s = p;
			for (; p < pe && *p != ' ' && *p != '\r' && *p != '\n'; p++);
			if (*p == ' ' || *p == '\r' || *p == '\n') {
				if (req->key == NULL)
					req->key = s;
				req->key_len = (p-- - req->key);
				req->key_count += 1;
			} else {
				p = s;
			}
		}
		action read_data {
			req->data = p;
			req->data_len = req->bytes;

			if (req->data + req->data_len <= pe - 2) {
				if (strncmp(req->data + req->data_len, "\r\n", 2) != 0) {
					/**
					 * IDK what to do - skip it or not
					 */
					memcached_error_EINVALS("malformed data (can't find \r\n "
											"at the end of the query)");
					con->close_connection = true;
					return -1;
				}
				p += req->bytes + 2;
			} else {
				return (req->data_len + 2) - (pe - req->data);
			}
		}
		action done {
			done = true;
		}
		printable = [^ \t\r\n];
		key = printable >key_start ;

		exptime = digit+
				>{ s = p; }
				%{ memcached_strtoul(s, p, &req->exptime); };
		flags = digit+
				>{ s = p; }
				%{ memcached_strtoul(s, p, &req->flags); };
		bytes = digit+
				>{ s = p; }
				%{ memcached_strtoul(s, p, &req->bytes); };
		cas_value = digit+
				>{ s = p; }
				%{ memcached_strtoul(s, p, &req->cas); };
		incr_value = digit+
				>{ s = p; }
				%{ memcached_strtoul(s, p, &req->delta); };
		flush_delay = digit+
				>{ s = p; }
				%{ memcached_strtoul(s, p, &req->exptime); };

		eol = ("\r\n" | "\n") @{ p++; };
		spc = " "+;
		noreply = (spc "noreply"i %{ req->noreply = true; })?;

		store_body = spc key spc flags spc exptime spc bytes				noreply spc? eol;
		cas_body   = spc key spc flags spc exptime spc bytes spc cas_value	noreply spc? eol;
		get_body   = (spc key)+														spc? eol;
		del_body   = spc key (spc exptime)?									noreply spc? eol;
		cr_body    = spc key spc incr_value									noreply spc? eol;
		flush_body = (spc flush_delay)?									 	noreply spc? eol;
		verb_body  = spc flush_delay									 	noreply spc? eol;

		set		= ("set"i		 %~{req->op = MEMCACHED_TXT_CMD_SET;}	  store_body) @read_data @done;
		add		= ("add"i		 %~{req->op = MEMCACHED_TXT_CMD_ADD;}	  store_body) @read_data @done;
		replace = ("replace"i	 %~{req->op = MEMCACHED_TXT_CMD_REPLACE;} store_body) @read_data @done;
		append	= ("append"i	 %~{req->op = MEMCACHED_TXT_CMD_APPEND;}  store_body) @read_data @done;
		prepend = ("prepend"i	 %~{req->op = MEMCACHED_TXT_CMD_PREPEND;} store_body) @read_data @done;
		cas		= ("cas"i		 %~{req->op = MEMCACHED_TXT_CMD_CAS;}	  cas_body)   @read_data @done;

		get		= ("get"i		 %~{req->op = MEMCACHED_TXT_CMD_GET;}	 get_body) @done;
		gets	= ("gets"i		 %~{req->op = MEMCACHED_TXT_CMD_GETS;}	 get_body) @done;
		delete	= ("delete"i	 %~{req->op = MEMCACHED_TXT_CMD_DELETE;} del_body) @done;
		incr	= ("incr"i		 %~{req->op = MEMCACHED_TXT_CMD_INCR;}	 cr_body)  @done;
		decr	= ("decr"i		 %~{req->op = MEMCACHED_TXT_CMD_DECR;}	 cr_body)  @done;

		version   = ("version"i   %~{req->op = MEMCACHED_TXT_CMD_VERSION;}	) eol		 @done;
		verbosity = ("verbosity"i %~{req->op = MEMCACHED_TXT_CMD_VERBOSITY;}) verb_body  @done;
		stats	  = ("stats"i	  %~{req->op = MEMCACHED_TXT_CMD_STATS;}	) eol		 @done;
		flush_all = ("flush_all"i %~{req->op = MEMCACHED_TXT_CMD_FLUSH;}	) flush_body @done;
		quit	  = ("quit"i	  %~{req->op = MEMCACHED_TXT_CMD_QUIT;}		) eol		 @done;

		main := set | add | replace | append | prepend | cas |
				get | gets | delete | incr | decr |
				version | verbosity | stats | flush_all | quit;

		write init;
		write exec;
	}%%

	if (req->bytes > MEMCACHED_MAX_SIZE) {
		memcached_error_E2BIG();
		done = false;
	}
	if (!done) {
		if (p == pe) {
			return 1;
		}
		if (box_error_last() == NULL) {
			if (con->request.op == MEMCACHED_TXT_CMD_UNKNOWN) {
				const char *name = *p_ptr;
				int32_t len = 0;
				while (1) {
					char c = *(name + len);
					if (c == ' ' || c == '\n' || c == '\r')
						break;
					++len;
				};
				memcached_error_txt_UNKNOWN_COMMAND(*p_ptr, len);
			} else {
				memcached_error_EINVALS("bad command line format");
			}
			if (!con->close_connection) {
				const char *request = *p_ptr;
/*				switch(req->op) {
					case(MEMCACHED_TXT_CMD_SET):
					case(MEMCACHED_TXT_CMD_ADD):
					case(MEMCACHED_TXT_CMD_REPLACE):
					case(MEMCACHED_TXT_CMD_APPEND):
					case(MEMCACHED_TXT_CMD_PREPEND):
					case(MEMCACHED_TXT_CMD_CAS):
						request = skip_line(request, pe);
						request = skip_line(request, pe);
						break;
					case(MEMCACHED_TXT_CMD_GET):
					case(MEMCACHED_TXT_CMD_GETS):
					case(MEMCACHED_TXT_CMD_DELETE):
					case(MEMCACHED_TXT_CMD_INCR):
					case(MEMCACHED_TXT_CMD_DECR):
					case(MEMCACHED_TXT_CMD_STATS):
					case(MEMCACHED_TXT_CMD_FLUSH):
					case(MEMCACHED_TXT_CMD_QUIT):
						request = skip_line(request, pe);
						break;
					default:
						request = skip_line(request, pe);
						break;
				} */
				request = skip_line(request, pe);
				if ((request == pe - 2) && (*request != '\r' || *(request + 1) != '\n'))
					return 1;
				*p_ptr = (request + 2);
				con->noprocess = true;
			}
		} else if (box_error_code(box_error_last()) == box_error_code_MAX +
													   MEMCACHED_RES_E2BIG) {
			*p_ptr = (p - 1);
		}
		return -1;
	}
	*p_ptr = (p - 1);
	return 0;
}

/* vim: set ft=ragel noexpandtab ts=4 : */
