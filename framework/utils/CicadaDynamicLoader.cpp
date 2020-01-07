//
//  CicadaDynamicLoader.h
//  utils
//
//  Created by huang_jiafa on 2019/4/17.
//  Copyright © 2019 alibaba. All rights reserved.
//

#include "CicadaDynamicLoader.h"

#ifdef __APPLE__
    #include <TargetConditionals.h>
#endif

#ifdef _WIN32
    #include <windows.h>
#else
    #include <dlfcn.h>
#endif // _WIN32

#include <sstream>

#if TARGET_OS_IPHONE
    #include <map>
    #include <mutex>
    static std::map<std::string, void *> gMap;
    static std::mutex gMutex;
#endif

#include "frame_work_log.h"

std::string CicadaDynamicLoader::mWorkPath = "@rpath/";

void CicadaDynamicLoader::setWorkPath(const std::string &workPath)
{
    AF_LOGI("setWorkPath :%s", workPath.c_str());
    mWorkPath = workPath;
}

void CicadaDynamicLoader::addFunctionToMap(const std::string &functionName, void *functionAddress)
{
#if TARGET_OS_IPHONE
    std::lock_guard<std::mutex> lock(gMutex);
    gMap[functionName] = functionAddress;
#endif
}

CicadaDynamicLoader::CicadaDynamicLoader(const std::string &libName)
    : mLibName(libName)
{
}

CicadaDynamicLoader::~CicadaDynamicLoader ()
{
    freeLib();
}

void *CicadaDynamicLoader::loadALib(const std::string &libName)
{
    void *handle = nullptr;
#if TARGET_OS_OSX
    std::ostringstream stringStream;
    stringStream << mWorkPath << libName << ".framework/" << libName;
    std::string loadLib = stringStream.str();
    handle = dlopen(loadLib.c_str(), RTLD_NOW);
#elif defined ANDROID
    std::ostringstream stringStream;
    stringStream << mWorkPath << "lib" << libName << ".so";
    std::string loadLib = stringStream.str();
    handle = dlopen(loadLib.c_str(), RTLD_NOW);
    AF_LOGI("Load Lib :%s :%p", loadLib.c_str(), handle);
#elif defined _WIN32
    char appPath[1024] = {0};

    if (handle == NULL) {
        ::GetModuleFileName(NULL, appPath, sizeof (appPath));
        std::string loadPath = appPath;
        std::size_t pos = loadPath.find_last_of('\\');

        if (pos != std::string::npos) {
            loadPath = loadPath.substr(0, pos + 1);
        }

        loadPath.append(libName);
        handle = LoadLibrary(loadPath.c_str());
        AF_LOGI("Load Lib :%s :%p", loadPath.c_str(), handle);

        if (nullptr == handle) {
            handle = LoadLibrary(libName.c_str());
            AF_LOGI("Load Lib :%s :%p", libName.c_str(), handle);
        }
    }

#endif
    return handle;
}

bool CicadaDynamicLoader::loadLib()
{
    if (nullptr == mDynamicHandle) {
        mDynamicHandle = loadALib(mLibName);
    }

    if (nullptr != mDynamicHandle) {
        return true;
    }

    return false;
}

bool CicadaDynamicLoader::getFunctionAddress(const std::string &functionName, void **functionAddress)
{
    void *func = nullptr;
#if TARGET_OS_OSX || defined ANDROID

    if ((nullptr != mDynamicHandle) && (nullptr != functionAddress)) {
        func = dlsym(mDynamicHandle, functionName.c_str());
        AF_LOGI("Dymamic get function :%s :%p", functionName.c_str(), func);
    }

#elif defined _WIN32

//    if ((nullptr != mDynamicHandle) && (nullptr != functionAddress)) {
//        func = GetProcAddress((HMODULE)mDynamicHandle, functionName.c_str());
//        AF_LOGI("Dymamic get function :%s :%p", functionName.c_str(), func);
//    }

#elif TARGET_OS_IPHONE
    {
        std::lock_guard<std::mutex> lock(gMutex);

        if (gMap.count(functionName) > 0) {
            func = gMap[functionName];
        }
    }
#endif
    *functionAddress = func;

    if (func) {
        return true;
    }

    return false;
}

bool CicadaDynamicLoader::freeALib(void *Handle)
{
#if TARGET_OS_OSX || defined ANDROID

    if (Handle != NULL) {
        dlclose(Handle);
        AF_LOGI("dlclose Lib :%p", Handle);
    }

#elif defined _WIN32

    if (Handle != NULL) {
        FreeLibrary((HMODULE)Handle);
        AF_LOGI("dlclose Lib :%p", Handle);
    }

#endif
    return true;
}

bool CicadaDynamicLoader::freeLib (void)
{
    bool ret = freeALib(mDynamicHandle);
    mDynamicHandle = nullptr;
    return ret;
}
