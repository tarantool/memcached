#!/bin/bash

curl -s https://packagecloud.io/install/repositories/tarantool/1_6/script.deb.sh | sudo bash
sudo apt-get install -y tarantool tarantool-dev --force-yes
pip install --user python-daemon PyYAML six==1.9.0 msgpack-python gevent
TARANTOOL_DIR=/usr/include cmake . -DCMAKE_BUILD_TYPE=Release
make internalso libmemcached
make test-memcached
