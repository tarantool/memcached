#!/usr/bin/env tarantool

package.cpath = './?.so;' .. package.cpath
--------------------------------------------------------------------------------
--                    Manipulating environment variables                      --
--------------------------------------------------------------------------------
local ffi = require('ffi')
local log = require('log')
local errno = require('errno')

ffi.cdef[[
    extern char **environ;

    int   setenv(const char *name, const char *value, int overwrite);
    int   unsetenv(const char *name);
    int   clearenv(void);
    char *getenv(const char *name);
]]

local env = setmetatable({}, {
    __call = function()
        local environ = ffi.C.environ
        if not environ then
            return nil
        end
        local r = {}
        local i = 0
        while environ[i] ~= 0 do
            local e = ffi.string(environ[i])
            local eq = e:find('=')
            if eq then
                r[e:sub(1, eq - 1)] = e:sub(eq + 1)
            end
            i = i + 1
        end
        return r
    end,
    __index = function(self, key)
        local var = ffi.C.getenv(key)
        if var == 0 then
            return nil
        end
        return ffi.string(var)
    end,
    __newindex = function(self, key, value)
        local rv = nil
        if value ~= nil then
            rv = ffi.C.setenv(key, value, 1)
        else
            rv = ffi.C.unsetenv(key)
            end
        if rv == -1 then
            error(string.format('error %d: %s', errno(), errno.errstring()))
        end
    end
})

--------------------------------------------------------------------------------

local fio = require('fio')

local sasldb = '/tmp/test-tarantool-memcached.sasldb'
fio.unlink(sasldb)
-- os.execute('echo testpass | saslpasswd2 -a tarantool-memcached -c -p testuser -f ' .. sasldb)
fio.symlink('sasl/sasl.db', sasldb)

env['SASL_CONF_PATH'] = fio.pathjoin(fio.cwd(), '../sasl/config/')

box.cfg{
    wal_mode = 'none',
    slab_alloc_arena = 0.1,
}

local memcached = require('memcached')

local listen_port = env['LISTEN']:match(':(.*)')
local admin_port  = env['ADMIN']

local inst = memcached.create('memcached', listen_port, {
    expire_full_scan_time = 1,
    sasl = true
}):grant('guest')

require('console').listen(admin_port)
