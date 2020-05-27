git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.sh
.\vcpkg integrate install
.\vcpkg install openssl curl[openssl] sdl2 pthread dirent
pause