## Development environment
This page is an introduction to the compilation for macOS.
1. Host Requirements
    - OS X 10.10 Yosemite (or later)
    - Latest Xcode version (version 11 works so far)
    - A correct shell (we recommend bash,zsh coming...)
    - CocoaPods (for dependency management)
    - [brew](https://brew.sh/)

     ```bash
    brew install automake git libtool autoconf cmake
    ```

2. build_external

     ```bash
    . setup.env
    cd external/
    ./build_external.sh macOS
    ```

3. build cmdline

    ```bash
    cd cmdline/
    mkdir build
    cd build
    cmake ../
    make
    ```
   You can also use Xcode to build cmdline, add -G Xcode to cmake command

4. build mac platform app

    ```bash
    cd ${TOP_DIR}/platform/Apple/demo/macOS/
    rm -rf SDK
    ./Genxcodeproj.sh
    cd CicadaDemo
    open CicadaDemo.xcworkspace
    ```
