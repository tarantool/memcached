#!/usr/bin/env tarantool

-- local mc = require('memcached')

require('log').info(
    require('yaml').encode({pcall(require, 'mc')}))


require('log').info('%s', tostring(mc))

box.cfg{}

local inst = require('memcached').create('memcached',
    os.getenv('LISTEN'):match(':(.*)'), {
        expire_full_scan_time = 1
})

require('log').info(os.getenv('LISTEN'):match(':(.*)'))

require('console').listen(os.getenv('ADMIN'))

box.schema.user.grant('guest', 'read,write,execute', 'universe')
