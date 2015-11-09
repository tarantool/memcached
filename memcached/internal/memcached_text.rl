#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "memcached_text.h"

%%{
    machine memcached;
    write data;
}%%

#if defined(__APPLE__)
#  define MC_EXPORT __attribute__((visibility("default")))
#elif defined(_WIN32) || defined(__WIN32__)
#  define MC_EXPORT __declspec(dllexport)
#else
#  define MC_EXPORT
#endif

static int
mc_strtol(const char *start, const char *end, int64_t *num)
{
    *num = 0;
    char sign = 1;
    if (*start == '-') { sign *= -1; start++; }
    while (start < end) {
        uint8_t code = *start++;
        if (code < '0' || code > '9')
            return -1;
        *num = (*num) * 10 + (code - '0');
    }
    return 0;
}

static int
mc_strtoul(const char *start, const char *end, uint64_t *num)
{
    *num = 0;
    while (start < end) {
        uint8_t code = *start++;
        if (code < '0' || code > '9')
            return -1;
        *num = (*num) * 10 + (code - '0');
    }
    return 0;
}

MC_EXPORT int
mc_parse(struct mc_request *req, const char **p_ptr, const char *pe) {
    const char *p = *p_ptr;
    int cs = 0;
    const char *s = NULL;
    bool done = false;

    memset(req, 0, sizeof(struct mc_request));
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
                    return -7;
                }
                p += req->bytes + 2;
            } else {
                return (req->data_len + 2) - (pe - req->data);
            }
        }
        action done {
            *p_ptr = p;
            done = true;
        }
        printable = [^ \t\r\n];
        key = printable >key_start ;

        exptime = digit+
                >{ s = p; }
                %{ if (mc_strtoul(s, p, &req->exptime) == -1) return -2; };
        flags = digit+
                >{ s = p; }
                %{ if (mc_strtoul(s, p, &req->flags) == -1) return -3; };
        bytes = digit+
                >{ s = p; }
                %{ if (mc_strtoul(s, p, &req->bytes) == -1) return -4; };
        cas_value = digit+
                >{ s = p; }
                %{ if (mc_strtoul(s, p, &req->cas) == -1) return -5; };
        incr_value = digit+
                >{ s = p; }
                %{ if (mc_strtoul(s, p, &req->inc_val) == -1) return -6; };
        flush_delay = digit+
                >{ s = p; }
                %{ if (mc_strtoul(s, p, &req->exptime) == -1) return -7; };

        eol = ("\r\n" | "\n") @{ p++; };
        spc = " "+;
        noreply = (spc "noreply"i %{ req->noreply = true; })?;

        store_body = spc key spc flags spc exptime spc bytes               noreply spc? eol;
        cas_body   = spc key spc flags spc exptime spc bytes spc cas_value noreply spc? eol;
        get_body   = (spc key)+                                                    spc? eol;
        del_body   = spc key (spc exptime)?                                noreply spc? eol;
        cr_body    = spc key spc incr_value                                noreply spc? eol;
        flush_body = (spc flush_delay)?                                    noreply spc? eol;

        set     = ("set"i     %{req->op = MC_SET;}     store_body) @read_data @done;
        add     = ("add"i     %{req->op = MC_ADD;}     store_body) @read_data @done;
        replace = ("replace"i %{req->op = MC_REPLACE;} store_body) @read_data @done;
        append  = ("append"i  %{req->op = MC_APPEND;}  store_body) @read_data @done;
        prepend = ("prepend"i %{req->op = MC_PREPEND;} store_body) @read_data @done;
        cas     = ("cas"i     %{req->op = MC_CAS;}     cas_body)   @read_data @done;

        get     = ("get"i    %{req->op = MC_GET;}    get_body) @done;
        gets    = ("gets"i   %{req->op = MC_GETS;}   get_body) @done;
        delete  = ("delete"i %{req->op = MC_DELETE;} del_body) @done;
        incr    = ("incr"i   %{req->op = MC_INCR;}   cr_body)  @done;
        decr    = ("decr"i   %{req->op = MC_DECR;}   cr_body)  @done;

        stats     = "stats"i     %{req->op = MC_STATS;} eol        @done;
        flush_all = "flush_all"i %{req->op = MC_FLUSH;} flush_body @done;
        quit      = "quit"i      %{req->op = MC_QUIT;}  eol        @done;

        main := set | add | replace | append | prepend | cas |
                get | gets | delete | incr | decr |
                stats | flush_all | quit;

        write init;
        write exec;
    }%%


    if (!done) {
        if (p == pe)
            return 1;
        return -1;
    }
    return 0;
}

LUA_API int
luaopen_memcached_libparser(lua_State *L)
{
    return 0;
}

/* vim: set ft=ragel: */
