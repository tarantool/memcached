# memcached

Memcached protocol 'wrapper' for tarantool.

## Getting started

### Prerequisites

 * Tarantol 1.6.7+ with header files (tarantool && tarantool-dev packages). 
   Use branch '1.6' on github or http://tarantool.org/dist/1.6 
 * Python >= 2.7, <3 with next packages (for testing only):
   - gevent
   - geventconnpool
   - PyYAML
   - msgpack-python
   - six==1.9.0

### Installation

Clone this repository and then build it using CMake:

``` bash
git clone https://github.com/tarantool/memcached.git
cd memcached && git submodule update --init --recursive
cd memcached && cmake . -DCMAKE_BUILD_TYPE=RelWithDebugInfo
make
make install
```

Or you can also use LuaRocks (in that case you'll need libsmall and libsmall-dev
packages that are available from our repository at http://tarantool.org/dist/1.6):

``` bash
luarocks install https://raw.githubusercontent.com/tarantool/memcached/master/memcached-scm-1.rockspec --local
```

### Usage

``` bash
box.cfg{}
local memcached = require('memcached')
local instance = memcached.create('my_instance', '0.0.0.0:11211')
```

Now you're set up and ready to go!

## API

* `local memcached = require('memcached')` - acquire library for your use
* `local instance = memcached.create(<name>, <uri>, <opts>)` - create a new instance, register it and run 
  `name` - a string with instance name
  `uri`  - a string with uri to bind to, for example: `0.0.0.0:11211`
  `opts` - a table with options, the list of possible options is described in the configuration configuration section
* `local instance = instance:cfg(<opts>)` - configure an existing instance. 
  `opts` - table with options that are in configuration section
* `local instance = instance:start()` - start an instance
* `local instance = instance:stop()` - stop an instance
* `local instance = instance:info()` - return executions statistics 

## Configuration

* *readahead* - (default) size of readahead buffer for connection. default is `box->cfg->readahead`
* *expire_enabled* - availability of expiration daemon. default is `true`.
* *expire_items_per_iter* - scan count for expiration (that's processed in one transaction). default is 200.
* *expire_full_scan_time* - time required for full index scan (in seconds). default is 3600
* *verbosity* - verbosity of memcached logging. default is 0.
* ~~*flush_enabled* - flush command availability. default is true~~

## Caution

This rock is in early beta.
