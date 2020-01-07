#!/usr/bin/env bash
xcodebuild -workspace CicadaDemo.xcworkspace -scheme CicadaDemo archive -configuration Release -archivePath ./build/Release-iphoneos/CicadaDemo.xcarchive
xcodebuild -exportArchive -exportOptionsPlist ./ipaPlayer.plist -archivePath "./build/Release-iphoneos/CicadaDemo.xcarchive" -exportPath "./build/Release-iphoneos"
