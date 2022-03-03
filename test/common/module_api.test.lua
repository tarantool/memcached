#!/usr/bin/env tarantool

local socket = require('socket')
local tap = require('tap')
local ffi = require('ffi')
package.cpath = './?.so;' .. package.cpath
local memcached = require('memcached')
local test = tap.test('memcached module api')
local abs = require('math').abs

-- 1. Open connection to memcached instance.
-- 2. Set a key 'key' to value 5.
-- 3. Get a value of key 'key'.
-- 4. Make sure value is equal to 5.
-- 5. Close connection.
local function check_memcached(port, timeout)
    timeout = timeout or 0.5
    local sock, err = socket.tcp_connect('127.0.0.1', port, timeout)
    if sock == nil then
        print(err)
        return false
    end
    sock:nonblock(true)

    -- Set a value.
    local cmd = 'set key 0 60 5\r\nvalue\r\n'
    local send = sock:send(cmd)
    assert(send > 0)

    -- Get a value.
    send = sock:send('get key\r\n')
    assert(send > 0)
    local read = sock:read(23)
    local v = read:match('VALUE key 0 ([0-9])')
    assert(v == '5')

    -- Close connection.
    sock:close()

    return tonumber(v) == 5
end

if type(box.cfg) == 'function' then
    box.cfg{
        wal_mode = 'none',
        memtx_memory = 100 * 1024 * 1024,
    }
    box.schema.user.grant('guest', 'read,write,execute', 'universe')
end

test:plan(31)

-- memcached.server: module is initialized, no instances

test:istable(memcached.server, 'type of memcached.server is a table')
test:is(#memcached.server, 0, 'memcached.server is empty')

-- memcached.slab(): memcached module is initialized, no instances

test:istable(memcached.slab, 'memcached.slab (no instances) returns a table')
test:isfunction(memcached.slab.info, 'memcached.slab.info() (no instances) returns a function')
local slab_info = memcached.slab.info()
test:iscdata(slab_info.quota_size, ffi.typeof('uint64_t'),
    'memcached.slab.info().quota_size (no instances) returns a cdata')
test:is(slab_info.quota_size, 4398046510080, 'memcached.slab.info().quota_size (no instances) is correct')
test:iscdata(slab_info.quota_used, ffi.typeof('uint64_t'),
    'memcached.slab.info().quota_used (no instances) returns a cdata')
test:is(slab_info.quota_used, 0, 'memcached.slab.info().quota_used (no instances) is correct')
test:isstring(slab_info.quota_used_ratio, 'memcached.slab.info().quota_used_ratio (no instances) returns a string')

-- memcached.create(): instance 1

local mc_1_port = 11211
local mc_1_name = 'memcached_1_xxx'
local mc_1_space_name = 'memcached_1_xxx_space'

local mc_1 = memcached.create(mc_1_name, tostring(mc_1_port), {
    space_name = mc_1_space_name
})
test:isnt(mc_1, nil, '1st memcached instance object is not nil')
test:is(check_memcached(mc_1_port), true, 'connected to 1st memcached instance')
mc_1:stop()

-- memcached.create(): instance 2

local mc_2_port = 11212
local mc_2_name = 'memcached_2_xxx'
local mc_2_space_name = 'memcached_2_xxx_space'

local mc_2 = memcached.create(mc_2_name, tostring(mc_2_port), {
    space_name = mc_2_space_name
})
test:isnt(mc_2, nil, '2nd memcached instance object is not nil')
test:is(check_memcached(mc_2_port), true, 'connected to 2nd memcached instance')
mc_2:stop()

-- memcached.server with created and started instances

mc_1:start()
mc_2:start()

test:istable(memcached.server, 'type of memcached.server is a table')
test:is(#memcached.server, 0, 'memcached.server is empty')

mc_1:stop()
mc_2:stop()

-- memcached.get()

test:is(memcached.get(), nil, 'memcached.get() without arguments returns nil')

-- memcached.get(instance_1_name)

local instance_1_info = memcached.get(mc_1_name)
test:is(instance_1_info.name, mc_1.name, 'memcached.get(): name of instance 1 is correct')
test:is(instance_1_info.space_name, mc_1.space_name, 'memcached.get(): space name of instance 1 is correct')
test:is(instance_1_info.space.id, mc_1.space.id, 'memcached.get(): space id of instance 1 is correct')

-- memcached.get(instance_2_name)

local instance_2_info = memcached.get(mc_2_name)
test:is(instance_2_info.name, mc_2.name, 'memcached.get(): name of instance 2 is correct')
test:is(instance_2_info.space_name, mc_2.space_name, 'memcached.get(): space name of instance 2 is correct')
test:is(instance_2_info.space.id, mc_2.space.id, 'memcached.get(): space id of instance 2 is correct')

-- memcached.slab(): check numbers with created memcached instances

mc_1:start()
mc_2:start()

test:istable(memcached.slab, 'memcached.slab returns a table')
test:isfunction(memcached.slab.info, 'memcached.slab.info() returns a function')
local slab_info = memcached.slab.info()
test:istable(slab_info, 'memcached.slab.info() returns a table')
test:iscdata(slab_info.quota_size, ffi.typeof('uint64_t'), 'memcached.slab.info().quota_size returns a cdata')
test:is(slab_info.quota_size, 4398046510080, 'memcached.slab.info().quota_size is correct')
test:iscdata(slab_info.quota_used, ffi.typeof('uint64_t'), 'memcached.slab.info().quota_used returns a cdata')
test:is(slab_info.quota_used, 4194304, 'memcached.slab.info().quota_used is correct')
test:isstring(slab_info.quota_used_ratio, 'memcached.slab.info().quota_used_ratio returns a string')
local expected_ratio = tonumber(100 * slab_info.quota_used / slab_info.quota_size)
local actual_ratio = tonumber(slab_info.quota_used_ratio:match('^(%d.%d+)%%'))
local eps_is_correct = abs(actual_ratio - expected_ratio) < 1
test:is(eps_is_correct, true, 'memcached.slab.info().quota_used_ratio is correct')

mc_1:stop()
mc_2:stop()

os.exit(test:check() and 0 or 1)
