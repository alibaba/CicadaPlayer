git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
git checkout 5a271a9290282e09149401486f88dc106dc65b71
.\bootstrap-vcpkg.sh
.\vcpkg integrate install
.\vcpkg install openssl curl[openssl] sdl2 pthread dirent libxml2
pause