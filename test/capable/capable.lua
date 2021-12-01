#!/usr/bin/env tarantool

box.cfg{
    wal_mode         = 'none',
    memtx_memory = 100 * 1024 * 1024,
}

package.cpath = './?.so;' .. package.cpath

require('memcached').create(
    'memcached',
    os.getenv('LISTEN'):match(':(.*)')
)

require('console').listen(os.getenv('ADMIN'))

box.schema.user.grant('guest', 'read,write,execute', 'universe')
