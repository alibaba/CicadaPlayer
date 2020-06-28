## Development environment
This page is an introduction to the compilation for iOS.
1. Host Requirements
    - OS X 10.10 Yosemite (or later)
    - Latest Xcode version (version 11 works so far)
    - A correct shell (we recommend bash,zsh coming...)
    - CocoaPods (for dependency management)
    - make sure the cmake is 3.14 or later
    - [brew](https://brew.sh/)
    ```bash
    brew install automake git libtool autoconf cmake
    ```
2. build

    ```bash
    . setup.env
    cd external/
    ./build_external.sh iOS
    cd ../platform/Apple/demo/iOS/
    ./genxcodeproj.sh
    cd CicadaDemo
    pod install
    open CicadaDemo.xcworkspace
    ```  

   If you update the Xcode version during developing,you should close your Xcode
   ```bash
   cd external/
   rm -rf build install
   ./build_external.sh iOS
   cd ../platform/Apple/demo/iOS/
   rm -rf SDK
   ./genxcodeproj.sh
   cd CicadaDemo
   open CicadaDemo.xcworkspace
   ```
