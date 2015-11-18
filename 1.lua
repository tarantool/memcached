require('console').listen('0.0.0.0:11212')
box.cfg{ wal_mode = 'none' }
local memcached = require('memcached')
local inst = memcached.create('memcached', '0.0.0.0:11211', { if_not_exists = true })
