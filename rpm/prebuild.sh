#!/bin/bash

set -exu  # Strict shell (w/o -o pipefail)

os=""
dist=""
if [ -f /etc/os-release ]; then
    . /etc/os-release
    os="$ID"
    dist="$VERSION_ID"
fi

# The installer script is ending with unexpected error of unsupported OS for
# "Almalinux 9". Here we fix this by overriding the cenos-release file so that
# the enviroment settings work correctly.
if [[ "$os" == "almalinux" && "$dist" == 9* ]]; then
    sudo sh -c "echo 8 > /etc/centos-release"
    curl -LsSf https://tarantool.io/release/3/installer.sh | sudo bash
else
    # We need to execute the 1.10 script due to it works for other platforms.
    curl -LsSf https://tarantool.io/release/1.10/installer.sh | sudo bash
fi
