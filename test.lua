box.cfg{
    wal_mode="none"
}

local mc = require('memcached')
local inst = mc.create('memcached', '0.0.0.0:11211', {})
require('log').info(tostring(inst))
