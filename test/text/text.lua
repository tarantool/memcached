#!/usr/bin/env tarantool

box.cfg{
    wal_mode = 'none',
    memtx_memory = 100 * 1024 * 1024,
}

package.cpath = './?.so;' .. package.cpath

-- The current version of test-run returns port zero in the LISTEN variable.
-- Since memcache has not yet implemented an API for receiving a real listen
-- port, it will have to be hardcoded.
require('memcached').create('memcached',
    "20509", {
        expire_full_scan_time = 1
})

-- require('log').info(os.getenv('LISTEN'):match(':(.*)'))

require('console').listen(os.getenv('ADMIN'))

box.schema.user.grant('guest', 'read,write,execute', 'universe')
