![Cicada Logo](doc/Cicada.svg)

[![coverity Status](https://scan.coverity.com/projects/20151/badge.svg?flat=1)](https://scan.coverity.com/projects/alibaba-cicadaplayer)
[![codecov](https://codecov.io/gh/alibaba/CicadaPlayer/branch/develop/graph/badge.svg)](https://codecov.io/gh/alibaba/CicadaPlayer/branch/develop)
[![LICENSE](https://img.shields.io/badge/license-MIT-orange.svg)](LICENSE)

[![iOS CI](https://github.com/alibaba/CicadaPlayer/actions/workflows/iOS.yml/badge.svg)](https://github.com/alibaba/CicadaPlayer/actions/workflows/iOS.yml)
[![macOS CI](https://github.com/alibaba/CicadaPlayer/actions/workflows/macOS.yml/badge.svg)](https://github.com/alibaba/CicadaPlayer/actions/workflows/macOS.yml)
[![Android CI](https://github.com/alibaba/CicadaPlayer/actions/workflows/Android.yml/badge.svg)](https://github.com/alibaba/CicadaPlayer/actions/workflows/Android.yml)
[![Linux CI](https://github.com/alibaba/CicadaPlayer/actions/workflows/Linux.yml/badge.svg)](https://github.com/alibaba/CicadaPlayer/actions/workflows/Linux.yml)

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
- [5.compile_Windows(msvc)](doc/compile_Windows_msvc.md)
- [6. compile_MacOS](doc/compile_mac.md)
- 7.webAssembly coming soon


## How to use

- [1. cmdline (Windows/MacOS/Linux)](cmdline/README.md)
- [2. Android](platform/Android/README.md)
- [3. iOS/MacOS](platform/Apple/README.md)
- 4.webAssembly coming soon


## Features
- HLS LL-HLS master play list support, seamless switch
- MPEG-DASH
- WideVine
- ABR
- hardware decode on Android and Apple platform
- HEVC support
- OpenGL render
- HDR render
- change volume by software
- speed playback
- snapshot
- subtitle title and external subtitle title
- on playing cache
- background video playback
- http keep-alive
- customer ip resolve
- video rotation
- black list on Android hardware h264 decoder
- plugin supporting


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
- MIT License

  [libxml2](http://xmlsoft.org/)


