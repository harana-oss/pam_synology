#!/bin/bash

set -xe

sudo apt-get update -qq || true
sudo apt-get install -qq -y --no-install-recommends libpam-dev

if [[ "$CC" == "scan-build" ]]; then
    unset CC
    unset CXX

    scan-build -o analysis --status-bugs make debug
elif [[ "$CC" == "lint" ]]; then
    unset CC
    unset CXX

    make lint
else
    make debug
    make release
fi
