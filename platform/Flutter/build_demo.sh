#!/bin/bash

flutter_sh_path=${PWD}

cd ../Android/source/

echo ${flutter_sh_path}

./gradlew assembleRelease

rm -rf ../../Flutter/android/libs
mkdir ../../Flutter/android/libs

cp ./releaseLibs/* ../../Flutter/android/libs/



