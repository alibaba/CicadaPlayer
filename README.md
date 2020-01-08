![Cicada Logo](doc/Cicada.svg)

[![HitCount](http://hits.dwyl.io/aliyun/CicadaPlayer.svg)](http://hits.dwyl.io/aliyun/CicadaPlayer)
[![Build Status](https://api.travis-ci.org/aliyun/CicadaPlayer.svg?branch=develop)](https://travis-ci.org/aliyun/CicadaPlayer)
[![coverity Status](https://scan.coverity.com/projects/20151/badge.svg?flat=1)](https://scan.coverity.com/projects/aliyun-cicadaplayer)
[![LICENSE](https://img.shields.io/badge/license-MIT-orange.svg)](LICENSE)

<h1 align="center">
  Keep the world free of difficult videos to play!
</h1>

Cicada Media Player is a multi platform player sdk，**Keep the world free of difficult videos to play**，using Cicada Media Player, build your multimedia apps happily.


## Features
- HLS master play list support, seamless switch
 
   [Support instructions](framework/demuxer/HLSSupport.md)

- ABR
- hardware decode on Android and iOS
- HEVC support
- OpenGL render
- change volume by software
- speed playback
- snapshot
- subtitle title support
- external subtitle title support
- on playing cache
- background video playback
- http keep-alive
- self ip resolve
- video rotation
- black list on Android hardware h264 decoder
- plugin support



## HOW TO compile

The default ffmpeg, curl, and openssl git url is the github mirror, if you want use another, set it before compile like:

```bash
export FFMPEG_GIT=https://gitee.com/mirrors/ffmpeg.git
export OPENSSL_GIT=https://gitee.com/mirrors/openssl.git
export CURL_GIT=https://gitee.com/mirrors/curl.git
```

- [1. compile iOS](doc/compile_ios.md)
- [2. compile Android](doc/compile_Android.md)
- [3. compile_Linux](doc/compile_Linux.md)
- [4. compile_Windows (cross compile)](doc/compile_Windows.md)
- [5. compile_MacOS](doc/compile_mac.md)
- 6.webAssemble coming soon


## How to use

- [1. cmdline (Windows/MacOS/Linux)](cmdline/README.md)
- [2. Android](platform/Android/README.md)
- [3. iOS/MacOS](platform/Apple/README.md)
- 4.webAssemble coming soon

## Contact

<img src="https://gw.alicdn.com/tfs/TB1ayl9mpYqK1RjSZLeXXbXppXa-170-62.png" width="28" height="10"  alt="dingding"/>

<img src="doc/CicadaDingDing.jpg" width="300" height="400"  alt="dingding"/>

## License
```c++
MIT LICENSE

Copyright (c) 2019-present Alibaba Inc.
```
CicadaPlayerSDK using the projects:

- LGPL

   [FFmpeg](http://ffmpeg.org/)

   [libvlc](https://www.videolan.org/vlc/libvlc.html)

- CURL LICENSE

  [curl](https://curl.haxx.se)

- Apache License v2

  [OpenSSL](https://www.openssl.org/)

- zlib license

  [SDL](https://www.libsdl.org/)
  
- BSL-1.0 license

  [boost](https://www.boost.org/)


## Code Learning
- [中文](doc/code_learning.zh.md)

