#!/bin/bash

curl -s https://packagecloud.io/install/repositories/tarantool/1_6/script.deb.sh | sudo bash
sudo apt-get install -y tarantool tarantool-dev libevent-dev libsasl2-dev --force-yes
pip install --user python-daemon PyYAML six==1.9.0 msgpack-python gevent==1.1.2
TARANTOOL_DIR=/usr/include cmake . -DCMAKE_BUILD_TYPE=Release
make internalso libmemcached
make test-memcached
