# AndroidCompile
This page is an introduction to the compilation for Android on MacOS or Ubuntu.
1. install android ndk (14b) and Android studio
     - [Android Studio](https://developer.android.com/studio)
     - [Android ndk](https://developer.android.com/ndk/downloads/older_releases.html)

       [we recommend 14b for MacOS](https://dl.google.com/android/repository/android-ndk-r14b-darwin-x86_64.zip)

       [we recommend 14b for Linux](https://dl.google.com/android/repository/android-ndk-r20b-linux-x86_64.zip)
     -  [jdk](https://www.oracle.com/technetwork/java/javase/downloads/index.html)

        [we recommend jdk8](https://www.oracle.com/technetwork/java/javase/downloads/jdk8-downloads-2133151.html)

2. Host Requirements
    - MacOS
    
        - [brew](https://brew.sh/)

        ```bash
        brew install automake git libtool autoconf
        ```

    - Ubuntu
    
        ```bash
        sudo apt-get utomake git libtool autoconf
        ```
 
2. Environment setup

     ```bash
     mkdir ~/Android-env/
     ln -s your_ndk_path ~/Android-env/
    ```

4. build_external
    ```bash
    . setup.env
    cd external/
    ./build_external.sh Android
    ```
5. open the Android studio project  ../platform/Android/source/
6. edit local.properties ,add ndk.dir=your_ndk_path


