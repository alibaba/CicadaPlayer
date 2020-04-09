//
// Created by lifujun on 2019/4/11.
//

#include "globalSettings.h"
#define LOG_TAG "globalSettings"

#include "frame_work_log.h"

using namespace std;

namespace Cicada {
    static globalSettings *gSetting = nullptr;
    static mutex globalMutex;

    int globalSettings::setProperty(const string &key, const string &value)
    {
        std::unique_lock<std::mutex> uMutex(mMutex);
        bool bProtected = key.compare(0, 10, "protected.") == 0;

        for (auto &property : mProperties) {
            if (property.key == key) {
                if (key.compare(0, 3, "ro.") == 0) {
                    AF_LOGE("set a read only property error\n");
                    return -1;
                }

                if (bProtected) {
                    if (this_thread::get_id() != property.bindingTid) {
                        AF_LOGE("set a protected property error\n");
                        return -1;
                    }
                }

                property.value = value;
                return 0;
            }
        }

        class property pro;

        pro.key = key;

        pro.value = value;

        if (bProtected) {
            pro.bindingTid = this_thread::get_id();
        }

        mProperties.push_back(pro);
        return 0;
    }

    const string &globalSettings::getProperty(const string &key)
    {
        std::unique_lock<std::mutex> uMutex(mMutex);

        for (auto &property : mProperties) {
            if (property.key == key) {
                return property.value;
            }
        }

        return mDefaultKeyValue;
    }

    globalSettings *globalSettings::getSetting()
    {
        std::lock_guard<std::mutex> uMutex(globalMutex);

        if (gSetting == nullptr) {
            gSetting = new globalSettings();
        }

        return gSetting;
    }

    int globalSettings::addResolve(const string &host, const string &ip)
    {
        std::unique_lock<std::mutex> uMutex(mMutex);
        auto item = mResolve.find(host);
        if (item != mResolve.end()) {
            (*item).second.insert(ip);
            return 0;
        }
        set<string> ipSet;
        ipSet.insert(ip);
        mResolve[host] = ipSet;
        return 0;
    }

    void globalSettings::removeResolve(const string &host, const string &ip)
    {
        std::unique_lock<std::mutex> uMutex(mMutex);
        auto item = mResolve.find(host);
        if (item != mResolve.end()) {

            if(ip.empty()) {
                (*item).second.clear();
            } else {
                (*item).second.erase(ip);
            }

            if ((*item).second.empty()) {
                mResolve.erase(item);
            }
        }
    }
}

