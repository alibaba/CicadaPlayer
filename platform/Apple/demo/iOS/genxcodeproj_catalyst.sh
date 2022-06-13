mkdir  SDK
cd SDK/
cmake ../../../../Apple/source/ -GXcode     -DCMAKE_SYSTEM_NAME=iOS  -DCMAKE_OSX_DEPLOYMENT_TARGET=8.0 -DMACCATALYST=ON


