# Building

```
cmake .
make libmemcached memtier
```

# Mem(a)slap

```
 * -s, --servers=        : List one or more servers to connect. Servers count must be less than threads count.
                           e.g.: –servers=localhost:1234,localhost:11211
 * -T, --threads=        : Number of threads to startup, better equal to CPU numbers.
                           Default 8.
 * -c, --concurrency=    : Number of concurrency to simulate with load.
                           Default 128.
 * -n, --conn_sock=      : Number of TCP socks per concurrency.
                           Default 1.
 * -x, --execute_number= : Number of operations(get and set) to execute for the given test.
                           Default 1000000.
 * -t, --time=           : How long the test to run, suffix: s,m,h,d
                           e.g.: –time=2h.
 * -F, --cfg_cmd=        : Load the configure file to get command, key and value distribution list.
 * -w, --win_size=       : Task window size of each concurrency, suffix: K, M
                           e.g.: –win_size=10k. Default 10k.
 * -X, --fixed_size=     : Fixed length of value.
 * -v, --verify=         : The proportion of date verification, e.g.:
                           –verify=0.01
 * -d, --division=       : Number of keys to multi-get once.
                           Default 1, means single get.
 * -S, --stat_freq=      : Frequency of dumping statistic information. suffix: s,m
                           e.g.: –resp_freq=10s.
 * -e, --exp_verify      : The proportion of objects with expire time
                           e.g.: –exp_verify=0.01
                           Default no object with expire time
 * -o, --overwrite=      : The proportion of objects need overwrite
                           e.g.: –overwrite=0.01.
                           Default never overwrite object.
 * -R, --reconnect=      : Reconnect tests, when connection is closed it will be reconnected.
 * -U, --udp             : UDP tests, default memaslap uses TCP, TCP port and UDP port of server must be same.
 * -a, --facebook        : Whether it enables facebook test feature, set with TCP and multi-get with UDP.
 * -B, --binary          : Whether it enables binary protocol.
                           Default with ASCII protocol.
 * -P, --tps=            : Expected throughput, suffix: K
                           e.g.: –tps=10k.
 * -p, --rep_write=      : The first nth servers can write data
                           e.g.: –rep_write=2.
 * -b, --verbose         : Whether it outputs detailed information when verification fails.
 * -h, --help            : Display this message and then exit.
 * -V, --version         : Display the version of the application and then exit.
```

Configuration file:
```
###################################
#
# key
# start_len end_len proportion
#
# example1: key range 16~100 accounts for 80%
#           key range 101~200 accounts for 10%
#           key range 201~250 accounts for 10%
#           total should be 1 (0.8+0.1+0.1 = 1)
#
# 16  100 0.8
# 101 200 0.1
# 201 249 0.1
#
# example2: all keys length are 128 bytes
#
# 128 128 1
#
###################################
#
# value
# start_len end_len proportion
#
# example1: value range 1~1000 accounts for 80%
#           value range 1001~10000 accounts for 10%
#           value range 10001~100000 accounts for 10%
#           total should be 1 (0.8+0.1+0.1 = 1)
#
# 1     1000   0.8
# 1001  10000  0.1
# 10001 100000 0.1
#
# example2: all value length are 128 bytes
#
# 128 128 1
#
###################################
#
# cmd
# cmd_type cmd_proportion
#
# currently memaslap only testss get and set command.
#
# cmd_type:
# set     0
# get     1
#
# example: set command accounts for 50%
#          get command accounts for 50%
#          total should be 1 (0.5+0.5 = 1)
#
# cmd
# 0    0.5
# 1    0.5
#
###################################
```

# Memtier benchmark

```
A memcache/redis NoSQL traffic generator and performance benchmarking tool.

Connection and General Options:
  -s, --server=ADDR              Server address (default: localhost)
  -p, --port=PORT                Server port (default: 6379)
  -S, --unix-socket=SOCKET       UNIX Domain socket name (default: none)
  -P, --protocol=PROTOCOL        Protocol to use (default: redis).  Other
                                 supported protocols are memcache_text,
                                 memcache_binary.
  -x, --run-count=NUMBER         Number of full-test iterations to perform
  -D, --debug                    Print debug output
      --client-stats=FILE        Produce per-client stats file
      --out-file=FILE            Name of output file (default: stdout)
      --show-config              Print detailed configuration before running
      --hide-histogram           Don't print detailed latency histogram

Test Options:
  -n, --requests=NUMBER          Number of total requests per client (default: 10000)
                                 use 'allkeys' to run on the entire key-range
  -c, --clients=NUMBER           Number of clients per thread (default: 50)
  -t, --threads=NUMBER           Number of threads (default: 4)
      --test-time=SECS           Number of seconds to run the test
      --ratio=RATIO              Set:Get ratio (default: 1:10)
      --pipeline=NUMBER          Number of concurrent pipelined requests (default: 1)
      --reconnect-interval=NUM   Number of requests after which re-connection is performed
      --multi-key-get=NUM        Enable multi-key get commands, up to NUM keys (default: 0)
  -a, --authenticate=CREDENTIALS Authenticate to redis using CREDENTIALS, which depending
                                 on the protocol can be PASSWORD or USER:PASSWORD.
      --select-db=DB             DB number to select, when testing a redis server
      --distinct-client-seed     Use a different random seed for each client
      --randomize                random seed based on timestamp (defalut is constant value)

Object Options:
  -d  --data-size=SIZE           Object data size (default: 32)
      --data-offset=OFFSET       Actual size of value will be data-size + data-offset
                                 Will use SETRANGE / GETRANGE (default: 0)
  -R  --random-data              Indicate that data should be randomized
      --data-size-range=RANGE    Use random-sized items in the specified range (min-max)
      --data-size-list=LIST      Use sizes from weight list (size1:weight1,..sizeN:weightN)
      --data-size-pattern=R|S    Use together with data-size-range
                                 when set to R, a random size from the defined data sizes will be used,
                                 when set to S, the defined data sizes will be evenly distributed across
                                 the key range, see --key-maximum (default R)
      --expiry-range=RANGE       Use random expiry values from the specified range

Imported Data Options:
      --data-import=FILE         Read object data from file
      --data-verify              Enable data verification when test is complete
      --verify-only              Only perform --data-verify, without any other test
      --generate-keys            Generate keys for imported objects
      --no-expiry                Ignore expiry information in imported data

Key Options:
      --key-prefix=PREFIX        Prefix for keys (default: "memtier-")
      --key-minimum=NUMBER       Key ID minimum value (default: 0)
      --key-maximum=NUMBER       Key ID maximum value (default: 10000000)
      --key-pattern=PATTERN      Set:Get pattern (default: R:R)
                                 G for Gaussian distribution.
                                 R for uniform Random.
                                 S for Sequential.
                                 P for Parallel (Sequential were each client has a subset of the key-range).
      --key-stddev               The standard deviation used in the Gaussian distribution
                                 (default is key range / 6)
      --key-median               The median point used in the Gaussian distribution
                                 (default is the center of the key range)
      --help                     Display this help
      --version                  Display version information

```
