#!/usr/bin/env bash

BUILD_TOOLS_DIR=$(cd $(dirname ${BASH_SOURCE[0]}); pwd)
PATH=$PATH:${BUILD_TOOLS_DIR}

function check_cmake(){
    if [ ! `which cmake` ]
    then
        echo 'cmake not found'
        if [ ! `which brew` ]
        then
            echo 'Homebrew not found. Trying to install...'
            ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)" \
            || exit 1
        fi
        echo 'Trying to install cmake...'
        brew install cmake || exit 1
    else
        local major=`cmake -version | head -1 | cut -d ' ' -f3 | cut -d '-' -f1 | cut -d '.' -f1`
        local minor=`cmake -version | head -1 | cut -d ' ' -f3 | cut -d '-' -f1 | cut -d '.' -f2`
        if [[ ${major} -ge 3  &&  ${minor} -ge 14 ]];then
            echo cmake version is ok ${major}.${minor}
        else
            brew upgrade cmake
        fi
    fi
}

function check_yasm(){
    if [ ! `which yasm` ]
    then
        echo 'yasm not found'
        if [ ! `which brew` ]
        then
            echo 'Homebrew not found. Trying to install...'
            ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)" \
            || exit 1
        fi
        echo 'Trying to install yasm...'
        brew install yasm
    fi
    echo `yasm --version`

    if [ ! `which nasm` ]
    then
        echo 'nasm not found'
        if [ ! `which brew` ]
        then
            echo 'Homebrew not found. Trying to install...'
            ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)" \
            || exit 1
        fi
        echo 'Trying to install nasm...'
        brew install nasm
    fi
}



#function Android_build_Asan(){
#    if [ -n "$MTL" ];then
#        export ANDROID_NDK_HOME=/home/admin/software/android-ndk-r18b
#        echo "ANDROID_NDK_HOME is ${ANDROID_NDK_HOME}"
#    fi
#    cd motuAsan/
#    sh gradlew build --refresh-dependencies --stacktrace -DALIYUN_APP_VERSION=$SAASPLAYERSDK_VERSION $JAVA_HOME_OPT
#    mkdir -p $TOP_DIR/output
#    cp app/build/outputs/apk/debug/CicadaDemo_debug.apk $TOP_DIR/output/
#    cp -r premierlibrary/build/intermediates/cmake/corePlayer/release/obj/armeabi-v7a $TOP_DIR/output
#}


function build_Android(){

    git submodule init
    git submodule update

    JAVA_HOME_OPT=
    if [ -n "$JAVA_HOME" ]
    then
        JAVA_HOME_OPT=-Dorg.gradle.java.home=${JAVA_HOME}
    fi

    cd ${TOP_DIR}/external
    ./build_external.sh Android
    export EXTERN_INSTALL_DIR_ANDROID=$PWD/install/
    export FFMPEG_INSTALL_DIR_ANDROID=$PWD/install/ffmpeg/Android/
    if [ $? -ne 0 ]; then
        echo "build_external break"
        return 1
    fi

    cd ${TOP_DIR}/platform/Android
#    if [ -n "$BUILD_ASAN" ];then
#       Android_build_Asan
#       return 0
#    fi
    cd source/
    sh gradlew clean
    # sh gradlew build --refresh-dependencies --stacktrace -DALIYUN_APP_VERSION=$SAASPLAYERSDK_VERSION $JAVA_HOME_OPT
    #   #run twice for copy aar
    export ANDROID_FULL_PACKAGE='true'
    sh gradlew build --refresh-dependencies --stacktrace $JAVA_HOME_OPT

    cd ${TOP_DIR}/platform/Android
    ./package.sh
    # ./apsaraPlayer_Android/build_demo.sh



    cd $TOP_DIR
    mkdir output
    cp `find platform/Android -name "*.apk"` output
    cp `find platform/Android/source/releaseLibs/ -name "*.aar"` output
    cp `find platform/Android/release -name "*.zip"` output

    mkdir -p output/armeabi-v7a/
    mkdir -p output/arm64-v8a/

    cp  platform/Android/source/premierlibrary/build/intermediates/cmake/corePlayer/release/obj/armeabi-v7a/*.so output/armeabi-v7a/
    cp  platform/Android/source/premierlibrary/build/intermediates/cmake/corePlayer/release/obj/arm64-v8a/*.so output/arm64-v8a/

    cp  external/install/ffmpeg/Android/armeabi-v7a/libalivcffmpeg.so   output/armeabi-v7a/
    cp  external/install/ffmpeg/Android/arm64-v8a/libalivcffmpeg.so     output/arm64-v8a/

    cd output
    tree
}

function packet_iOS(){
    rm -rf ${TOP_DIR}/output
    mkdir -p ${TOP_DIR}/output/
    mkdir -p ${TOP_DIR}/output/CicadaPlayerSDK/
    mkdir -p ${TOP_DIR}/output/CicadaPlayerSDK/CicadaDemo
    CicadaSDK_PATH=${TOP_DIR}/output/CicadaPlayerSDK/SDK/
    CicadaSDK_ALL=${CicadaSDK_PATH}/ARM_SIMULATOR
    CicadaSDK_ARM=${TOP_DIR}/output/CicadaPlayerSDK/SDK/ARM
    mkdir -p ${CicadaSDK_ALL}
    mkdir -p ${CicadaSDK_ARM}

    #cp clear CicadaDemo at first
    cp -rf CicadaDemo/ ${TOP_DIR}/output/CicadaPlayerSDK/CicadaDemo

    cd ${DEMO_SOURCE_DIR_IOS}/SDK/
    ALL_FRAMEWORK_PATH=${DEMO_SOURCE_DIR_IOS}/SDK/ARM_SIMULATOR
    mkdir -p ${ALL_FRAMEWORK_PATH}
    #build iphoneos
    xcodebuild -scheme ALL_BUILD ONLY_ACTIVE_ARCH=NO -configuration MinSizeRel -sdk iphoneos VALID_ARCHS="armv7 arm64"
    cp -rf ./MinSizeRel/*.framework ${ALL_FRAMEWORK_PATH}
    cp -rf ./MinSizeRel/*.dSYM ${CicadaSDK_ALL}/

    cp -rf ./MinSizeRel/*.framework ${CicadaSDK_ARM}/

    #build simulator
    xcodebuild -scheme ALL_BUILD ONLY_ACTIVE_ARCH=NO -configuration Release -sdk iphonesimulator VALID_ARCHS="x86_64 i386"
    if [ $? -ne 0 ]; then
        echo "simulator build failed"
    else
        lipo -create "${ALL_FRAMEWORK_PATH}/CicadaPlayerSDK.framework/CicadaPlayerSDK" "./Release/CicadaPlayerSDK.framework/CicadaPlayerSDK" -output "${ALL_FRAMEWORK_PATH}/CicadaPlayerSDK.framework/CicadaPlayerSDK"
        lipo -create "${ALL_FRAMEWORK_PATH}/alivcffmpeg.framework/alivcffmpeg" "./Release/alivcffmpeg.framework/alivcffmpeg" -output "${ALL_FRAMEWORK_PATH}/alivcffmpeg.framework/alivcffmpeg"
    fi

    # copy to SDK folder
    cp -rf ${ALL_FRAMEWORK_PATH}/*.framework ${CicadaSDK_ALL}/

    nobit_path=${CicadaSDK_PATH}/ARM_NO_BITCODE
    mkdir -p ${nobit_path}
    cp -rf ${CicadaSDK_ARM}/*.framework ${nobit_path}
    xcrun bitcode_strip ${nobit_path}/CicadaPlayerSDK.framework/CicadaPlayerSDK -r -o ${nobit_path}/CicadaPlayerSDK.framework/CicadaPlayerSDK
    xcrun bitcode_strip ${nobit_path}/alivcffmpeg.framework/alivcffmpeg -r -o ${nobit_path}/alivcffmpeg.framework/alivcffmpeg

    #build app without SDK
#    mv CicadaPlayerSDK.xcodeproj CicadaPlayerSDKBak.xcodeproj
#    cd ${DEMO_SOURCE_DIR_IOS}/CicadaDemo
#    sh packetIPA.sh
#    cp ./build/Release-iphoneos/CicadaDemo.ipa ${TOP_DIR}/output
#    cd ./build/Release-iphoneos/CicadaDemo.xcarchive/dSYMs
#    tar -cjvf ${TOP_DIR}/output/CicadaDemo.app.dSYM.bz2 ./CicadaDemo.app.dSYM
#    cd ${DEMO_SOURCE_DIR_IOS}/SDK/
#    mv CicadaPlayerSDKBak.xcodeproj CicadaPlayerSDK.xcodeproj

    cd ${TOP_DIR}/output/
    tar -cjvf ${TOP_DIR}/output/CicadaPlayerSDK_${MUPP_BUILD_ID}.bz2 CicadaPlayerSDK/
}

function build_iOS_new(){
    DEMO_SOURCE_DIR_IOS=${TOP_DIR}/platform/Apple/demo/iOS
    cd ${DEMO_SOURCE_DIR_IOS}
    ./genxcodeproj.sh
    if [ -n "$MTL" ];then
      packet_iOS
    fi
}

function build_iOS(){

#    if [ -n "$MTL" ];then
#        export HOMEBREW_NO_AUTO_UPDATE=true
#    fi
    check_cmake
    check_yasm

    if [ $? -ne 0 ]; then
        return 1
    fi

    cd ${TOP_DIR}/external
    ./build_external.sh iOS

    if [ $? -ne 0 ]; then
        echo "build_external break"
        return 1
    fi

    build_iOS_new

    if [ ! -f "${TOP_DIR}/output/CicadaPlayerSDK/SDK/ARM_SIMULATOR/CicadaPlayerSDK.framework/CicadaPlayerSDK" ]; then
        echo "CicadaPlayerSDK.framework build failed"
        return  1
    fi
}

function packet_mac(){
    mkdir ${TOP_DIR}/output
    OUT_PUT_IDR=${TOP_DIR}/output

    cd ${DEMO_SOURCE_DIR_MAC}/SDK
    xcodebuild -scheme install ONLY_ACTIVE_ARCH=NO -configuration MinSizeRel
    #build app without SDK
    mv CicadaPlayerSDK.xcodeproj CicadaPlayerSDKBak.xcodeproj

    cd ${DEMO_SOURCE_DIR_MAC}/
    mkdir CicadaDemo_release
    cp -rf CicadaDemo CicadaDemo_release/
    mkdir -p CicadaDemo_release/SDK/

    cd ${DEMO_SOURCE_DIR_MAC}/CicadaDemo
    xcodebuild -workspace CicadaDemo.xcworkspace -scheme CicadaDemo ONLY_ACTIVE_ARCH=NO -configuration Release

    cd ${DEMO_SOURCE_DIR_MAC}/Release
    tar -cjvf ${OUT_PUT_IDR}/CicadaDemo.app.bz2  CicadaDemo.app
    tar -cjvf ${OUT_PUT_IDR}/CicadaDemo.app.dSYM.bz2  CicadaDemo.app.dSYM

    cd ${DEMO_SOURCE_DIR_MAC}
    mv ./SDK/CicadaPlayerSDKBak.xcodeproj ./SDK/CicadaPlayerSDK.xcodeproj
    mv  SDK/*.framework CicadaDemo_release/SDK/
    mv  SDK/MinSizeRel/*.dSYM CicadaDemo_release/SDK/
    tar -cjvf ${OUT_PUT_IDR}/CicadaDemo_release.bz2 CicadaDemo_release
}

function build_mac(){
    if [ -n "$MTL" ];then
        export HOMEBREW_NO_AUTO_UPDATE=true
    fi
    check_cmake
    check_yasm

    if [ $? -ne 0 ]; then
        return 1
    fi

    cd ${TOP_DIR}/external
    ./build_external.sh macOS

    if [ $? -ne 0 ]; then
        echo "build_external break"
        return  1
    fi

    DEMO_SOURCE_DIR_MAC=${TOP_DIR}/platform/Apple/demo/macOS
    cd ${DEMO_SOURCE_DIR_MAC}
    ./Genxcodeproj.sh

    if [ -n "$MTL" ];then
        packet_mac
    fi

    cd ${TOP_DIR}
}

