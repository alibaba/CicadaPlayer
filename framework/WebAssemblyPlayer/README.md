WABitStreamDecoder

compile webAssembly

### 1. ffmpeg

`emconfigure ./configure --cc="emcc" --cxx="em++" --ar="emar" --prefix=$(pwd)/../WasmVideoPlayer/dist --enable-cross-compile --target-os=none --arch=x86_32 --cpu=generic --disable-avdevice --disable-swresample --disable-postproc --disable-filters --enable-filter=atempo --disable-programs --disable-logging --disable-everything --enable-avformat --enable-decoder=hevc --enable-decoder=aac --enable-decoder=h264 --disable-ffplay --disable-ffprobe  --disable-asm --disable-doc --disable-devices --disable-pthreads --disable-w32threads  --disable-hwaccels --disable-parsers  --enable-parser=aac --enable-parser=h264 --enable-parser=hevc --disable-bsfs  --enable-bsf=h264_mp4toannexb --enable-bsf=hevc_mp4toannexb --disable-debug --enable-protocol=file --enable-protocol=http --enable-demuxer=mov  --disable-indevs --disable-outdevs
make -j8

make install
`
### 2. openssl

`export CC=emcc
export CXX=emcc
export LINK=${CXX}
export ARCH_FLAGS=""
export ARCH_LINK=""
export CPPFLAGS=" ${ARCH_FLAGS} "
export CXXFLAGS=" ${ARCH_FLAGS} "
export CFLAGS=" ${ARCH_FLAGS} "
export LDFLAGS=" ${ARCH_LINK} "
echo  $OSTYPE | grep -i darwin > /dev/null 2> /dev/null
`
`./Configure purify --openssldir=--prefix=$(pwd)/../WasmVideoPlayer/dist --prefix=$(pwd)/../WasmVideoPlayer/dist no-engine no-dso no-dgram no-sock no-srtp no-stdio no-ui no-err no-ocsp no-psk no-stdio no-ts
make install -j8
`
### 3. edit the cmake

edit ../Emscripten.cmake

make sure the ffmpeg and openssl dir

### make
`mkdir build
cd build
cmake ../  -DCMAKE_TOOLCHAIN_FILE=${EMSDK}/fastcomp/emscripten/cmake/Modules/Platform/Emscripten.cmake
make WAPlayer
`
### api
see  WABitStreamDecoder.h



