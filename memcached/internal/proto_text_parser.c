
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
static const int memcached_text_parser_first_final = 114;
static const int memcached_text_parser_error = 0;

static const int memcached_text_parser_en_main = 1;


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
	if ( ++p == pe )
		goto _test_eof4;
case 4:
	if ( (*p) == 32 )
		goto tr14;
	goto st0;
tr14:
/* #line 152 "memcached/internal/proto_text_parser.rl" */
	{req->op = MEMCACHED_TXT_CMD_ADD;}
	goto st5;
tr49:
/* #line 154 "memcached/internal/proto_text_parser.rl" */
	{req->op = MEMCACHED_TXT_CMD_APPEND;}
	goto st5;
tr147:
/* #line 155 "memcached/internal/proto_text_parser.rl" */
	{req->op = MEMCACHED_TXT_CMD_PREPEND;}
	goto st5;
tr159:
/* #line 153 "memcached/internal/proto_text_parser.rl" */
	{req->op = MEMCACHED_TXT_CMD_REPLACE;}
	goto st5;
tr163:
/* #line 151 "memcached/internal/proto_text_parser.rl" */
	{req->op = MEMCACHED_TXT_CMD_SET;}
	goto st5;
st5:
	if ( ++p == pe )
		goto _test_eof5;
case 5:
/* #line 142 "memcached/internal/proto_text_parser.c" */
	switch( (*p) ) {
		case 13: goto st0;
		case 32: goto st5;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st0;
	goto tr15;
tr15:
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
/* #line 169 "memcached/internal/proto_text_parser.c" */
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
/* #line 90 "memcached/internal/proto_text_parser.rl" */
	{ s = p; }
	goto st8;
st8:
	if ( ++p == pe )
		goto _test_eof8;
case 8:
/* #line 190 "memcached/internal/proto_text_parser.c" */
	if ( (*p) == 32 )
		goto tr19;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st8;
	goto st0;
tr19:
/* #line 91 "memcached/internal/proto_text_parser.rl" */
	{
					if (memcached_strtoul(s, p, &req->flags) == -1) {
						memcached_error_EINVALS("bad flags value");
						con->close_connection = true;
						return -1;
					}
				}
	goto st9;
st9:
	if ( ++p == pe )
		goto _test_eof9;
case 9:
/* #line 210 "memcached/internal/proto_text_parser.c" */
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
/* #line 224 "memcached/internal/proto_text_parser.c" */
	if ( (*p) == 32 )
		goto tr23;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st10;
	goto st0;
tr23:
/* #line 82 "memcached/internal/proto_text_parser.rl" */
	{
					if (memcached_strtoul(s, p, &req->exptime) == -1) {
						memcached_error_EINVALS("bad expiration time value");
						con->close_connection = true;
						return -1;
					}
				}
	goto st11;
st11:
	if ( ++p == pe )
		goto _test_eof11;
case 11:
/* #line 244 "memcached/internal/proto_text_parser.c" */
	if ( (*p) == 32 )
		goto st11;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr26;
	goto st0;
tr26:
/* #line 99 "memcached/internal/proto_text_parser.rl" */
	{ s = p; }
	goto st12;
st12:
	if ( ++p == pe )
		goto _test_eof12;
case 12:
/* #line 258 "memcached/internal/proto_text_parser.c" */
	switch( (*p) ) {
		case 10: goto tr27;
		case 13: goto tr28;
		case 32: goto tr29;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st12;
	goto st0;
tr27:
/* #line 100 "memcached/internal/proto_text_parser.rl" */
	{
					if (memcached_strtoul(s, p, &req->bytes) == -1) {
						memcached_error_EINVALS("bad bytes value");
						con->close_connection = true;
						return -1;
					} else if (req->bytes > MEMCACHED_MAX_SIZE) {
						memcached_error_E2BIG();
						con->close_connection = true;
						return -1;
					}
				}
/* #line 140 "memcached/internal/proto_text_parser.rl" */
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
	goto st114;
tr31:
/* #line 140 "memcached/internal/proto_text_parser.rl" */
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
	goto st114;
tr41:
/* #line 142 "memcached/internal/proto_text_parser.rl" */
	{ req->noreply = true; }
/* #line 140 "memcached/internal/proto_text_parser.rl" */
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
	goto st114;
tr69:
/* #line 113 "memcached/internal/proto_text_parser.rl" */
	{
					if (memcached_strtoul(s, p, &req->cas) == -1) {
						memcached_error_EINVALS("bad cas value");
						con->close_connection = true;
						return -1;
					}
				}
/* #line 140 "memcached/internal/proto_text_parser.rl" */
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
	goto st114;
tr82:
/* #line 122 "memcached/internal/proto_text_parser.rl" */
	{
					if (memcached_strtoul(s, p, &req->delta) == -1) {
						memcached_error_DELTA_BADVAL();
						// memcached_error_EINVALS("bad incr/decr value");
						con->close_connection = true;
						return -1;
					}
				}
/* #line 140 "memcached/internal/proto_text_parser.rl" */
	{ p++; }
/* #line 74 "memcached/internal/proto_text_parser.rl" */
	{
			done = true;
		}
	goto st114;
tr86:
/* #line 140 "memcached/internal/proto_text_parser.rl" */
	{ p++; }
/* #line 74 "memcached/internal/proto_text_parser.rl" */
	{
			done = true;
		}
	goto st114;
tr96:
/* #line 142 "memcached/internal/proto_text_parser.rl" */
	{ req->noreply = true; }
/* #line 140 "memcached/internal/proto_text_parser.rl" */
	{ p++; }
/* #line 74 "memcached/internal/proto_text_parser.rl" */
	{
			done = true;
		}
	goto st114;
tr108:
/* #line 82 "memcached/internal/proto_text_parser.rl" */
	{
					if (memcached_strtoul(s, p, &req->exptime) == -1) {
						memcached_error_EINVALS("bad expiration time value");
						con->close_connection = true;
						return -1;
					}
				}
/* #line 140 "memcached/internal/proto_text_parser.rl" */
	{ p++; }
/* #line 74 "memcached/internal/proto_text_parser.rl" */
	{
			done = true;
		}
	goto st114;
tr120:
/* #line 165 "memcached/internal/proto_text_parser.rl" */
	{req->op = MEMCACHED_TXT_CMD_FLUSH;}
/* #line 140 "memcached/internal/proto_text_parser.rl" */
	{ p++; }
/* #line 74 "memcached/internal/proto_text_parser.rl" */
	{
			done = true;
		}
	goto st114;
tr125:
/* #line 132 "memcached/internal/proto_text_parser.rl" */
	{
					if (memcached_strtoul(s, p, &req->exptime) == -1) {
						memcached_error_EINVALS("bad flush value");
						con->close_connection = true;
						return -1;
					}
				}
/* #line 140 "memcached/internal/proto_text_parser.rl" */
	{ p++; }
/* #line 74 "memcached/internal/proto_text_parser.rl" */
	{
			done = true;
		}
	goto st114;
tr151:
/* #line 166 "memcached/internal/proto_text_parser.rl" */
	{req->op = MEMCACHED_TXT_CMD_QUIT;}
/* #line 140 "memcached/internal/proto_text_parser.rl" */
	{ p++; }
/* #line 74 "memcached/internal/proto_text_parser.rl" */
	{
			done = true;
		}
	goto st114;
tr167:
/* #line 164 "memcached/internal/proto_text_parser.rl" */
	{req->op = MEMCACHED_TXT_CMD_STATS;}
/* #line 140 "memcached/internal/proto_text_parser.rl" */
	{ p++; }
/* #line 74 "memcached/internal/proto_text_parser.rl" */
	{
			done = true;
		}
	goto st114;
st114:
	if ( ++p == pe )
		goto _test_eof114;
case 114:
/* #line 502 "memcached/internal/proto_text_parser.c" */
	goto st0;
tr28:
/* #line 100 "memcached/internal/proto_text_parser.rl" */
	{
					if (memcached_strtoul(s, p, &req->bytes) == -1) {
						memcached_error_EINVALS("bad bytes value");
						con->close_connection = true;
						return -1;
					} else if (req->bytes > MEMCACHED_MAX_SIZE) {
						memcached_error_E2BIG();
						con->close_connection = true;
						return -1;
					}
				}
	goto st13;
tr42:
/* #line 142 "memcached/internal/proto_text_parser.rl" */
	{ req->noreply = true; }
	goto st13;
tr70:
/* #line 113 "memcached/internal/proto_text_parser.rl" */
	{
					if (memcached_strtoul(s, p, &req->cas) == -1) {
						memcached_error_EINVALS("bad cas value");
						con->close_connection = true;
						return -1;
					}
				}
	goto st13;
st13:
	if ( ++p == pe )
		goto _test_eof13;
case 13:
/* #line 536 "memcached/internal/proto_text_parser.c" */
	if ( (*p) == 10 )
		goto tr31;
	goto st0;
tr29:
/* #line 100 "memcached/internal/proto_text_parser.rl" */
	{
					if (memcached_strtoul(s, p, &req->bytes) == -1) {
						memcached_error_EINVALS("bad bytes value");
						con->close_connection = true;
						return -1;
					} else if (req->bytes > MEMCACHED_MAX_SIZE) {
						memcached_error_E2BIG();
						con->close_connection = true;
						return -1;
					}
				}
	goto st14;
tr71:
/* #line 113 "memcached/internal/proto_text_parser.rl" */
	{
					if (memcached_strtoul(s, p, &req->cas) == -1) {
						memcached_error_EINVALS("bad cas value");
						con->close_connection = true;
						return -1;
					}
				}
	goto st14;
st14:
	if ( ++p == pe )
		goto _test_eof14;
case 14:
/* #line 568 "memcached/internal/proto_text_parser.c" */
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
/* #line 142 "memcached/internal/proto_text_parser.rl" */
	{ req->noreply = true; }
	goto st22;
st22:
	if ( ++p == pe )
		goto _test_eof22;
case 22:
/* #line 649 "memcached/internal/proto_text_parser.c" */
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
	if ( ++p == pe )
		goto _test_eof27;
case 27:
	if ( (*p) == 32 )
		goto tr49;
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
	if ( ++p == pe )
		goto _test_eof30;
case 30:
	if ( (*p) == 32 )
		goto tr52;
	goto st0;
tr52:
/* #line 156 "memcached/internal/proto_text_parser.rl" */
	{req->op = MEMCACHED_TXT_CMD_CAS;}
	goto st31;
st31:
	if ( ++p == pe )
		goto _test_eof31;
case 31:
/* #line 732 "memcached/internal/proto_text_parser.c" */
	switch( (*p) ) {
		case 13: goto st0;
		case 32: goto st31;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st0;
	goto tr53;
tr53:
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
/* #line 759 "memcached/internal/proto_text_parser.c" */
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
		goto tr56;
	goto st0;
tr56:
/* #line 90 "memcached/internal/proto_text_parser.rl" */
	{ s = p; }
	goto st34;
st34:
	if ( ++p == pe )
		goto _test_eof34;
case 34:
/* #line 780 "memcached/internal/proto_text_parser.c" */
	if ( (*p) == 32 )
		goto tr57;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st34;
	goto st0;
tr57:
/* #line 91 "memcached/internal/proto_text_parser.rl" */
	{
					if (memcached_strtoul(s, p, &req->flags) == -1) {
						memcached_error_EINVALS("bad flags value");
						con->close_connection = true;
						return -1;
					}
				}
	goto st35;
st35:
	if ( ++p == pe )
		goto _test_eof35;
case 35:
/* #line 800 "memcached/internal/proto_text_parser.c" */
	if ( (*p) == 32 )
		goto st35;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr60;
	goto st0;
tr60:
/* #line 81 "memcached/internal/proto_text_parser.rl" */
	{ s = p; }
	goto st36;
st36:
	if ( ++p == pe )
		goto _test_eof36;
case 36:
/* #line 814 "memcached/internal/proto_text_parser.c" */
	if ( (*p) == 32 )
		goto tr61;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st36;
	goto st0;
tr61:
/* #line 82 "memcached/internal/proto_text_parser.rl" */
	{
					if (memcached_strtoul(s, p, &req->exptime) == -1) {
						memcached_error_EINVALS("bad expiration time value");
						con->close_connection = true;
						return -1;
					}
				}
	goto st37;
st37:
	if ( ++p == pe )
		goto _test_eof37;
case 37:
/* #line 834 "memcached/internal/proto_text_parser.c" */
	if ( (*p) == 32 )
		goto st37;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr64;
	goto st0;
tr64:
/* #line 99 "memcached/internal/proto_text_parser.rl" */
	{ s = p; }
	goto st38;
st38:
	if ( ++p == pe )
		goto _test_eof38;
case 38:
/* #line 848 "memcached/internal/proto_text_parser.c" */
	if ( (*p) == 32 )
		goto tr65;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st38;
	goto st0;
tr65:
/* #line 100 "memcached/internal/proto_text_parser.rl" */
	{
					if (memcached_strtoul(s, p, &req->bytes) == -1) {
						memcached_error_EINVALS("bad bytes value");
						con->close_connection = true;
						return -1;
					} else if (req->bytes > MEMCACHED_MAX_SIZE) {
						memcached_error_E2BIG();
						con->close_connection = true;
						return -1;
					}
				}
	goto st39;
st39:
	if ( ++p == pe )
		goto _test_eof39;
case 39:
/* #line 872 "memcached/internal/proto_text_parser.c" */
	if ( (*p) == 32 )
		goto st39;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr68;
	goto st0;
tr68:
/* #line 112 "memcached/internal/proto_text_parser.rl" */
	{ s = p; }
	goto st40;
st40:
	if ( ++p == pe )
		goto _test_eof40;
case 40:
/* #line 886 "memcached/internal/proto_text_parser.c" */
	switch( (*p) ) {
		case 10: goto tr69;
		case 13: goto tr70;
		case 32: goto tr71;
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
	if ( ++p == pe )
		goto _test_eof44;
case 44:
	if ( (*p) == 32 )
		goto tr77;
	goto st0;
tr77:
/* #line 162 "memcached/internal/proto_text_parser.rl" */
	{req->op = MEMCACHED_TXT_CMD_DECR;}
	goto st45;
tr140:
/* #line 161 "memcached/internal/proto_text_parser.rl" */
	{req->op = MEMCACHED_TXT_CMD_INCR;}
	goto st45;
st45:
	if ( ++p == pe )
		goto _test_eof45;
case 45:
/* #line 943 "memcached/internal/proto_text_parser.c" */
	switch( (*p) ) {
		case 13: goto st0;
		case 32: goto st45;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st0;
	goto tr78;
tr78:
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
/* #line 970 "memcached/internal/proto_text_parser.c" */
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
		goto tr81;
	goto st0;
tr81:
/* #line 121 "memcached/internal/proto_text_parser.rl" */
	{ s = p; }
	goto st48;
st48:
	if ( ++p == pe )
		goto _test_eof48;
case 48:
/* #line 991 "memcached/internal/proto_text_parser.c" */
	switch( (*p) ) {
		case 10: goto tr82;
		case 13: goto tr83;
		case 32: goto tr84;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st48;
	goto st0;
tr109:
/* #line 82 "memcached/internal/proto_text_parser.rl" */
	{
					if (memcached_strtoul(s, p, &req->exptime) == -1) {
						memcached_error_EINVALS("bad expiration time value");
						con->close_connection = true;
						return -1;
					}
				}
	goto st49;
tr97:
/* #line 142 "memcached/internal/proto_text_parser.rl" */
	{ req->noreply = true; }
	goto st49;
tr83:
/* #line 122 "memcached/internal/proto_text_parser.rl" */
	{
					if (memcached_strtoul(s, p, &req->delta) == -1) {
						memcached_error_DELTA_BADVAL();
						// memcached_error_EINVALS("bad incr/decr value");
						con->close_connection = true;
						return -1;
					}
				}
	goto st49;
tr121:
/* #line 165 "memcached/internal/proto_text_parser.rl" */
	{req->op = MEMCACHED_TXT_CMD_FLUSH;}
	goto st49;
tr126:
/* #line 132 "memcached/internal/proto_text_parser.rl" */
	{
					if (memcached_strtoul(s, p, &req->exptime) == -1) {
						memcached_error_EINVALS("bad flush value");
						con->close_connection = true;
						return -1;
					}
				}
	goto st49;
tr152:
/* #line 166 "memcached/internal/proto_text_parser.rl" */
	{req->op = MEMCACHED_TXT_CMD_QUIT;}
	goto st49;
tr168:
/* #line 164 "memcached/internal/proto_text_parser.rl" */
	{req->op = MEMCACHED_TXT_CMD_STATS;}
	goto st49;
st49:
	if ( ++p == pe )
		goto _test_eof49;
case 49:
/* #line 1051 "memcached/internal/proto_text_parser.c" */
	if ( (*p) == 10 )
		goto tr86;
	goto st0;
tr110:
/* #line 82 "memcached/internal/proto_text_parser.rl" */
	{
					if (memcached_strtoul(s, p, &req->exptime) == -1) {
						memcached_error_EINVALS("bad expiration time value");
						con->close_connection = true;
						return -1;
					}
				}
	goto st50;
tr84:
/* #line 122 "memcached/internal/proto_text_parser.rl" */
	{
					if (memcached_strtoul(s, p, &req->delta) == -1) {
						memcached_error_DELTA_BADVAL();
						// memcached_error_EINVALS("bad incr/decr value");
						con->close_connection = true;
						return -1;
					}
				}
	goto st50;
tr127:
/* #line 132 "memcached/internal/proto_text_parser.rl" */
	{
					if (memcached_strtoul(s, p, &req->exptime) == -1) {
						memcached_error_EINVALS("bad flush value");
						con->close_connection = true;
						return -1;
					}
				}
	goto st50;
st50:
	if ( ++p == pe )
		goto _test_eof50;
case 50:
/* #line 1090 "memcached/internal/proto_text_parser.c" */
	switch( (*p) ) {
		case 10: goto tr86;
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
		case 10: goto tr96;
		case 13: goto tr97;
		case 32: goto tr98;
	}
	goto st0;
tr98:
/* #line 142 "memcached/internal/proto_text_parser.rl" */
	{ req->noreply = true; }
	goto st58;
st58:
	if ( ++p == pe )
		goto _test_eof58;
case 58:
/* #line 1171 "memcached/internal/proto_text_parser.c" */
	switch( (*p) ) {
		case 10: goto tr86;
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
	if ( ++p == pe )
		goto _test_eof62;
case 62:
	if ( (*p) == 32 )
		goto tr103;
	goto st0;
tr103:
/* #line 160 "memcached/internal/proto_text_parser.rl" */
	{req->op = MEMCACHED_TXT_CMD_DELETE;}
	goto st63;
st63:
	if ( ++p == pe )
		goto _test_eof63;
case 63:
/* #line 1220 "memcached/internal/proto_text_parser.c" */
	switch( (*p) ) {
		case 13: goto st0;
		case 32: goto st63;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st0;
	goto tr104;
tr104:
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
/* #line 1247 "memcached/internal/proto_text_parser.c" */
	switch( (*p) ) {
		case 10: goto tr86;
		case 13: goto st49;
		case 32: goto st65;
	}
	goto st0;
st65:
	if ( ++p == pe )
		goto _test_eof65;
case 65:
	switch( (*p) ) {
		case 10: goto tr86;
		case 13: goto st49;
		case 32: goto st65;
		case 78: goto st51;
		case 110: goto st51;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr107;
	goto st0;
tr107:
/* #line 81 "memcached/internal/proto_text_parser.rl" */
	{ s = p; }
	goto st66;
st66:
	if ( ++p == pe )
		goto _test_eof66;
case 66:
/* #line 1276 "memcached/internal/proto_text_parser.c" */
	switch( (*p) ) {
		case 10: goto tr108;
		case 13: goto tr109;
		case 32: goto tr110;
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
	if ( ++p == pe )
		goto _test_eof75;
case 75:
	switch( (*p) ) {
		case 10: goto tr120;
		case 13: goto tr121;
		case 32: goto tr122;
	}
	goto st0;
tr122:
/* #line 165 "memcached/internal/proto_text_parser.rl" */
	{req->op = MEMCACHED_TXT_CMD_FLUSH;}
	goto st76;
st76:
	if ( ++p == pe )
		goto _test_eof76;
case 76:
/* #line 1373 "memcached/internal/proto_text_parser.c" */
	switch( (*p) ) {
		case 10: goto tr86;
		case 13: goto st49;
		case 32: goto st76;
		case 78: goto st51;
		case 110: goto st51;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr124;
	goto st0;
tr124:
/* #line 131 "memcached/internal/proto_text_parser.rl" */
	{ s = p; }
	goto st77;
st77:
	if ( ++p == pe )
		goto _test_eof77;
case 77:
/* #line 1392 "memcached/internal/proto_text_parser.c" */
	switch( (*p) ) {
		case 10: goto tr125;
		case 13: goto tr126;
		case 32: goto tr127;
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
	if ( ++p == pe )
		goto _test_eof80;
case 80:
	switch( (*p) ) {
		case 32: goto tr131;
		case 83: goto st84;
		case 115: goto st84;
	}
	goto st0;
tr131:
/* #line 158 "memcached/internal/proto_text_parser.rl" */
	{req->op = MEMCACHED_TXT_CMD_GET;}
	goto st81;
tr136:
/* #line 159 "memcached/internal/proto_text_parser.rl" */
	{req->op = MEMCACHED_TXT_CMD_GETS;}
	goto st81;
st81:
	if ( ++p == pe )
		goto _test_eof81;
case 81:
/* #line 1441 "memcached/internal/proto_text_parser.c" */
	switch( (*p) ) {
		case 13: goto st0;
		case 32: goto st81;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st0;
	goto tr133;
tr133:
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
/* #line 1468 "memcached/internal/proto_text_parser.c" */
	switch( (*p) ) {
		case 10: goto tr86;
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
		case 10: goto tr86;
		case 13: goto st49;
		case 32: goto st83;
	}
	goto tr133;
st84:
	if ( ++p == pe )
		goto _test_eof84;
case 84:
	if ( (*p) == 32 )
		goto tr136;
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
	if ( ++p == pe )
		goto _test_eof88;
case 88:
	if ( (*p) == 32 )
		goto tr140;
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
	if ( ++p == pe )
		goto _test_eof95;
case 95:
	if ( (*p) == 32 )
		goto tr147;
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
	if ( ++p == pe )
		goto _test_eof99;
case 99:
	switch( (*p) ) {
		case 10: goto tr151;
		case 13: goto tr152;
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
	if ( ++p == pe )
		goto _test_eof106;
case 106:
	if ( (*p) == 32 )
		goto tr159;
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
	if ( ++p == pe )
		goto _test_eof109;
case 109:
	if ( (*p) == 32 )
		goto tr163;
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
	if ( ++p == pe )
		goto _test_eof113;
case 113:
	switch( (*p) ) {
		case 10: goto tr167;
		case 13: goto tr168;
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
	_test_eof114: cs = 114; goto _test_eof; 
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

	_test_eof: {}
	_out: {}
	}

/* #line 174 "memcached/internal/proto_text_parser.rl" */



	if (!done) {
		if (p == pe) {
			return 1;
		}
		
		if (box_error_last() == NULL) {
			if (con->request.op == MEMCACHED_TXT_CMD_UNKNOWN) {
				memcached_error_UNKNOWN_COMMAND(con->request.op);
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
		}
		return -1;
	}
	*p_ptr = (p - 1);
	return 0;
}

/* vim: set ft=ragel noexpandtab ts=4 : */
