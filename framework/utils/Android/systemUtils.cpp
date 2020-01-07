//
// Created by moqi on 2019-08-15.
//

#include "systemUtils.h"

#include <android/log.h>
#include <dlfcn.h>
#include <utils/frame_work_log.h>
#include <sys/system_properties.h>
#include <string>

using namespace std;

#if (__ANDROID_API__ >= 21)

// Android 'L' makes __system_property_get a non-global symbol.
// Here we provide a stub which loads the symbol from libc via dlsym.
typedef int (*PFN_SYSTEM_PROP_GET)(const char *, char *);

int __system_property_get(const char *name, char *value)
{
    static PFN_SYSTEM_PROP_GET __real_system_property_get = NULL;

    if (!__real_system_property_get) {
        // libc.so should already be open, get a handle to it.
        void *handle = dlopen("libc.so", RTLD_NOLOAD);

        if (!handle) {
            __android_log_print(ANDROID_LOG_ERROR, "foobar", "Cannot dlopen libc.so: %s.\n", dlerror());
        } else {
            __real_system_property_get = (PFN_SYSTEM_PROP_GET) dlsym(handle, "__system_property_get");
        }

        if (!__real_system_property_get) {
            __android_log_print(ANDROID_LOG_ERROR, "foobar", "Cannot resolve __system_property_get(): %s.\n", dlerror());
        }
    }

    return (*__real_system_property_get)(name, value);
}

#endif // __ANDROID_API__ >= 21

string get_android_property(const char *key)
{
    //char *key = (char *)"ro.build.version.release";
    char value[PROP_VALUE_MAX] = {0};
    string Value = "";
    int ret = __system_property_get(key, value);

    if (ret <= 0) {
        AF_LOGE("get prop value failed.\n");
        return Value;
    }

    Value = value;
    return Value;
}