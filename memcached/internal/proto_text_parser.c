
/* #line 1 "memcached/internal/proto_text_parser.rl" */
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#include <tarantool/module.h>

#include "memcached.h"
#include "constants.h"
#include "utils.h"
#include "error.h"

#include "proto_text.h"
#include "proto_text_parser.h"


/* #line 22 "memcached/internal/proto_text_parser.c" */
static const int memcached_text_parser_start = 1;
/* static const int memcached_text_parser_first_final = 128; */
/* static const int memcached_text_parser_error = 0;         */
/* static const int memcached_text_parser_en_main = 1;       */


/* #line 21 "memcached/internal/proto_text_parser.rl" */


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

	
/* #line 53 "memcached/internal/proto_text_parser.c" */
	{
	cs = memcached_text_parser_start;
	}

/* #line 58 "memcached/internal/proto_text_parser.c" */
	{
	if ( p == pe )
		goto _test_eof;
	switch ( cs )
	{
case 1:
	switch( (*p) ) {
		case 65: goto st2;
		case 67: goto st28;
		case 68: goto st41;
		case 70: goto st67;
		case 71: goto st78;
		case 73: goto st85;
		case 80: goto st89;
		case 81: goto st96;
		case 82: goto st100;
		case 83: goto st107;
		case 86: goto st114;
		case 97: goto st2;
		case 99: goto st28;
		case 100: goto st41;
		case 102: goto st67;
		case 103: goto st78;
		case 105: goto st85;
		case 112: goto st89;
		case 113: goto st96;
		case 114: goto st100;
		case 115: goto st107;
		case 118: goto st114;
	}
	goto st0;
st0:
cs = 0;
	goto _out;
st2:
	if ( ++p == pe )
		goto _test_eof2;
case 2:
	switch( (*p) ) {
		case 68: goto st3;
		case 80: goto st23;
		case 100: goto st3;
		case 112: goto st23;
	}
	goto st0;
st3:
	if ( ++p == pe )
		goto _test_eof3;
case 3:
	switch( (*p) ) {
		case 68: goto st4;
		case 100: goto st4;
	}
	goto st0;
st4:
/* #line 112 "memcached/internal/proto_text_parser.rl" */
	{req->op = MEMCACHED_TXT_CMD_ADD;}
	if ( ++p == pe )
		goto _test_eof4;
case 4:
/* #line 119 "memcached/internal/proto_text_parser.c" */
	if ( (*p) == 32 )
		goto st5;
	goto st0;
st5:
	if ( ++p == pe )
		goto _test_eof5;
case 5:
	switch( (*p) ) {
		case 13: goto st0;
		case 32: goto st5;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st0;
	goto tr16;
tr16:
/* #line 43 "memcached/internal/proto_text_parser.rl" */
	{
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
	goto st6;
st6:
	if ( ++p == pe )
		goto _test_eof6;
case 6:
/* #line 153 "memcached/internal/proto_text_parser.c" */
	if ( (*p) == 32 )
		goto st7;
	goto st0;
st7:
	if ( ++p == pe )
		goto _test_eof7;
case 7:
	if ( (*p) == 32 )
		goto st7;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr18;
	goto st0;
tr18:
/* #line 84 "memcached/internal/proto_text_parser.rl" */
	{ s = p; }
	goto st8;
st8:
	if ( ++p == pe )
		goto _test_eof8;
case 8:
/* #line 174 "memcached/internal/proto_text_parser.c" */
	if ( (*p) == 32 )
		goto tr19;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st8;
	goto st0;
tr19:
/* #line 85 "memcached/internal/proto_text_parser.rl" */
	{ memcached_strtoul(s, p, &req->flags); }
	goto st9;
st9:
	if ( ++p == pe )
		goto _test_eof9;
case 9:
/* #line 188 "memcached/internal/proto_text_parser.c" */
	if ( (*p) == 32 )
		goto st9;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr22;
	goto st0;
tr22:
/* #line 81 "memcached/internal/proto_text_parser.rl" */
	{ s = p; }
	goto st10;
st10:
	if ( ++p == pe )
		goto _test_eof10;
case 10:
/* #line 202 "memcached/internal/proto_text_parser.c" */
	if ( (*p) == 32 )
		goto tr23;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st10;
	goto st0;
tr23:
/* #line 82 "memcached/internal/proto_text_parser.rl" */
	{ memcached_strtoul(s, p, &req->exptime); }
	goto st11;
st11:
	if ( ++p == pe )
		goto _test_eof11;
case 11:
/* #line 216 "memcached/internal/proto_text_parser.c" */
	if ( (*p) == 32 )
		goto st11;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr26;
	goto st0;
tr26:
/* #line 87 "memcached/internal/proto_text_parser.rl" */
	{ s = p; }
	goto st12;
st12:
	if ( ++p == pe )
		goto _test_eof12;
case 12:
/* #line 230 "memcached/internal/proto_text_parser.c" */
	switch( (*p) ) {
		case 10: goto tr27;
		case 13: goto tr28;
		case 32: goto tr29;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st12;
	goto st0;
tr27:
/* #line 88 "memcached/internal/proto_text_parser.rl" */
	{ memcached_strtoul(s, p, &req->bytes); }
/* #line 99 "memcached/internal/proto_text_parser.rl" */
	{ p++; }
/* #line 55 "memcached/internal/proto_text_parser.rl" */
	{
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
/* #line 74 "memcached/internal/proto_text_parser.rl" */
	{
			done = true;
		}
	goto st128;
tr31:
/* #line 99 "memcached/internal/proto_text_parser.rl" */
	{ p++; }
/* #line 55 "memcached/internal/proto_text_parser.rl" */
	{
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
/* #line 74 "memcached/internal/proto_text_parser.rl" */
	{
			done = true;
		}
	goto st128;
tr41:
/* #line 101 "memcached/internal/proto_text_parser.rl" */
	{ req->noreply = true; }
/* #line 99 "memcached/internal/proto_text_parser.rl" */
	{ p++; }
/* #line 55 "memcached/internal/proto_text_parser.rl" */
	{
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
/* #line 74 "memcached/internal/proto_text_parser.rl" */
	{
			done = true;
		}
	goto st128;
tr67:
/* #line 91 "memcached/internal/proto_text_parser.rl" */
	{ memcached_strtoul(s, p, &req->cas); }
/* #line 99 "memcached/internal/proto_text_parser.rl" */
	{ p++; }
/* #line 55 "memcached/internal/proto_text_parser.rl" */
	{
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
/* #line 74 "memcached/internal/proto_text_parser.rl" */
	{
			done = true;
		}
	goto st128;
tr79:
/* #line 94 "memcached/internal/proto_text_parser.rl" */
	{ memcached_strtoul(s, p, &req->delta); }
/* #line 99 "memcached/internal/proto_text_parser.rl" */
	{ p++; }
/* #line 74 "memcached/internal/proto_text_parser.rl" */
	{
			done = true;
		}
	goto st128;
tr83:
/* #line 99 "memcached/internal/proto_text_parser.rl" */
	{ p++; }
/* #line 74 "memcached/internal/proto_text_parser.rl" */
	{
			done = true;
		}
	goto st128;
tr93:
/* #line 101 "memcached/internal/proto_text_parser.rl" */
	{ req->noreply = true; }
/* #line 99 "memcached/internal/proto_text_parser.rl" */
	{ p++; }
/* #line 74 "memcached/internal/proto_text_parser.rl" */
	{
			done = true;
		}
	goto st128;
tr104:
/* #line 82 "memcached/internal/proto_text_parser.rl" */
	{ memcached_strtoul(s, p, &req->exptime); }
/* #line 99 "memcached/internal/proto_text_parser.rl" */
	{ p++; }
/* #line 74 "memcached/internal/proto_text_parser.rl" */
	{
			done = true;
		}
	goto st128;
tr118:
/* #line 97 "memcached/internal/proto_text_parser.rl" */
	{ memcached_strtoul(s, p, &req->exptime); }
/* #line 99 "memcached/internal/proto_text_parser.rl" */
	{ p++; }
/* #line 74 "memcached/internal/proto_text_parser.rl" */
	{
			done = true;
		}
	goto st128;
st128:
	if ( ++p == pe )
		goto _test_eof128;
case 128:
/* #line 409 "memcached/internal/proto_text_parser.c" */
	goto st0;
tr28:
/* #line 88 "memcached/internal/proto_text_parser.rl" */
	{ memcached_strtoul(s, p, &req->bytes); }
	goto st13;
tr42:
/* #line 101 "memcached/internal/proto_text_parser.rl" */
	{ req->noreply = true; }
	goto st13;
tr68:
/* #line 91 "memcached/internal/proto_text_parser.rl" */
	{ memcached_strtoul(s, p, &req->cas); }
	goto st13;
st13:
	if ( ++p == pe )
		goto _test_eof13;
case 13:
/* #line 427 "memcached/internal/proto_text_parser.c" */
	if ( (*p) == 10 )
		goto tr31;
	goto st0;
tr29:
/* #line 88 "memcached/internal/proto_text_parser.rl" */
	{ memcached_strtoul(s, p, &req->bytes); }
	goto st14;
tr69:
/* #line 91 "memcached/internal/proto_text_parser.rl" */
	{ memcached_strtoul(s, p, &req->cas); }
	goto st14;
st14:
	if ( ++p == pe )
		goto _test_eof14;
case 14:
/* #line 443 "memcached/internal/proto_text_parser.c" */
	switch( (*p) ) {
		case 10: goto tr31;
		case 13: goto st13;
		case 32: goto st14;
		case 78: goto st15;
		case 110: goto st15;
	}
	goto st0;
st15:
	if ( ++p == pe )
		goto _test_eof15;
case 15:
	switch( (*p) ) {
		case 79: goto st16;
		case 111: goto st16;
	}
	goto st0;
st16:
	if ( ++p == pe )
		goto _test_eof16;
case 16:
	switch( (*p) ) {
		case 82: goto st17;
		case 114: goto st17;
	}
	goto st0;
st17:
	if ( ++p == pe )
		goto _test_eof17;
case 17:
	switch( (*p) ) {
		case 69: goto st18;
		case 101: goto st18;
	}
	goto st0;
st18:
	if ( ++p == pe )
		goto _test_eof18;
case 18:
	switch( (*p) ) {
		case 80: goto st19;
		case 112: goto st19;
	}
	goto st0;
st19:
	if ( ++p == pe )
		goto _test_eof19;
case 19:
	switch( (*p) ) {
		case 76: goto st20;
		case 108: goto st20;
	}
	goto st0;
st20:
	if ( ++p == pe )
		goto _test_eof20;
case 20:
	switch( (*p) ) {
		case 89: goto st21;
		case 121: goto st21;
	}
	goto st0;
st21:
	if ( ++p == pe )
		goto _test_eof21;
case 21:
	switch( (*p) ) {
		case 10: goto tr41;
		case 13: goto tr42;
		case 32: goto tr43;
	}
	goto st0;
tr43:
/* #line 101 "memcached/internal/proto_text_parser.rl" */
	{ req->noreply = true; }
	goto st22;
st22:
	if ( ++p == pe )
		goto _test_eof22;
case 22:
/* #line 524 "memcached/internal/proto_text_parser.c" */
	switch( (*p) ) {
		case 10: goto tr31;
		case 13: goto st13;
		case 32: goto st22;
	}
	goto st0;
st23:
	if ( ++p == pe )
		goto _test_eof23;
case 23:
	switch( (*p) ) {
		case 80: goto st24;
		case 112: goto st24;
	}
	goto st0;
st24:
	if ( ++p == pe )
		goto _test_eof24;
case 24:
	switch( (*p) ) {
		case 69: goto st25;
		case 101: goto st25;
	}
	goto st0;
st25:
	if ( ++p == pe )
		goto _test_eof25;
case 25:
	switch( (*p) ) {
		case 78: goto st26;
		case 110: goto st26;
	}
	goto st0;
st26:
	if ( ++p == pe )
		goto _test_eof26;
case 26:
	switch( (*p) ) {
		case 68: goto st27;
		case 100: goto st27;
	}
	goto st0;
st27:
/* #line 114 "memcached/internal/proto_text_parser.rl" */
	{req->op = MEMCACHED_TXT_CMD_APPEND;}
	if ( ++p == pe )
		goto _test_eof27;
case 27:
/* #line 573 "memcached/internal/proto_text_parser.c" */
	if ( (*p) == 32 )
		goto st5;
	goto st0;
st28:
	if ( ++p == pe )
		goto _test_eof28;
case 28:
	switch( (*p) ) {
		case 65: goto st29;
		case 97: goto st29;
	}
	goto st0;
st29:
	if ( ++p == pe )
		goto _test_eof29;
case 29:
	switch( (*p) ) {
		case 83: goto st30;
		case 115: goto st30;
	}
	goto st0;
st30:
/* #line 116 "memcached/internal/proto_text_parser.rl" */
	{req->op = MEMCACHED_TXT_CMD_CAS;}
	if ( ++p == pe )
		goto _test_eof30;
case 30:
/* #line 601 "memcached/internal/proto_text_parser.c" */
	if ( (*p) == 32 )
		goto st31;
	goto st0;
st31:
	if ( ++p == pe )
		goto _test_eof31;
case 31:
	switch( (*p) ) {
		case 13: goto st0;
		case 32: goto st31;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st0;
	goto tr52;
tr52:
/* #line 43 "memcached/internal/proto_text_parser.rl" */
	{
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
	goto st32;
st32:
	if ( ++p == pe )
		goto _test_eof32;
case 32:
/* #line 635 "memcached/internal/proto_text_parser.c" */
	if ( (*p) == 32 )
		goto st33;
	goto st0;
st33:
	if ( ++p == pe )
		goto _test_eof33;
case 33:
	if ( (*p) == 32 )
		goto st33;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr54;
	goto st0;
tr54:
/* #line 84 "memcached/internal/proto_text_parser.rl" */
	{ s = p; }
	goto st34;
st34:
	if ( ++p == pe )
		goto _test_eof34;
case 34:
/* #line 656 "memcached/internal/proto_text_parser.c" */
	if ( (*p) == 32 )
		goto tr55;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st34;
	goto st0;
tr55:
/* #line 85 "memcached/internal/proto_text_parser.rl" */
	{ memcached_strtoul(s, p, &req->flags); }
	goto st35;
st35:
	if ( ++p == pe )
		goto _test_eof35;
case 35:
/* #line 670 "memcached/internal/proto_text_parser.c" */
	if ( (*p) == 32 )
		goto st35;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr58;
	goto st0;
tr58:
/* #line 81 "memcached/internal/proto_text_parser.rl" */
	{ s = p; }
	goto st36;
st36:
	if ( ++p == pe )
		goto _test_eof36;
case 36:
/* #line 684 "memcached/internal/proto_text_parser.c" */
	if ( (*p) == 32 )
		goto tr59;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st36;
	goto st0;
tr59:
/* #line 82 "memcached/internal/proto_text_parser.rl" */
	{ memcached_strtoul(s, p, &req->exptime); }
	goto st37;
st37:
	if ( ++p == pe )
		goto _test_eof37;
case 37:
/* #line 698 "memcached/internal/proto_text_parser.c" */
	if ( (*p) == 32 )
		goto st37;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr62;
	goto st0;
tr62:
/* #line 87 "memcached/internal/proto_text_parser.rl" */
	{ s = p; }
	goto st38;
st38:
	if ( ++p == pe )
		goto _test_eof38;
case 38:
/* #line 712 "memcached/internal/proto_text_parser.c" */
	if ( (*p) == 32 )
		goto tr63;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st38;
	goto st0;
tr63:
/* #line 88 "memcached/internal/proto_text_parser.rl" */
	{ memcached_strtoul(s, p, &req->bytes); }
	goto st39;
st39:
	if ( ++p == pe )
		goto _test_eof39;
case 39:
/* #line 726 "memcached/internal/proto_text_parser.c" */
	if ( (*p) == 32 )
		goto st39;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr66;
	goto st0;
tr66:
/* #line 90 "memcached/internal/proto_text_parser.rl" */
	{ s = p; }
	goto st40;
st40:
	if ( ++p == pe )
		goto _test_eof40;
case 40:
/* #line 740 "memcached/internal/proto_text_parser.c" */
	switch( (*p) ) {
		case 10: goto tr67;
		case 13: goto tr68;
		case 32: goto tr69;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st40;
	goto st0;
st41:
	if ( ++p == pe )
		goto _test_eof41;
case 41:
	switch( (*p) ) {
		case 69: goto st42;
		case 101: goto st42;
	}
	goto st0;
st42:
	if ( ++p == pe )
		goto _test_eof42;
case 42:
	switch( (*p) ) {
		case 67: goto st43;
		case 76: goto st59;
		case 99: goto st43;
		case 108: goto st59;
	}
	goto st0;
st43:
	if ( ++p == pe )
		goto _test_eof43;
case 43:
	switch( (*p) ) {
		case 82: goto st44;
		case 114: goto st44;
	}
	goto st0;
st44:
/* #line 122 "memcached/internal/proto_text_parser.rl" */
	{req->op = MEMCACHED_TXT_CMD_DECR;}
	if ( ++p == pe )
		goto _test_eof44;
case 44:
/* #line 784 "memcached/internal/proto_text_parser.c" */
	if ( (*p) == 32 )
		goto st45;
	goto st0;
st45:
	if ( ++p == pe )
		goto _test_eof45;
case 45:
	switch( (*p) ) {
		case 13: goto st0;
		case 32: goto st45;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st0;
	goto tr76;
tr76:
/* #line 43 "memcached/internal/proto_text_parser.rl" */
	{
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
	goto st46;
st46:
	if ( ++p == pe )
		goto _test_eof46;
case 46:
/* #line 818 "memcached/internal/proto_text_parser.c" */
	if ( (*p) == 32 )
		goto st47;
	goto st0;
st47:
	if ( ++p == pe )
		goto _test_eof47;
case 47:
	if ( (*p) == 32 )
		goto st47;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr78;
	goto st0;
tr78:
/* #line 93 "memcached/internal/proto_text_parser.rl" */
	{ s = p; }
	goto st48;
st48:
	if ( ++p == pe )
		goto _test_eof48;
case 48:
/* #line 839 "memcached/internal/proto_text_parser.c" */
	switch( (*p) ) {
		case 10: goto tr79;
		case 13: goto tr80;
		case 32: goto tr81;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st48;
	goto st0;
tr105:
/* #line 82 "memcached/internal/proto_text_parser.rl" */
	{ memcached_strtoul(s, p, &req->exptime); }
	goto st49;
tr94:
/* #line 101 "memcached/internal/proto_text_parser.rl" */
	{ req->noreply = true; }
	goto st49;
tr80:
/* #line 94 "memcached/internal/proto_text_parser.rl" */
	{ memcached_strtoul(s, p, &req->delta); }
	goto st49;
tr119:
/* #line 97 "memcached/internal/proto_text_parser.rl" */
	{ memcached_strtoul(s, p, &req->exptime); }
	goto st49;
st49:
	if ( ++p == pe )
		goto _test_eof49;
case 49:
/* #line 868 "memcached/internal/proto_text_parser.c" */
	if ( (*p) == 10 )
		goto tr83;
	goto st0;
tr106:
/* #line 82 "memcached/internal/proto_text_parser.rl" */
	{ memcached_strtoul(s, p, &req->exptime); }
	goto st50;
tr81:
/* #line 94 "memcached/internal/proto_text_parser.rl" */
	{ memcached_strtoul(s, p, &req->delta); }
	goto st50;
tr120:
/* #line 97 "memcached/internal/proto_text_parser.rl" */
	{ memcached_strtoul(s, p, &req->exptime); }
	goto st50;
st50:
	if ( ++p == pe )
		goto _test_eof50;
case 50:
/* #line 888 "memcached/internal/proto_text_parser.c" */
	switch( (*p) ) {
		case 10: goto tr83;
		case 13: goto st49;
		case 32: goto st50;
		case 78: goto st51;
		case 110: goto st51;
	}
	goto st0;
st51:
	if ( ++p == pe )
		goto _test_eof51;
case 51:
	switch( (*p) ) {
		case 79: goto st52;
		case 111: goto st52;
	}
	goto st0;
st52:
	if ( ++p == pe )
		goto _test_eof52;
case 52:
	switch( (*p) ) {
		case 82: goto st53;
		case 114: goto st53;
	}
	goto st0;
st53:
	if ( ++p == pe )
		goto _test_eof53;
case 53:
	switch( (*p) ) {
		case 69: goto st54;
		case 101: goto st54;
	}
	goto st0;
st54:
	if ( ++p == pe )
		goto _test_eof54;
case 54:
	switch( (*p) ) {
		case 80: goto st55;
		case 112: goto st55;
	}
	goto st0;
st55:
	if ( ++p == pe )
		goto _test_eof55;
case 55:
	switch( (*p) ) {
		case 76: goto st56;
		case 108: goto st56;
	}
	goto st0;
st56:
	if ( ++p == pe )
		goto _test_eof56;
case 56:
	switch( (*p) ) {
		case 89: goto st57;
		case 121: goto st57;
	}
	goto st0;
st57:
	if ( ++p == pe )
		goto _test_eof57;
case 57:
	switch( (*p) ) {
		case 10: goto tr93;
		case 13: goto tr94;
		case 32: goto tr95;
	}
	goto st0;
tr95:
/* #line 101 "memcached/internal/proto_text_parser.rl" */
	{ req->noreply = true; }
	goto st58;
st58:
	if ( ++p == pe )
		goto _test_eof58;
case 58:
/* #line 969 "memcached/internal/proto_text_parser.c" */
	switch( (*p) ) {
		case 10: goto tr83;
		case 13: goto st49;
		case 32: goto st58;
	}
	goto st0;
st59:
	if ( ++p == pe )
		goto _test_eof59;
case 59:
	switch( (*p) ) {
		case 69: goto st60;
		case 101: goto st60;
	}
	goto st0;
st60:
	if ( ++p == pe )
		goto _test_eof60;
case 60:
	switch( (*p) ) {
		case 84: goto st61;
		case 116: goto st61;
	}
	goto st0;
st61:
	if ( ++p == pe )
		goto _test_eof61;
case 61:
	switch( (*p) ) {
		case 69: goto st62;
		case 101: goto st62;
	}
	goto st0;
st62:
/* #line 120 "memcached/internal/proto_text_parser.rl" */
	{req->op = MEMCACHED_TXT_CMD_DELETE;}
	if ( ++p == pe )
		goto _test_eof62;
case 62:
/* #line 1009 "memcached/internal/proto_text_parser.c" */
	if ( (*p) == 32 )
		goto st63;
	goto st0;
st63:
	if ( ++p == pe )
		goto _test_eof63;
case 63:
	switch( (*p) ) {
		case 13: goto st0;
		case 32: goto st63;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st0;
	goto tr101;
tr101:
/* #line 43 "memcached/internal/proto_text_parser.rl" */
	{
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
	goto st64;
st64:
	if ( ++p == pe )
		goto _test_eof64;
case 64:
/* #line 1043 "memcached/internal/proto_text_parser.c" */
	switch( (*p) ) {
		case 10: goto tr83;
		case 13: goto st49;
		case 32: goto st65;
	}
	goto st0;
st65:
	if ( ++p == pe )
		goto _test_eof65;
case 65:
	switch( (*p) ) {
		case 10: goto tr83;
		case 13: goto st49;
		case 32: goto st65;
		case 78: goto st51;
		case 110: goto st51;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr103;
	goto st0;
tr103:
/* #line 81 "memcached/internal/proto_text_parser.rl" */
	{ s = p; }
	goto st66;
st66:
	if ( ++p == pe )
		goto _test_eof66;
case 66:
/* #line 1072 "memcached/internal/proto_text_parser.c" */
	switch( (*p) ) {
		case 10: goto tr104;
		case 13: goto tr105;
		case 32: goto tr106;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st66;
	goto st0;
st67:
	if ( ++p == pe )
		goto _test_eof67;
case 67:
	switch( (*p) ) {
		case 76: goto st68;
		case 108: goto st68;
	}
	goto st0;
st68:
	if ( ++p == pe )
		goto _test_eof68;
case 68:
	switch( (*p) ) {
		case 85: goto st69;
		case 117: goto st69;
	}
	goto st0;
st69:
	if ( ++p == pe )
		goto _test_eof69;
case 69:
	switch( (*p) ) {
		case 83: goto st70;
		case 115: goto st70;
	}
	goto st0;
st70:
	if ( ++p == pe )
		goto _test_eof70;
case 70:
	switch( (*p) ) {
		case 72: goto st71;
		case 104: goto st71;
	}
	goto st0;
st71:
	if ( ++p == pe )
		goto _test_eof71;
case 71:
	if ( (*p) == 95 )
		goto st72;
	goto st0;
st72:
	if ( ++p == pe )
		goto _test_eof72;
case 72:
	switch( (*p) ) {
		case 65: goto st73;
		case 97: goto st73;
	}
	goto st0;
st73:
	if ( ++p == pe )
		goto _test_eof73;
case 73:
	switch( (*p) ) {
		case 76: goto st74;
		case 108: goto st74;
	}
	goto st0;
st74:
	if ( ++p == pe )
		goto _test_eof74;
case 74:
	switch( (*p) ) {
		case 76: goto st75;
		case 108: goto st75;
	}
	goto st0;
st75:
/* #line 127 "memcached/internal/proto_text_parser.rl" */
	{req->op = MEMCACHED_TXT_CMD_FLUSH;}
	if ( ++p == pe )
		goto _test_eof75;
case 75:
/* #line 1157 "memcached/internal/proto_text_parser.c" */
	switch( (*p) ) {
		case 10: goto tr83;
		case 13: goto st49;
		case 32: goto st76;
	}
	goto st0;
st76:
	if ( ++p == pe )
		goto _test_eof76;
case 76:
	switch( (*p) ) {
		case 10: goto tr83;
		case 13: goto st49;
		case 32: goto st76;
		case 78: goto st51;
		case 110: goto st51;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr117;
	goto st0;
tr117:
/* #line 96 "memcached/internal/proto_text_parser.rl" */
	{ s = p; }
	goto st77;
st77:
	if ( ++p == pe )
		goto _test_eof77;
case 77:
/* #line 1186 "memcached/internal/proto_text_parser.c" */
	switch( (*p) ) {
		case 10: goto tr118;
		case 13: goto tr119;
		case 32: goto tr120;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st77;
	goto st0;
st78:
	if ( ++p == pe )
		goto _test_eof78;
case 78:
	switch( (*p) ) {
		case 69: goto st79;
		case 101: goto st79;
	}
	goto st0;
st79:
	if ( ++p == pe )
		goto _test_eof79;
case 79:
	switch( (*p) ) {
		case 84: goto st80;
		case 116: goto st80;
	}
	goto st0;
st80:
/* #line 118 "memcached/internal/proto_text_parser.rl" */
	{req->op = MEMCACHED_TXT_CMD_GET;}
	if ( ++p == pe )
		goto _test_eof80;
case 80:
/* #line 1219 "memcached/internal/proto_text_parser.c" */
	switch( (*p) ) {
		case 32: goto st81;
		case 83: goto st84;
		case 115: goto st84;
	}
	goto st0;
st81:
	if ( ++p == pe )
		goto _test_eof81;
case 81:
	switch( (*p) ) {
		case 13: goto st0;
		case 32: goto st81;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st0;
	goto tr126;
tr126:
/* #line 43 "memcached/internal/proto_text_parser.rl" */
	{
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
	goto st82;
st82:
	if ( ++p == pe )
		goto _test_eof82;
case 82:
/* #line 1256 "memcached/internal/proto_text_parser.c" */
	switch( (*p) ) {
		case 10: goto tr83;
		case 13: goto st49;
		case 32: goto st83;
	}
	goto st0;
st83:
	if ( ++p == pe )
		goto _test_eof83;
case 83:
	switch( (*p) ) {
		case 9: goto st0;
		case 10: goto tr83;
		case 13: goto st49;
		case 32: goto st83;
	}
	goto tr126;
st84:
/* #line 119 "memcached/internal/proto_text_parser.rl" */
	{req->op = MEMCACHED_TXT_CMD_GETS;}
	if ( ++p == pe )
		goto _test_eof84;
case 84:
/* #line 1280 "memcached/internal/proto_text_parser.c" */
	if ( (*p) == 32 )
		goto st81;
	goto st0;
st85:
	if ( ++p == pe )
		goto _test_eof85;
case 85:
	switch( (*p) ) {
		case 78: goto st86;
		case 110: goto st86;
	}
	goto st0;
st86:
	if ( ++p == pe )
		goto _test_eof86;
case 86:
	switch( (*p) ) {
		case 67: goto st87;
		case 99: goto st87;
	}
	goto st0;
st87:
	if ( ++p == pe )
		goto _test_eof87;
case 87:
	switch( (*p) ) {
		case 82: goto st88;
		case 114: goto st88;
	}
	goto st0;
st88:
/* #line 121 "memcached/internal/proto_text_parser.rl" */
	{req->op = MEMCACHED_TXT_CMD_INCR;}
	if ( ++p == pe )
		goto _test_eof88;
case 88:
/* #line 1317 "memcached/internal/proto_text_parser.c" */
	if ( (*p) == 32 )
		goto st45;
	goto st0;
st89:
	if ( ++p == pe )
		goto _test_eof89;
case 89:
	switch( (*p) ) {
		case 82: goto st90;
		case 114: goto st90;
	}
	goto st0;
st90:
	if ( ++p == pe )
		goto _test_eof90;
case 90:
	switch( (*p) ) {
		case 69: goto st91;
		case 101: goto st91;
	}
	goto st0;
st91:
	if ( ++p == pe )
		goto _test_eof91;
case 91:
	switch( (*p) ) {
		case 80: goto st92;
		case 112: goto st92;
	}
	goto st0;
st92:
	if ( ++p == pe )
		goto _test_eof92;
case 92:
	switch( (*p) ) {
		case 69: goto st93;
		case 101: goto st93;
	}
	goto st0;
st93:
	if ( ++p == pe )
		goto _test_eof93;
case 93:
	switch( (*p) ) {
		case 78: goto st94;
		case 110: goto st94;
	}
	goto st0;
st94:
	if ( ++p == pe )
		goto _test_eof94;
case 94:
	switch( (*p) ) {
		case 68: goto st95;
		case 100: goto st95;
	}
	goto st0;
st95:
/* #line 115 "memcached/internal/proto_text_parser.rl" */
	{req->op = MEMCACHED_TXT_CMD_PREPEND;}
	if ( ++p == pe )
		goto _test_eof95;
case 95:
/* #line 1381 "memcached/internal/proto_text_parser.c" */
	if ( (*p) == 32 )
		goto st5;
	goto st0;
st96:
	if ( ++p == pe )
		goto _test_eof96;
case 96:
	switch( (*p) ) {
		case 85: goto st97;
		case 117: goto st97;
	}
	goto st0;
st97:
	if ( ++p == pe )
		goto _test_eof97;
case 97:
	switch( (*p) ) {
		case 73: goto st98;
		case 105: goto st98;
	}
	goto st0;
st98:
	if ( ++p == pe )
		goto _test_eof98;
case 98:
	switch( (*p) ) {
		case 84: goto st99;
		case 116: goto st99;
	}
	goto st0;
st99:
/* #line 128 "memcached/internal/proto_text_parser.rl" */
	{req->op = MEMCACHED_TXT_CMD_QUIT;}
	if ( ++p == pe )
		goto _test_eof99;
case 99:
/* #line 1418 "memcached/internal/proto_text_parser.c" */
	switch( (*p) ) {
		case 10: goto tr83;
		case 13: goto st49;
	}
	goto st0;
st100:
	if ( ++p == pe )
		goto _test_eof100;
case 100:
	switch( (*p) ) {
		case 69: goto st101;
		case 101: goto st101;
	}
	goto st0;
st101:
	if ( ++p == pe )
		goto _test_eof101;
case 101:
	switch( (*p) ) {
		case 80: goto st102;
		case 112: goto st102;
	}
	goto st0;
st102:
	if ( ++p == pe )
		goto _test_eof102;
case 102:
	switch( (*p) ) {
		case 76: goto st103;
		case 108: goto st103;
	}
	goto st0;
st103:
	if ( ++p == pe )
		goto _test_eof103;
case 103:
	switch( (*p) ) {
		case 65: goto st104;
		case 97: goto st104;
	}
	goto st0;
st104:
	if ( ++p == pe )
		goto _test_eof104;
case 104:
	switch( (*p) ) {
		case 67: goto st105;
		case 99: goto st105;
	}
	goto st0;
st105:
	if ( ++p == pe )
		goto _test_eof105;
case 105:
	switch( (*p) ) {
		case 69: goto st106;
		case 101: goto st106;
	}
	goto st0;
st106:
/* #line 113 "memcached/internal/proto_text_parser.rl" */
	{req->op = MEMCACHED_TXT_CMD_REPLACE;}
	if ( ++p == pe )
		goto _test_eof106;
case 106:
/* #line 1484 "memcached/internal/proto_text_parser.c" */
	if ( (*p) == 32 )
		goto st5;
	goto st0;
st107:
	if ( ++p == pe )
		goto _test_eof107;
case 107:
	switch( (*p) ) {
		case 69: goto st108;
		case 84: goto st110;
		case 101: goto st108;
		case 116: goto st110;
	}
	goto st0;
st108:
	if ( ++p == pe )
		goto _test_eof108;
case 108:
	switch( (*p) ) {
		case 84: goto st109;
		case 116: goto st109;
	}
	goto st0;
st109:
/* #line 111 "memcached/internal/proto_text_parser.rl" */
	{req->op = MEMCACHED_TXT_CMD_SET;}
	if ( ++p == pe )
		goto _test_eof109;
case 109:
/* #line 1514 "memcached/internal/proto_text_parser.c" */
	if ( (*p) == 32 )
		goto st5;
	goto st0;
st110:
	if ( ++p == pe )
		goto _test_eof110;
case 110:
	switch( (*p) ) {
		case 65: goto st111;
		case 97: goto st111;
	}
	goto st0;
st111:
	if ( ++p == pe )
		goto _test_eof111;
case 111:
	switch( (*p) ) {
		case 84: goto st112;
		case 116: goto st112;
	}
	goto st0;
st112:
	if ( ++p == pe )
		goto _test_eof112;
case 112:
	switch( (*p) ) {
		case 83: goto st113;
		case 115: goto st113;
	}
	goto st0;
st113:
/* #line 126 "memcached/internal/proto_text_parser.rl" */
	{req->op = MEMCACHED_TXT_CMD_STATS;}
	if ( ++p == pe )
		goto _test_eof113;
case 113:
/* #line 1551 "memcached/internal/proto_text_parser.c" */
	switch( (*p) ) {
		case 10: goto tr83;
		case 13: goto st49;
	}
	goto st0;
st114:
	if ( ++p == pe )
		goto _test_eof114;
case 114:
	switch( (*p) ) {
		case 69: goto st115;
		case 101: goto st115;
	}
	goto st0;
st115:
	if ( ++p == pe )
		goto _test_eof115;
case 115:
	switch( (*p) ) {
		case 82: goto st116;
		case 114: goto st116;
	}
	goto st0;
st116:
	if ( ++p == pe )
		goto _test_eof116;
case 116:
	switch( (*p) ) {
		case 66: goto st117;
		case 83: goto st124;
		case 98: goto st117;
		case 115: goto st124;
	}
	goto st0;
st117:
	if ( ++p == pe )
		goto _test_eof117;
case 117:
	switch( (*p) ) {
		case 79: goto st118;
		case 111: goto st118;
	}
	goto st0;
st118:
	if ( ++p == pe )
		goto _test_eof118;
case 118:
	switch( (*p) ) {
		case 83: goto st119;
		case 115: goto st119;
	}
	goto st0;
st119:
	if ( ++p == pe )
		goto _test_eof119;
case 119:
	switch( (*p) ) {
		case 73: goto st120;
		case 105: goto st120;
	}
	goto st0;
st120:
	if ( ++p == pe )
		goto _test_eof120;
case 120:
	switch( (*p) ) {
		case 84: goto st121;
		case 116: goto st121;
	}
	goto st0;
st121:
	if ( ++p == pe )
		goto _test_eof121;
case 121:
	switch( (*p) ) {
		case 89: goto st122;
		case 121: goto st122;
	}
	goto st0;
st122:
/* #line 125 "memcached/internal/proto_text_parser.rl" */
	{req->op = MEMCACHED_TXT_CMD_VERBOSITY;}
	if ( ++p == pe )
		goto _test_eof122;
case 122:
/* #line 1637 "memcached/internal/proto_text_parser.c" */
	if ( (*p) == 32 )
		goto st123;
	goto st0;
st123:
	if ( ++p == pe )
		goto _test_eof123;
case 123:
	if ( (*p) == 32 )
		goto st123;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr117;
	goto st0;
st124:
	if ( ++p == pe )
		goto _test_eof124;
case 124:
	switch( (*p) ) {
		case 73: goto st125;
		case 105: goto st125;
	}
	goto st0;
st125:
	if ( ++p == pe )
		goto _test_eof125;
case 125:
	switch( (*p) ) {
		case 79: goto st126;
		case 111: goto st126;
	}
	goto st0;
st126:
	if ( ++p == pe )
		goto _test_eof126;
case 126:
	switch( (*p) ) {
		case 78: goto st127;
		case 110: goto st127;
	}
	goto st0;
st127:
/* #line 124 "memcached/internal/proto_text_parser.rl" */
	{req->op = MEMCACHED_TXT_CMD_VERSION;}
	if ( ++p == pe )
		goto _test_eof127;
case 127:
/* #line 1683 "memcached/internal/proto_text_parser.c" */
	switch( (*p) ) {
		case 10: goto tr83;
		case 13: goto st49;
	}
	goto st0;
	}
	_test_eof2: cs = 2; goto _test_eof; 
	_test_eof3: cs = 3; goto _test_eof; 
	_test_eof4: cs = 4; goto _test_eof; 
	_test_eof5: cs = 5; goto _test_eof; 
	_test_eof6: cs = 6; goto _test_eof; 
	_test_eof7: cs = 7; goto _test_eof; 
	_test_eof8: cs = 8; goto _test_eof; 
	_test_eof9: cs = 9; goto _test_eof; 
	_test_eof10: cs = 10; goto _test_eof; 
	_test_eof11: cs = 11; goto _test_eof; 
	_test_eof12: cs = 12; goto _test_eof; 
	_test_eof128: cs = 128; goto _test_eof; 
	_test_eof13: cs = 13; goto _test_eof; 
	_test_eof14: cs = 14; goto _test_eof; 
	_test_eof15: cs = 15; goto _test_eof; 
	_test_eof16: cs = 16; goto _test_eof; 
	_test_eof17: cs = 17; goto _test_eof; 
	_test_eof18: cs = 18; goto _test_eof; 
	_test_eof19: cs = 19; goto _test_eof; 
	_test_eof20: cs = 20; goto _test_eof; 
	_test_eof21: cs = 21; goto _test_eof; 
	_test_eof22: cs = 22; goto _test_eof; 
	_test_eof23: cs = 23; goto _test_eof; 
	_test_eof24: cs = 24; goto _test_eof; 
	_test_eof25: cs = 25; goto _test_eof; 
	_test_eof26: cs = 26; goto _test_eof; 
	_test_eof27: cs = 27; goto _test_eof; 
	_test_eof28: cs = 28; goto _test_eof; 
	_test_eof29: cs = 29; goto _test_eof; 
	_test_eof30: cs = 30; goto _test_eof; 
	_test_eof31: cs = 31; goto _test_eof; 
	_test_eof32: cs = 32; goto _test_eof; 
	_test_eof33: cs = 33; goto _test_eof; 
	_test_eof34: cs = 34; goto _test_eof; 
	_test_eof35: cs = 35; goto _test_eof; 
	_test_eof36: cs = 36; goto _test_eof; 
	_test_eof37: cs = 37; goto _test_eof; 
	_test_eof38: cs = 38; goto _test_eof; 
	_test_eof39: cs = 39; goto _test_eof; 
	_test_eof40: cs = 40; goto _test_eof; 
	_test_eof41: cs = 41; goto _test_eof; 
	_test_eof42: cs = 42; goto _test_eof; 
	_test_eof43: cs = 43; goto _test_eof; 
	_test_eof44: cs = 44; goto _test_eof; 
	_test_eof45: cs = 45; goto _test_eof; 
	_test_eof46: cs = 46; goto _test_eof; 
	_test_eof47: cs = 47; goto _test_eof; 
	_test_eof48: cs = 48; goto _test_eof; 
	_test_eof49: cs = 49; goto _test_eof; 
	_test_eof50: cs = 50; goto _test_eof; 
	_test_eof51: cs = 51; goto _test_eof; 
	_test_eof52: cs = 52; goto _test_eof; 
	_test_eof53: cs = 53; goto _test_eof; 
	_test_eof54: cs = 54; goto _test_eof; 
	_test_eof55: cs = 55; goto _test_eof; 
	_test_eof56: cs = 56; goto _test_eof; 
	_test_eof57: cs = 57; goto _test_eof; 
	_test_eof58: cs = 58; goto _test_eof; 
	_test_eof59: cs = 59; goto _test_eof; 
	_test_eof60: cs = 60; goto _test_eof; 
	_test_eof61: cs = 61; goto _test_eof; 
	_test_eof62: cs = 62; goto _test_eof; 
	_test_eof63: cs = 63; goto _test_eof; 
	_test_eof64: cs = 64; goto _test_eof; 
	_test_eof65: cs = 65; goto _test_eof; 
	_test_eof66: cs = 66; goto _test_eof; 
	_test_eof67: cs = 67; goto _test_eof; 
	_test_eof68: cs = 68; goto _test_eof; 
	_test_eof69: cs = 69; goto _test_eof; 
	_test_eof70: cs = 70; goto _test_eof; 
	_test_eof71: cs = 71; goto _test_eof; 
	_test_eof72: cs = 72; goto _test_eof; 
	_test_eof73: cs = 73; goto _test_eof; 
	_test_eof74: cs = 74; goto _test_eof; 
	_test_eof75: cs = 75; goto _test_eof; 
	_test_eof76: cs = 76; goto _test_eof; 
	_test_eof77: cs = 77; goto _test_eof; 
	_test_eof78: cs = 78; goto _test_eof; 
	_test_eof79: cs = 79; goto _test_eof; 
	_test_eof80: cs = 80; goto _test_eof; 
	_test_eof81: cs = 81; goto _test_eof; 
	_test_eof82: cs = 82; goto _test_eof; 
	_test_eof83: cs = 83; goto _test_eof; 
	_test_eof84: cs = 84; goto _test_eof; 
	_test_eof85: cs = 85; goto _test_eof; 
	_test_eof86: cs = 86; goto _test_eof; 
	_test_eof87: cs = 87; goto _test_eof; 
	_test_eof88: cs = 88; goto _test_eof; 
	_test_eof89: cs = 89; goto _test_eof; 
	_test_eof90: cs = 90; goto _test_eof; 
	_test_eof91: cs = 91; goto _test_eof; 
	_test_eof92: cs = 92; goto _test_eof; 
	_test_eof93: cs = 93; goto _test_eof; 
	_test_eof94: cs = 94; goto _test_eof; 
	_test_eof95: cs = 95; goto _test_eof; 
	_test_eof96: cs = 96; goto _test_eof; 
	_test_eof97: cs = 97; goto _test_eof; 
	_test_eof98: cs = 98; goto _test_eof; 
	_test_eof99: cs = 99; goto _test_eof; 
	_test_eof100: cs = 100; goto _test_eof; 
	_test_eof101: cs = 101; goto _test_eof; 
	_test_eof102: cs = 102; goto _test_eof; 
	_test_eof103: cs = 103; goto _test_eof; 
	_test_eof104: cs = 104; goto _test_eof; 
	_test_eof105: cs = 105; goto _test_eof; 
	_test_eof106: cs = 106; goto _test_eof; 
	_test_eof107: cs = 107; goto _test_eof; 
	_test_eof108: cs = 108; goto _test_eof; 
	_test_eof109: cs = 109; goto _test_eof; 
	_test_eof110: cs = 110; goto _test_eof; 
	_test_eof111: cs = 111; goto _test_eof; 
	_test_eof112: cs = 112; goto _test_eof; 
	_test_eof113: cs = 113; goto _test_eof; 
	_test_eof114: cs = 114; goto _test_eof; 
	_test_eof115: cs = 115; goto _test_eof; 
	_test_eof116: cs = 116; goto _test_eof; 
	_test_eof117: cs = 117; goto _test_eof; 
	_test_eof118: cs = 118; goto _test_eof; 
	_test_eof119: cs = 119; goto _test_eof; 
	_test_eof120: cs = 120; goto _test_eof; 
	_test_eof121: cs = 121; goto _test_eof; 
	_test_eof122: cs = 122; goto _test_eof; 
	_test_eof123: cs = 123; goto _test_eof; 
	_test_eof124: cs = 124; goto _test_eof; 
	_test_eof125: cs = 125; goto _test_eof; 
	_test_eof126: cs = 126; goto _test_eof; 
	_test_eof127: cs = 127; goto _test_eof; 

	_test_eof: {}
	_out: {}
	}

/* #line 136 "memcached/internal/proto_text_parser.rl" */


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
