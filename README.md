# memcached [![Build Status](https://travis-ci.org/tarantool/memcached.png?branch=master)](https://travis-ci.org/tarantool/memcached)

Memcached protocol 'wrapper' for tarantool.

## Getting started

### Prerequisites

 * Tarantol 1.6.7+ with header files (tarantool && tarantool-dev packages).
 * Python >= 2.7, <3 with next packages (for testing only):
   - PyYAML
   - msgpack-python
   - six==1.9.0

### Installation

Clone this repository and then build it using CMake:

``` bash
git clone https://github.com/tarantool/memcached.git
cd memcached && git submodule update --init --recursive
cd memcached && cmake . -DCMAKE_BUILD_TYPE=RelWithDebugInfo -DCMAKE_INSTALL_PREFIX=/usr
make
make install
```

Or use LuaRocks (in this case you'll need `libsmall`, `libsmall-dev` and `tarantool-dev`
packages available from our binary repository at http://tarantool.org/dist/master):

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

### How to connect

Install tarantool package from repository ([described here](http://tarantool.org/download.html)).

Paste the previous example to `/etc/tarantool/instances.enabled/memcached.lua` and start it with
`tarantoolctl start memcached`.

Then try the following example:

``` session
$ printf "set key 0 60 5\r\nvalue\r\n" | nc localhost 11211
STORED
$ printf "get key\r\n" | nc localhost 11211
VALUE key 0 5
value
END
$ printf "set key2 0 60 6\r\nvalue2\r\n" | nc localhost 11211
STORED
$ printf "get key key2\r\n" | nc localhost 11211
VALUE key 0 5
value
VALUE key2 0 6
value2
END
```

## API

* `local memcached = require('memcached')` - acquire a library  andle
* `local instance = memcached.create(<name>, <uri>, <opts>)` - create a new instance, register it and run
  - `name` - a string with instance name
  - `uri`  - a string with uri to bind to, for example: `0.0.0.0:11211` (only TCP is supported now)
  - `opts` - a table with options, the list of possible options is described in the configuration section
* `local instance = instance:cfg(<opts>)` - (re)configure an existing instance.
  `opts` - a table with options, same as in `create`
* `local instance = instance:start()` - start an instance
* `local instance = instance:stop()` - stop an instance
* `local instance = instance:info()` - return execution statistics

## Configuration

* *readahead* - (default) size of readahead buffer for connection. default is `box->cfg->readahead`
* *expire_enabled* - availability of expiration daemon. default is `true`.
* *expire_items_per_iter* - scan count for expiration (tuples processed in one transaction). default is 200.
* *expire_full_scan_time* - time required for a full index scan (in seconds). defaiult is 3600
* *verbosity* - verbosity of memcached logging. default is 0.
* ~~*flush_enabled* - flush command availability. default is true~~
* *proto* - the protocol, one of `negotiation`, `binary` or `text`).
  - `negotiation` - detect the protocol automatically at handshake (the default)
  - `binary` - binary memcached protocol
  - `text` - text memcached protocol
* *engine* - the engine to store data in
  - `memory` - store everything in memory. (using `memtx` engine)
  - ~~`disk` - store everything on hdd/ssd (using `sophia` engine)~~ (not yet supported)
* *space_name* - custom name for a memcached space, default is `memcached`
* *if_not_exists* - do not throw error if an instance already exists.

## What's supported, what's not and other features

**Everything is supported, unless the opposite is stated explicitly**

* Text protocol commands:
  - `set`/`add`/`cas`/`replace`/`append`/`prepend` commands (set section)
  - `get`/`gets` commands (including multiget)
  - `delete` command
  - `incr`/`decr` commands
  - `flush`/`version`/`quit` commands
  - `verbosity` - partially, logging is not very good.
  - `stat` - `reset` is supported and all stats too.
* Binary protocol's commands:
  - `get`/`getk`/`getq`/`getkq` commands (get section)
  - `add`/`addq`/`replace`/`replaceq`/`set`/`setq` commands (set section)
  - `quit`/`quitq`/`flush`/`flushq`/`noop`/`version` commands
  - `gat`/`gatq`/`touch`/`gatk`/`gatkq` commands
  - `append`/`prepend`/`incr`/`decr`
  - `verbosity` - partially, logging is not very good.
  - `stat` - `reset` is supported and all stats too.
  - for now **SASL** authentication is not supported
  - **range** operations are not supported as well.
* Expiration is supported
* Flush is supported
* The protocol is synchronous
* Full support of Tarantool means of consistency (write-ahead logs, snapshots, replication)
* You can access data from Lua
* for now LRU is not supported
* TAP is not supported (for now)
* VBucket is not supported (for now)
* UDP/UNIX sockets are not supported (for now)

## Caution

This rock is in early beta.
