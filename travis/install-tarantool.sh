#!/bin/sh
set -e
set -o verbose
# rm -rf $HOME/tarantool-1.6
# check to see if tarantool folder is empty
if [ -d "$HOME/tarantool-1.6" && ! -d "$HOME/tarantool-1.6/.git" ]; then
    rm -rf $HOME/tarantool-1.6
fi
if [ ! -d "$HOME/tarantool-1.6" ]; then
  echo 'Getting and building';
  cd $HOME/ && git clone https://github.com/tarantool/tarantool.git tarantool-1.6 -b 1.6
  cd $HOME/tarantool-1.6 && git submodule update --init --recursive
  cd $HOME/tarantool-1.6 && cmake . -DCMAKE_INSTALL_PREFIX=$HOME/local && make && make install
else
  echo 'Using cached directory.';
  cd $HOME/tarantool-1.6 && git pull
  cd $HOME/tarantool-1.6 && git submodule update --init --recursive
  cd $HOME/tarantool-1.6 && make && make install
fi
