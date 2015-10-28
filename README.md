# memcached

Memcached protocol 'wrapper' for tarantool.

## Getting started

### Prerequisites

 * Tarantol 1.6.7+ with header files (tarantool && tarantool-dev packages)
 * Python >= 2.7, <3 with next packages (for testing only):
   - gevent
   - geventconnpool
   - PyYAML
   - msgpack-python
   - six==1.9.0

### Installation

Clone repository and then build it using CMake:

``` bash
git clone https://github.com/tarantool/memcached.git
cd memcached && cmake . -DCMAKE_BUILD_TYPE=RelWithDebugInfo
make
make install
```

Or you can also use LuaRocks (in that case you'll need libsmall and libsmall-dev
packages that are available from our repository):

``` bash
luarocks install https://raw.githubusercontent.com/tarantool/memcached/master/memcached-scm-1.rockspec --local
```

### Usage

``` bash
box.cfg{}
local memcached = require('memcached')
local inst = memcached.create('mc_name', '0.0.0.0:11211')
```

Now you're set up and ready to go!

## API

* `local memcached = require('memcached')` - acquire library for your use
* `local instance = memcached.create(<name>, <uri>, <opts>)` - create instance, register it and run 
  `name` - string with instance name
  `uri`  - string with uri to bind on (example: `0.0.0.0:11211`)
  `opts` - table with options that are in configuration section
* `local instance = instance:cfg(<opts>)` - configure existing instance. 
  `opts` - table with options that are in configuration section
* `local instance = instance:start()` - start existing instance
* `local instance = instance:stop()` - stop existing instance
* `local instance = instance:info()` - return stat

## Configuration

* *readahead* - (default) size of readahead buffer for connection. default is `box->cfg->readahead`
* *expire_enabled* - availability of expiration daemon. default is `true`.
* *expire_items_per_iter* - scan count for expiration (that's processed in one transaction). default is 200.
* *expire_full_scan_time* - time required for full index scan (in seconds). default is 3600
* *verbosity* - verbosity of memcached logging. default is 0.
* ~~*flush_enabled* - flush command availability. default is true~~

## Conclusion

This product is in early beta, use it at you own risk.
