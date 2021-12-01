#!/usr/bin/env tarantool

box.cfg{
    wal_mode         = 'none',
    memtx_memory     = 100 * 1024 * 1024,
    log_nonblock  = false,
}

package.cpath = './?.so;' .. package.cpath

require('memcached').create('memcached', '0.0.0.0:11211', {
    expire_full_scan_time = 120
})

-- box.schema.user.grant('guest', 'read,write,execute', 'universe')
