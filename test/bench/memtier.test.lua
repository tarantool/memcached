#!/usr/bin/env tarantool

local tap = require('tap')
local fio = require('fio')
local has_popen, popen = pcall(require, 'popen')
package.cpath = './?.so;' .. package.cpath
local memcached = require('memcached')
local test = tap.test('memcached benchmarks')

if not has_popen then
    test:plan(0)
    os.exit(0)
end

test:plan(3)

local is_test_run = os.getenv('LISTEN')

if type(box.cfg) == 'function' then
    box.cfg{
        wal_mode = 'none',
        memtx_memory = 100 * 1024 * 1024,
    }
    box.schema.user.grant('guest', 'read,write,execute', 'universe')
end

local port = 11211
local mc = memcached.create('memcached', tostring(port), {})

local function run_memtier(instance, memtier_path, proto)
    instance:cfg({
        protocol = proto,
    })
    local memtier_proto = 'memcache_' .. proto
    local memtier_argv = {
        memtier_path,
        '--server=127.0.0.1',
        string.format('--port=%d', port),
        string.format('--protocol=%s', memtier_proto),
        '--threads=10',
        '--test-time=2',
        '--hide-histogram'
    }

    local stdout = is_test_run and popen.opts.DEVNULL or popen.opts.INHERIT
    local stderr = is_test_run and popen.opts.DEVNULL or popen.opts.INHERIT
    local ph = popen.new(memtier_argv, {
        stdout = stdout,
        stderr = stderr,
    })
    local res = ph:wait()
    ph:close()

    return res.exit_code
end

-- path to memtier_benchmark in case of tarantool that run in project root dir
local memtier_path = 'test/bench/memtier_benchmark'
if not fio.path.exists(memtier_path) then
    -- path to memtier_benchmark in case of test-run
    memtier_path = './memtier_benchmark'
end

test:is(fio.path.exists(memtier_path), true, 'memtier_benchmark binary is available')
test:is(run_memtier(mc, memtier_path, 'text'), 0, 'memtier with text protocol')
test:is(run_memtier(mc, memtier_path, 'binary'), 0, 'memtier with binary protocol')

os.exit(test:check() and 0 or 1)
