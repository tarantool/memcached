#!/bin/bash

set -e -o pipefail

curl -LsSf https://www.tarantool.io/release/2/installer.sh | sudo bash
