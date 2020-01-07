#!/bin/sh
# to install mingw-w64 on mac:
# http://mingw-w64.org/doku.php/download/macports
function build(){
    mkdir -p build_Windows/$1
    cd build_Windows/$1

    cmake -DARCH=$1 -DCMAKE_TOOLCHAIN_FILE=${TOP_DIR}/prebuild/Toolchain-cross-mingw32-linux.cmake  ../../
    make -j8
    cd -
}

build i686
build x86_64


