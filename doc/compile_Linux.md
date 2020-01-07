# Linux Compile on Ubuntu 18.04

1. Environment setup

    ```bash
    sudo apt-get install git make gcc autoconf libtool yasm cmake g++ libsdl2-dev
    ```

2. update cmake
    https://github.com/Kitware/CMake/releases/download/v3.16.0/cmake-3.16.0-Linux-x86_64.sh
3. use clang instead of gcc
    ```bash
    sudo apt-get install clang llvm
    sudo update-alternatives --config cc #select clang
    ```

4. build_external

    ```bash
    . setup.env
    cd external/
    ./build_external.sh Linux
    ```
   
5. build cmdline
    ```bash
    cd cmdline/
    mkdir build
    cd build
    cmake ../
    make cicadaPlayer
    ```
   