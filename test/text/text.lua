#!/usr/bin/env tarantool

box.cfg{
    wal_mode = 'none',
    slab_alloc_arena = 0.1,
}

local inst = require('memcached').create('memcached',
    os.getenv('LISTEN'):match(':(.*)'), {
        expire_full_scan_time = 1
})

-- require('log').info(os.getenv('LISTEN'):match(':(.*)'))

require('console').listen(os.getenv('ADMIN'))

box.schema.user.grant('guest', 'read,write,execute', 'universe')
