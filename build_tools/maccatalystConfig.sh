#!/usr/bin/env bash

function maccatalyst_init_globe_env(){
    export MACOS_X86_64_VERSION="10.15"
    MACOS_ARM64_VERSION="11.0"      # Min support for Apple Silicon is 11.0
 #   CATALYST_IOS="13.0"				# Min supported is iOS 13.0 for Mac Catalyst
    MACOSX_SDK=$(xcrun --sdk macosx --show-sdk-path)
    BUILD_MACHINE=`uname -m`

    DEVELOPER=$(xcode-select --print-path)
    MACOSX_SDK_VERSION=$(xcrun --sdk macosx --show-sdk-version)
    MACOSX_PLATFORM=$(xcrun --sdk macosx --show-sdk-platform-path)

}
function maccatalyst_init_env(){
    maccatalyst_init_globe_env
    local arch
    local ARCH=""
    for arch in ${MACOS_ARCHS}
    do
        if [ "$arch" == "$1" ]
        then
            ARCH=$1
        fi
    done

    if [ "$ARCH" == "" ]
    then
        echo unsupported ARCH $1
        exit 1
    fi

    CPU_ARCH=${ARCH}
    local cross_compile=true
    if [[ $ARCH == "x86_64" ]]; then
      MACOS_VER="${MACOS_X86_64_VERSION}"
      if [ ${BUILD_MACHINE} == 'arm64' ]; then
        cross_compile=true
      fi
    fi

    if [[ $ARCH == "arm64" ]]; then
      MACOS_VER="${MACOS_ARM64_VERSION}"
      if [ ${BUILD_MACHINE} == 'x86_64' ]; then
        cross_compile=true
      fi
    fi

    CPU_FLAGS="-arch $ARCH"
    CPU_FLAGS="$CPU_FLAGS -mmacosx-version-min=$MACOS_VER -fembed-bitcode -target ${ARCH}-apple-ios${CATALYST_IOS}-macabi \
    -isysroot  ${MACOSX_SDK} \
    -isystem ${MACOSX_SDK}/System/iOSSupport/usr/include \
    -iframework ${MACOSX_SDK}/System/iOSSupport/System/Library/Frameworks"
    if [ $cross_compile ];then
      CPU_LDFLAGS="-arch $ARCH  -target ${ARCH}-apple-ios${CATALYST_IOS}-macabi \
      -isysroot ${MACOSX_SDK} \
      -L${MACOSX_SDK}/System/iOSSupport/usr/lib \
			-L${DEVELOPER}/Toolchains/XcodeDefault.xctoolchain/usr/lib/swift/maccatalyst \
			-iframework $MACOSX_SDK/System/iOSSupport/System/Library/Frameworks"
    fi
    SYSROOT=${MACOSX_SDK}
    if [ "$ARCH" = "arm64" ];then
      CROSS_COMPILE=arm-apple-darwin
    else
      CROSS_COMPILE=${ARCH}-apple-darwin
    fi

    CC="xcrun --sdk iphoneos clang"

   if [ "$ARCH" = "arm64" ]
   then
        NEON_SUPPORT="TRUE"
        AS="gas-preprocessor.pl -arch aarch64 -- $CC"
#    else
#        AS="gas-preprocessor.pl -- $CC"
    fi
}