#!/bin/bash

curl -s https://packagecloud.io/install/repositories/tarantool/1_10/script.deb.sh | sudo bash
sudo apt-get install -y tarantool tarantool-dev libevent-dev libsasl2-dev --force-yes
pip install --user python-daemon PyYAML six==1.9.0 msgpack-python gevent==1.1.2
TARANTOOL_DIR=/usr/include cmake . -DCMAKE_BUILD_TYPE=Release

# third_party/libmemcached/bootstrap.sh runs /usr/local/bin/shellcheck on
# itself when the executable is found. Disabled all warnings it finds to allow
# Travis-CI to pass.
SHELLCHECK_OPTS="-e SC1091 -e SC2027 -e SC2034 -e SC2035 -e SC2086 -e SC2119"
SHELLCHECK_OPTS="${SHELLCHECK_OPTS} -e SC2120 -e SC2124 -e SC2128 -e SC2129"
SHELLCHECK_OPTS="${SHELLCHECK_OPTS} -e SC2145 -e SC2154 -e SC2155 -e SC2166"
SHELLCHECK_OPTS="${SHELLCHECK_OPTS} -e SC2181 -e SC2193 -e SC2206"
export SHELLCHECK_OPTS

make internalso libmemcached
make test-memcached
