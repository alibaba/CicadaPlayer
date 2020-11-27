#!/bin/bash

mkdir -p release/
mkdir -p release/saas_release_aar
mkdir -p release/CicadaPlayerDemo

file="source/releaseLibs/*.aar"
cp $file release/saas_release_aar/

cd release
zip -r saas_release_aar-$MUPP_BUILD_ID.zip saas_release_aar
cd ..

cd release
rm -rf ../../../apsaraPlayer_demo/Android/CicadaPlayerDemo/app/build/
mkdir -p ../../../apsaraPlayer_demo/Android/CicadaPlayerDemo/app/libs/
cp ./saas_release_aar/*.aar ../../../apsaraPlayer_demo/Android/CicadaPlayerDemo/app/libs/
cp -rf -n ../../../apsaraPlayer_demo/Android/CicadaPlayerDemo ./
zip -r CicadaDemo-$MUPP_BUILD_ID.zip ./CicadaPlayerDemo
cd ..