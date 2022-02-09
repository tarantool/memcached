#!/usr/bin/env tarantool

local socket = require('socket')
local tap = require('tap')
package.cpath = './?.so;' .. package.cpath
local memcached = require('memcached')
local test = tap.test('memcached default values')

local function is_port_open(port)
    local sock, _ = socket.tcp_connect('127.0.0.1', port)
    return sock:sysconnect(0, port) and true or false
end

if type(box.cfg) == 'function' then
    box.cfg{
        wal_mode = 'none',
        memtx_memory = 100 * 1024 * 1024,
    }
    box.schema.user.grant('guest', 'read,write,execute', 'universe')
end

test:plan(22)

local mc_port = 11211
local mc_name = 'memcached_xxx'
local mc_space_name = 'memcached_xxx_space'

local mc = memcached.create(mc_name, tostring(mc_port), {
    space_name = mc_space_name
})
test:isnt(mc, nil, 'memcached instance object is not nil')
test:is(is_port_open(mc_port), true, 'memcached instance is started')

test:isnt(mc.listener, nil, 'memcached instance is started')
test:istable(mc.listener, 'type of instance.listener is a table')
test:is(mc.uri, tostring(mc_port), 'instance.uri is equal to port number specified by user')
test:istable(mc.opts, 'type ofinstance.opts is a table in memcached instance object')
test:is(mc.opts.if_not_exists, false, 'instance.opts.if_not_exists is equal to false by default')
test:is(mc.opts.protocol, 'negotiation', 'instance.opts.protocol is equal to "negotiation" by default')
test:is(mc.opts.expire_full_scan_time, 3600, 'instance.opts.expire_full_scan_time is equal to 3600 by default')
test:is(mc.opts.expire_items_per_iter, 200, 'instance.opts.expire_items_per_iter is equal to 200 by default')
test:is(mc.opts.readahead, 16320, 'instance.opts.readahead is equal to 16320 by default')
test:is(mc.opts.uri, '0.0.0.0:11211', 'instance.opts.uri is equal to "0.0.0.0:11211" by default')
test:is(mc.opts.storage, 'memory', 'instance.opts.storage is equal to "memory" by default')
test:is(mc.space.engine, 'memtx', 'default space engine is memtx')
test:is(mc.opts.name, 'memcached', 'instance.opts.name is equal to "memcached"')
test:is(mc.opts.verbosity, 0, 'instance.opts.verbosity is equal to 0 by default')
test:is(mc.opts.sasl, false, 'instance.opts.sasl is false by default')
test:is(mc.opts.expire_enabled, true, 'instance.opts.expire_enabled is true by default')
test:is(mc.status, 'r', 'instance.status is equal to "r" by default')
test:is(mc.name, mc_name, 'instance.name is equal to specified by user')
test:is(mc.space_name, mc_space_name, 'instance.space_name is equal to specified name')
test:isnt(mc.service, nil, 'instance.service is not nil')

os.exit(test:check() and 0 or 1)
