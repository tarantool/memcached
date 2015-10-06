#!/usr/bin/env tarantool

box.cfg{
    wal_mode         = 'none',
    slab_alloc_arena = 0.1,
}

local inst = require('memcached').create{
    name = 'memcached',
    uri  = '0.0.0.0:11211',
--    uri  = os.getenv('LISTEN'):match(':(.*)'),
    expire_full_scan_time = 120
}

-- box.schema.user.grant('guest', 'read,write,execute', 'universe')
