## 1.仓库介绍
本仓库是一个编译和交叉编译开源库的工具包，目前支持了ffmpeg在Android和iOS平台的交叉编译,使用MinGW 交叉编译Windows, Mac的native编译
ffmpeg目前支持fdk-aac和x264的外部编译。
支持curl librtmp opessl 的编译。

## 2.使用
### 1.配置项

#### 1.环境变量的配置

使用该项目进行编译，用户需要设置一些变量，在编译目录下请创建一个设置环境变量的脚本，脚本名称是
user_env.sh，编译系统会自动执行该脚本
该脚本的作用主要是设置要编译的项目的源码目录和Android编译时的ndk路径
可以参考env.sh 进行设置。如

    #!/bin/bash
    FFMPEG_SOURCE_DIR=$TOP_DIR/external/ffmpeg
    FDK_AAC_SOURCE_DIR=$TOP_DIR/external/fdk-aac/
    X264_SOURCE_DIR=$TOP_DIR/external/x264/
    OPEN_SSL_SOURCE_DIR=$TOP_DIR/external/openssl/
    CURL_SOURCE_DIR=$TOP_DIR/external/curl
    DAV1D_SOURCE_DIR=$TOP_DIR/external/dav1d
    RTMPDUMP_SOURCE_DIR=$TOP_DIR/external/rtmpdump/
    ANDROID_NDK=$ANDROID_NDK_HOME
    
如果某些源码不需要编译，则可以不设置源码目录地址，目前支持x264的可选编译，其他工程理论上都支持，但是未测试。

env.sh 脚本中的BUILD变量是用于调试用，设置成False后将不会做任何编译，只进行最后的链接工作
，目前不建议使用。

#### 2.ffmpeg编译的配置

要去配置ffmpeg的组件，请在编译目录下创建一个或多个叫*_ffmpeg_config.sh的脚本，编译系统
会自动执行这些脚本，并对脚本进行做和的操作,具体用法可以参考ffmpeg_default_config.sh进行，

**ffmpeg_default_config.sh目前没有被纳入到编译系统中来，默认不会被执行。**

目前编译系统中已经内置了三个ffmpeg的config配置，放在了configs目录下

    oldplayer_ffmpeg_config.sh 老版本播放使用的配置项

    player_ffmpeg_config.sh 新版本播放使用的配置项

    videoEditor_ffmpeg_config.sh 短视频目前使用的配置项

如果内置配置项可以满足编译需求，则可以直接将该配置项拷贝到编译目录中使用，
如

单独编译新版本播放器的时候 将player_ffmpeg_config.sh拷贝到编译目录
    
单独编译短视频播放器的时候 将videoEditor_ffmpeg_config.sh拷贝到编译目录
    
编译的ffmpeg希望可以同时满足短视频和播放的需求时将videoEditor_ffmpeg_config.sh 和player_ffmpeg_config.sh
都拷贝到编译工作目录下


### 2.编译
```
cd path/to/your/build/dir
path/to/build_iOS(Android/win32/native).sh
```

在编译目录用户可以自己创建一个ffmpeg的配置文件，编译系统会自动将该配置文件的内容添加到配置中
，此配置文件只限于ffmpeg组件的配置，不要去添加其他配置。

如播放器中的使用：

build_external_android.sh 

    #!/usr/bin/env bash
    cp $TOP_DIR/build_tools/configs/player_ffmpeg_config.sh ./
    $TOP_DIR/build_tools/build_Android.sh



### 3.编译生成目录说明
build  编译中间文件目录，由于有些工程并不支持脱离源码目录编译(OpenSSL librtmp)，所以该目录中并不包含所有的编译文件

install 编译后生成的静态库动态库的和头文件的安装目录

目前所有的静态库都和ffmpeg打包成一个动态库或者是动态framework，所以若使用最终的动态库，请去install下面的ffmpeg目录去找。
后面策略会有变化，到时再更新。

### 4.MinGW 编译Windows环境说明

推荐使用Ubuntu 环境下编译， 推荐使用Ubuntu 16.04以及以上版本。环境初始化可以参考 [vlc的编译说明](https://wiki.videolan.org/Win32Compile/)

    apt-get install gcc-mingw-w64-i686 g++-mingw-w64-i686 mingw-w64-tools
    apt-get install gcc-mingw-w64-x86-64 g++-mingw-w64-x86-64 mingw-w64-tools
    apt-get install wine64-development-tools libwine-dev zip p7zip nsis bzip2
    apt-get install lua5.2 libtool automake autoconf autopoint make gettext pkg-config
    apt-get install yasm nasm

编译x264可能需要nasm 2.13以上，请手动安装


## 3.集成tips

目前播放器的Android版本已经集成了该编译系统。
播放器中有alivc_framework 和 player_framework两个工程都依赖于ffmpeg和其他开源库，
1. 把本仓库加入到播放器的repo管理列表中，repo sync后本仓库会自动被下载到指定目录
2. 进入到预设的编译目录，执行编译脚本，如build_external_android.sh
3. 将环境变量导出，以便编译alivc_framework

```
export EXTERN_INSTALL_DIR_ANDROID=$PWD/install/
export FFMPEG_INSTALL_DIR_ANDROID=$PWD/install/ffmpeg/Android/
```

  使用脚本编译的时候alivc_framework的CmakeLists文件中会引用FFMPEG_INSTALL_DIR_ANDROID，
  从而找到ffmpeg的头文件路径

```
include_directories($ENV{FFMPEG_INSTALL_DIR_ANDROID}/${ANDROID_ABI}/include)
``` 
  此处后面应该会改为直接引用变量的方式，而非环境变量，此变量的值可以通过cmake 传递进来，方便在Android studio中编译。
  
4.由于播放器自有代码 Android目前使用gradle 进行编译，并考虑到使用Android studio去编译，所以在gradle中
做了以下配置

```
def rootPath  = rootProject.getRootDir().getAbsolutePath()
externalNativeBuild {
   cmake {
       arguments '-DANDROID_PLATFORM=android-18',
             '-DANDROID_TOOLCHAIN=gcc', '-DANDROID_STL=gnustl_static'
              '-DANDROID_TOOLCHAIN=gcc', '-DANDROID_STL=gnustl_static',
              "-DFFMPEG_INSTALL_DIR_ANDROID=$rootPath/../../apsaraPlayer/external/install/ffmpeg/Android/",
              "-DEXTERN_INSTALL_DIR_ANDROID=$rootPath/../../apsaraPlayer/external/install/"
   }
}
```    
将安装的目录设置给cmake，这样在cmake中就可以引用这些变量了，注意必须使用""，单引号不会把$rootPath 展开

5.摩天轮编译player和svideo的
    mtl目录下实现了一个在摩天轮环境下编译的实例，可以编译单独的播放器或者短视频的
    ios地址:
    http://mtl3.alibaba-inc.com/project/project_sdk_cr_list.htm?projectId=93185
    Android地址:
    http://mtl3.alibaba-inc.com/project/project_sdk_cr_list.htm?projectId=93099
```
cd mtl
./build_player.sh Android "svideo player" #编译Android版本的短视频和播放器的合集
./build_player.sh iOS "player" #编译iOS版本的播放器版本
```
其他方案类似


## 4.后续工作
1. 更好的支持调试方式，改动一个文件不需要整体都编译，目前也能支持，不过要手动改改才行。
2. Android 编译单独的动态库。
  