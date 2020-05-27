# compile windows on win10 use visual studio 2017

1. Environment setup
    ```bash
    install visual studio 2017, include English language pack (see https://github.com/microsoft/vcpkg/issues/2295)
    install cmake 3.15.2
    ```

2. build external
	```bash
	cross compile ffmpeg on ubuntu use mingGW, see compile_Windows.md
	copy build result CicadaPlayer\external\install\ffmpeg in ubuntu to the same dir in your windows system
	find avc.h hevc.h avio_internal.h url.h in CicadaPlayer\external\external\ffmpeg\libavformat in ubuntu, copy to CicadaPlayer\external\install\ffmpeg\win32\i686\include\libavformat
	run CicadaPlayer\external\win\install_external.bat, you can set proxy by set HTTPS_PROXY and HTTP_PROXY system environment variables, see https://github.com/microsoft/vcpkg/issues/9527
	```

3. build cmdline
    ```bash
    run CicadaPlayer\cmdline\msvc_build\build_win.bat
    ```

4. copy depends
    ```bash
    copy CicadaPlayer\external\install\ffmpeg\win32\i686\bin\*.dll to CicadaPlayer\cmdline\msvc_build\_generated\cmake_win\Release
	find libgcc_s_sjlj-1.dll(posix version) in mingw install dir, copy to CicadaPlayer\cmdline\msvc_build\_generated\cmake_win\Release
    ```
