#!/usr/bin/env tarantool

box.cfg{
    wal_mode = 'none',
    memtx_memory = 100 * 1024 * 1024,
}

require('console').listen(os.getenv('ADMIN'))

box.schema.user.grant('guest', 'read,write,execute', 'universe')
