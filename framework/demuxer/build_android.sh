#!/bin/sh

# review alivc_framework/script/tool/build-ios.sh usage

cd build_android/

mkdir demuxer
cd demuxer/

mkdir armeabi-v7a
mkdir arm64-v8a

cd armeabi-v7a/
cmake  `pwd`/../../../demuxer/ \
-DCMAKE_TOOLCHAIN_FILE=`pwd`/../../../application.cmake \
-DANDROID_ABI=armeabi-v7a \
-DCMAKE_BUILD_TYPE=Debug

make

cd ../../../

#TODO arm64-v8a
#cd ../arm64-v8
#make VERBOSE=1

