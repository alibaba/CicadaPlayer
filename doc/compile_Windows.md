# coross compile windows on Ubuntu 18.04 use mingw-w64 or macOS


# 1. Environment setup

## Ubuntu 18.04

```bash
sudo apt install git make autoconf libtool yasm
sudo apt install g++-mingw-w64-x86-64 gcc-mingw-w64-i686
```

## macOS

```bash
brew install mingw-w64
```

# 2. update cmake for Ubuntu
https://github.com/Kitware/CMake/releases/download/v3.16.0/cmake-3.16.0-Linux-x86_64.sh

# 3. select posix mingw for Ubuntu
    ```bash
    sudo update-alternatives --config x86_64-w64-mingw32-g++ # Set the default mingw32 g++ compiler option to posix.
    sudo update-alternatives --config i686-w64-mingw32-g++   
    sudo update-alternatives --config x86_64-w64-mingw32-gcc # Set the default mingw32 gcc compiler option to posix.
    sudo update-alternatives --config i686-w64-mingw32-gcc
    ```

4. download sdl

   https://www.libsdl.org/release/SDL2-devel-2.0.10-mingw.tar.gz

   edit framework/windows.cmake
   add /path/to/your/sdl/x86_64-w64-mingw32/lib to COMMON_LIB_DIR
   add /path/to/your/sdl/x86_64-w64-mingw32/include to COMMON_INC_DIR

4. build_external

    ```bash
    . setup.env
    cd external/
    ./build_external.sh Windows
    ```
   
5. build cmdline
    ```bash
    cd cmdline/
    mkdir build
    cd build
    cmake -DCMAKE_TOOLCHAIN_FILE=../cmdline/toolchain.windows.cmake ../
    make cicadaPlayer
    ```
6. copy to windows

    copy the files under to a same dir to windows pc
     - **cicadaPlayer.exe** in build dir
     - **libgcc_s_seh-1.dll,  libstdc++-6.dll, libwinpthread-1.dll**, find it in mingw install dir
     - **avcodec-58.dll, avfilter-7.dll, avformat-58.dll,avresample-4.dll, avutil-56.dll, swresample-3.dll, swscale-5.dll**, find in external/install/ffmpeg/win32/x86_64/bin/
     - **SDL2.dll** find in sdl download dir

     double click **cicadaPlayer.exe**


