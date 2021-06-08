git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
git checkout 109ce457421dbef04011a7e5a4fd482f61e21f62
.\bootstrap-vcpkg.sh
.\vcpkg integrate install
.\vcpkg install openssl sdl2 pthread dirent libxml2
git checkout 5a271a9290282e09149401486f88dc106dc65b71
.\bootstrap-vcpkg.bat
.\vcpkg install curl[openssl]
pause