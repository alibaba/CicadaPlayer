#!/bin/sh
rm -rf build_ios

mkdir build_ios
cd build_ios/
mkdir OS
mkdir SIMULATOR64
cd OS/
# review alivc_framework/script/tool/build-ios.sh usage

cmake -DCMAKE_TOOLCHAIN_FILE=../../../prebuild/iOS.cmake  ../../  -DIOS_PLATFORM=OS  -GXcode
xcodebuild IPHONEOS_DEPLOYMENT_TARGET=8.0 -destination generic/platform=iOS -configuration Debug -target ALL_BUILD

cd ../SIMULATOR64
cmake -DCMAKE_TOOLCHAIN_FILE=../../../prebuild/iOS.cmake  ../../  -DIOS_PLATFORM=SIMULATOR64  -GXcode

xcodebuild IPHONEOS_DEPLOYMENT_TARGET=8.0 -destination "platform=iOS Simulator" -configuration Debug -target ALL_BUILD
#make VERBOSE=1

