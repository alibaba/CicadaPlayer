#!/usr/bin/env bash

BUILD_TOOLS_DIR=$(cd $(dirname ${BASH_SOURCE[0]}); pwd)
mkdir -p ~/bin
cp ${BUILD_TOOLS_DIR}/gas-preprocessor.pl ~/bin/
PATH=~/bin:$PATH
echo "gas-preprocessor.pl is $(which gas-preprocessor.pl)"