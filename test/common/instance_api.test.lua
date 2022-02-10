#!/usr/bin/env tarantool

local socket = require('socket')
local tap = require('tap')
package.cpath = './?.so;' .. package.cpath
local memcached = require('memcached')
local test = tap.test('memcached instance api')

local function is_port_open(port)
    local sock, _ = socket.tcp_connect('127.0.0.1', port)
    if sock == nil then
        return false
    end
    return true
end

if type(box.cfg) == 'function' then
    box.cfg{
        wal_mode = 'none',
        memtx_memory = 100 * 1024 * 1024,
    }
    box.schema.user.grant('guest', 'read,write,execute', 'universe')
end

test:plan(50)

-- memcached.create()

local mc_port = 11211
local mc_name = 'memcached_xxx'
local mc_space_name = 'memcached_xxx_space'

local mc = memcached.create(mc_name, tostring(mc_port), {
    space_name = mc_space_name
})
test:isnt(mc, nil, 'memcached instance object is not nil')
test:is(is_port_open(mc_port), true, 'memcached instance is started')

-- instance:stop()

local self_instance = mc:stop()
test:is(self_instance, mc, 'instance:stop() returns self object')
test:is(is_port_open(mc_port), false, 'port is closed when memcached instance is stopped')
local ok, err = pcall(mc.stop, mc)
test:is(ok, false, 'instance:stop() of stopped instance is failed')
test:like(err:gsub("^.+:%d+: ", ""), ("Memcached instance '%s' is already stopped"):format(mc_name),
    'instance:stop() of stopped instance shows a correct error message')

-- instance:start()

local self_instance = mc:start()
test:is(self_instance, mc, 'instance:start() returns self object')
test:is(is_port_open(mc_port), true, 'instance:start() opens a port')
local ok, err = pcall(mc.start, mc)
test:is(ok, false, 'instance:start() of started instance is failed')
test:like(err:gsub("^.+:%d+: ", ""), ("Memcached instance '%s' is already started"):format(mc_name),
    'instance:start() of started instance shows a correct error message')

-- instance:info()

local instance_info = mc:info()
test:istable(instance_info, 'type of instance:info() is a table')
test:is(instance_info.delete_misses, 0, 'default value of delete_misses is correct')
test:is(instance_info.get_hits, 0, 'default value of get_hits is correct')
test:is(instance_info.bytes_written, 0, 'default value of bytes_written is correct')
test:is(instance_info.curr_conns, 0, 'default value of curr_conns is correct')
test:is(instance_info.total_items, 0, 'default value of total_items is correct')
test:is(instance_info.evictions, 0, 'default value of evictions is correct')
test:is(instance_info.auth_errors, 0, 'default value of auth_errors is correct')
test:is(instance_info.auth_cmds, 0, 'default value of auth_cmds is correct')
test:is(instance_info.reclaimed, 0, 'default value of reclaimed is correct')
test:is(instance_info.cmd_incr, 0, 'default value of cmd_incr is correct')
test:is(instance_info.cmd_decr, 0, 'default value of cmd_decr is correct')
test:is(instance_info.cmd_touch, 0, 'default value of cmd_touch is correct')
test:is(instance_info.touch_hits, 0, 'default value of touch_hits is correct')
test:is(instance_info.touch_misses, 0, 'default value of touch_misses is correct')
test:is(instance_info.decr_misses, 0, 'default value of decr_misses is correct')
test:is(instance_info.curr_items, 0, 'default value of curr_items is correct')
test:is(instance_info.total_conns, 0, 'default value of total_conns is correct')
test:is(instance_info.incr_misses, 0, 'default value of incr_misses is correct')
test:is(instance_info.cmd_flush, 0, 'default value of cmd_flush is correct')
test:is(instance_info.decr_hits, 0, 'default value of decr_hits is correct')
test:is(instance_info.incr_hits, 0, 'default value of incr_hits is correct')
test:is(instance_info.cmd_delete, 0, 'default value of cmd_delete is correct')
test:is(instance_info.cas_misses, 0, 'default value of cas_misses is correct')
test:is(instance_info.bytes_read, 0, 'default value of bytes_read is correct')
test:is(instance_info.cas_badval, 0, 'default value of cas_badval is correct')
test:is(instance_info.delete_hits, 0, 'default value of delete_hits is correct')
test:is(instance_info.cmd_get, 0, 'default value of cmd_get is correct')
test:is(instance_info.cas_hits, 0, 'default value of cas_hits is correct')
test:is(instance_info.cmd_set, 0, 'default value of cmd_set is correct')
test:is(instance_info.get_misses, 0, 'default value of get_misses is correct')

-- instance:cfg()

local updated_expire_full_scan_time = mc.opts.expire_full_scan_time + 100
local updated_expire_items_per_iter = mc.opts.expire_items_per_iter + 50
local updated_expire_enabled = not mc.opts.expire_enabled
local updated_protocol = 'binary'
local updated_name = mc.opts.name .. '_hello'
local updated_verbosity = mc.opts.verbosity + 1
local updated_sasl = not mc.opts.sasl

local self_instance = mc:cfg({
    expire_full_scan_time = updated_expire_full_scan_time,
    expire_items_per_iter = updated_expire_items_per_iter,
    expire_enabled = updated_expire_enabled,
    protocol = updated_protocol,
    name = updated_name,
    verbosity = updated_verbosity,
    sasl = updated_sasl,
})
test:is(self_instance, mc, 'instance:cfg() returns a self object')
test:is(is_port_open(mc_port), true, 'instance:cfg() keeps port opened')
test:is(mc.opts.expire_full_scan_time, updated_expire_full_scan_time,
    'instance:cfg() updated expire_full_scan_time correctly')
test:is(mc.opts.expire_items_per_iter, updated_expire_items_per_iter,
    'instance:cfg() updated expire_items_per_iter correctly')
test:is(mc.opts.expire_enabled, updated_expire_enabled,
    'instance:cfg() updated expire_enabled correctly')
test:is(mc.opts.protocol, updated_protocol, 'instance:cfg() updated protocol correctly')
test:is(mc.opts.verbosity, updated_verbosity, 'instance:cfg() updated verbosity correctly')
test:is(mc.opts.sasl, updated_sasl, 'instance:cfg() updated sasl correctly')

-- instance:grant()

box.schema.user.create('Vera')
local self_instance = mc:grant('Vera')
test:is(self_instance, mc, 'instance:grant() returns self object')

os.exit(test:check() and 0 or 1)
