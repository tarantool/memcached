#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#include <tarantool.h>

#include "memcached.h"
#include "constants.h"
#include "utils.h"
#include "error.h"

#include "proto_text_parser.h"

%%{
	machine memcached_text_parser;
	write data;
}%%

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
				%{
					if (memcached_strtoul(s, p, &req->exptime) == -1) {
						memcached_error_EINVALS("bad expiration time value");
						con->close_connection = true;
						return -1;
					}
				};
		flags = digit+
				>{ s = p; }
				%{
					if (memcached_strtoul(s, p, &req->flags) == -1) {
						memcached_error_EINVALS("bad flags value");
						con->close_connection = true;
						return -1;
					}
				};
		bytes = digit+
				>{ s = p; }
				%{
					if (memcached_strtoul(s, p, &req->bytes) == -1) {
						memcached_error_EINVALS("bad bytes value");
						con->close_connection = true;
						return -1;
					} else if (req->bytes > MEMCACHED_MAX_SIZE) {
						memcached_error_E2BIG();
						con->close_connection = true;
						return -1;
					}
				};
		cas_value = digit+
				>{ s = p; }
				%{
					if (memcached_strtoul(s, p, &req->cas) == -1) {
						memcached_error_EINVALS("bad cas value");
						con->close_connection = true;
						return -1;
					}
				};
		incr_value = digit+
				>{ s = p; }
				%{
					if (memcached_strtoul(s, p, &req->inc_val) == -1) {
						memcached_error_DELTA_BADVAL();
						// memcached_error_EINVALS("bad incr/decr value");
						con->close_connection = true;
						return -1;
					}
				};
		flush_delay = digit+
				>{ s = p; }
				%{
					if (memcached_strtoul(s, p, &req->exptime) == -1) {
						memcached_error_EINVALS("bad flush value");
						con->close_connection = true;
						return -1;
					}
				};

		eol = ("\r\n" | "\n") @{ p++; };
		spc = " "+;
		noreply = (spc "noreply"i %{ req->noreply = true; })?;

		store_body = spc key spc flags spc exptime spc bytes				noreply spc? eol;
		cas_body   = spc key spc flags spc exptime spc bytes spc cas_value	noreply spc? eol;
		get_body   = (spc key)+														spc? eol;
		del_body   = spc key (spc exptime)?									noreply spc? eol;
		cr_body    = spc key spc incr_value									noreply spc? eol;
		flush_body = (spc flush_delay)?									 	noreply spc? eol;

		set		= ("set"i		 %{req->op = MEMCACHED_TXT_CMD_SET;}	 store_body) @read_data @done;
		add		= ("add"i		 %{req->op = MEMCACHED_TXT_CMD_ADD;}	 store_body) @read_data @done;
		replace = ("replace"i	 %{req->op = MEMCACHED_TXT_CMD_REPLACE;} store_body) @read_data @done;
		append	= ("append"i	 %{req->op = MEMCACHED_TXT_CMD_APPEND;}  store_body) @read_data @done;
		prepend = ("prepend"i	 %{req->op = MEMCACHED_TXT_CMD_PREPEND;} store_body) @read_data @done;
		cas		= ("cas"i		 %{req->op = MEMCACHED_TXT_CMD_CAS;}	 cas_body)	 @read_data @done;

		get		= ("get"i		 %{req->op = MEMCACHED_TXT_CMD_GET;}	 get_body) @done;
		gets	= ("gets"i		 %{req->op = MEMCACHED_TXT_CMD_GETS;}	 get_body) @done;
		delete	= ("delete"i	 %{req->op = MEMCACHED_TXT_CMD_DELETE;}	 del_body) @done;
		incr	= ("incr"i		 %{req->op = MEMCACHED_TXT_CMD_INCR;}	 cr_body)  @done;
		decr	= ("decr"i		 %{req->op = MEMCACHED_TXT_CMD_DECR;}	 cr_body)  @done;

		stats	  = "stats"i	 %{req->op = MEMCACHED_TXT_CMD_STATS;}	 eol		@done;
		flush_all = "flush_all"i %{req->op = MEMCACHED_TXT_CMD_FLUSH;}	 flush_body @done;
		quit	  = "quit"i		 %{req->op = MEMCACHED_TXT_CMD_QUIT;}	 eol		@done;

		main := set | add | replace | append | prepend | cas |
				get | gets | delete | incr | decr |
				stats | flush_all | quit;

		write init;
		write exec;
	}%%


	if (!done) {
		if (p == pe)
			return 1;
		memcached_error_EINVALS("Invalid package structure");
		return -1;
	}
	*p_ptr = p;
	return 0;
}

/* vim: set ft=ragel noexpandtab ts=4 : */
