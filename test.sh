curl -s https://packagecloud.io/install/repositories/tarantool/1_6/script.deb.sh | sudo bash
sudo apt-get install -y tarantool tarantool-dev libsasl2-dev sasl2-bin --force-yes
pip install --user python-daemon PyYAML six==1.9.0
TARANTOOL_DIR=/usr/include cmake . -DCMAKE_BUILD_TYPE=Release
make internalso libmemcached
make test-memcached
