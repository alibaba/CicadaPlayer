![Cicada Logo](doc/Cicada.svg)

[![HitCount](http://hits.dwyl.io/aliyun/CicadaPlayer.svg)](http://hits.dwyl.io/aliyun/CicadaPlayer)
[![Build Status](https://api.travis-ci.org/alibaba/CicadaPlayer.svg?branch=develop)](https://travis-ci.org/alibaba/CicadaPlayer)
[![coverity Status](https://scan.coverity.com/projects/20151/badge.svg?flat=1)](https://scan.coverity.com/projects/alibaba-cicadaplayer)
[![codecov](https://codecov.io/gh/alibaba/CicadaPlayer/branch/develop/graph/badge.svg)](https://codecov.io/gh/alibaba/CicadaPlayer/branch/develop)
[![LICENSE](https://img.shields.io/badge/license-MIT-orange.svg)](LICENSE)

<h1 align="center">
  Keep the world free of difficult videos to play!
</h1>

Cicada Media Player is a multi platform player sdk，**Keep the world free of difficult videos to play**，using Cicada Media Player, build your multimedia apps happily.

## Try it on Android devices

[![deom](doc/demoQR.png)](https://alivc-demo-cms.alicdn.com/versionProduct/other/public/cicadaPlayer/cicadaPlayer.html)

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
- 6.webAssembly coming soon


## How to use

- [1. cmdline (Windows/MacOS/Linux)](cmdline/README.md)
- [2. Android](platform/Android/README.md)
- [3. iOS/MacOS](platform/Apple/README.md)
- 4.webAssembly coming soon


## Features
- HLS master play list support, seamless switch
 
   [HLS Support instructions](framework/demuxer/HLSSupport.md)

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


## Contact

[![DingDing](doc/CicadaDingDing.png)](https://h5.dingtalk.com/invite-page/index.html?bizSource=____source____&corpId=ding42c495ce0dcfdb7f35c2f4657eb6378f&inviterUid=B8D63ADF200A8E9DFF4BBDCA828801C7&encodeDeptId=FE36B0936DFD1AC591E7FE61FE0552A6)

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


