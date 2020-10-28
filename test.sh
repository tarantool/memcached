#!/bin/sh

set -eu  # Strict shell, w/o print commands (set -x), w/o -o pipefail

TARANTOOL_VERSION="${TARANTOOL_VERSION:-1.10}"

# Setup tarantool repository.
#
# --repo-only: Setup the repository, but don't install tarantool.
#              The tarantool executable is installed later,
#              together with the development package.
#
# --type live: Setup so called 'live' repository with the latest
#              tarantool versions of given ${TARANTOOL_VERSION}
#              branch. We want to verify the module against the
#              latest versions to spot problems earlier.
curl -fsSL https://tarantool.io/installer.sh | \
    sudo "VER=${TARANTOOL_VERSION}" bash -s - --repo-only --type live

# Install tarantool executable, headers and other build/test
# dependencies of the module.
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
