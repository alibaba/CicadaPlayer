//
// Created by lifujun on 2019/9/6.
//
#include <cstdlib>
#include "VSyncFactory.h"
#include "timedVSync.h"
#include "CADisplayLinkVSync.h"
#include <string>

#ifdef ANDROID
    #include "AndroidVSync.h"
    #include <utils/Android/systemUtils.h>

#endif

std::unique_ptr<IVSync> VSyncFactory::create(IVSync::Listener &listener, float HZ)
{
    //TODO use android vsync: compatibility issue
//#if defined(ANDROID)
//    std::string version = get_android_property("ro.build.version.sdk");
//
//    if (atoi(version.c_str()) >= 16)
//        return std::unique_ptr<IVSync>(new AndroidVSync(listener));
//
//#endif
    return std::unique_ptr<IVSync>(new timedVSync(listener, HZ));
    //  return std::unique_ptr<IVSync>(new CADisplayLinkVSync(listener));
}
