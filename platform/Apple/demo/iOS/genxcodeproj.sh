mkdir  SDK
cd SDK/
cmake ../../../../Apple/source/ -GXcode     -DCMAKE_SYSTEM_NAME=iOS     "-DCMAKE_OSX_ARCHITECTURES=arm64;x86_64" -DCMAKE_OSX_DEPLOYMENT_TARGET=8.0


