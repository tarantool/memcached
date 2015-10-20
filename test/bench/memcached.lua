#!/usr/bin/env tarantool

box.cfg{
    wal_mode         = 'none',
    slab_alloc_arena = 0.1,
    log_level        = 7,
    logger_nonblock  = false,
}

local inst = require('memcached').create('memcached', '0.0.0.0:11211', {
    expire_full_scan_time = 120
})

-- box.schema.user.grant('guest', 'read,write,execute', 'universe')
