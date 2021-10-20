git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
git checkout 109ce457421dbef04011a7e5a4fd482f61e21f62
.\bootstrap-vcpkg.sh
.\vcpkg integrate install
.\vcpkg install sdl2 pthread dirent libxml2
pause