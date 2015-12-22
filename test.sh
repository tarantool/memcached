curl -s https://packagecloud.io/install/repositories/tarantool/1_6/script.deb.sh | sudo bash
sudo apt-get install -y tarantool tarantool-dev --force-yes
pip install --user python-daemon PyYAML six==1.9.0
cmake . -DCMAKE_BUILD_TYPE=Release
make test-memcached
