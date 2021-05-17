//
// Created by 平凯 on 2021/5/17.
//

#include "globalNetWorkManager.h"
#include <mutex>

using namespace Cicada;
using namespace std;
static globalNetWorkManager *g_globalNetWorkManager = nullptr;
globalNetWorkManager *globalNetWorkManager::getGlobalNetWorkManager()
{
    static std::once_flag oc;
    std::call_once(oc, [&] { g_globalNetWorkManager = new globalNetWorkManager(); });
    return g_globalNetWorkManager;
}
void globalNetWorkManager::addListener(globalNetWorkManager::globalNetWorkManagerListener *listener)
{
    if (!listener) {
        return;
    }
    std::unique_lock<std::mutex> uMutex(mMutex);
    mListeners.insert(listener);
}
void globalNetWorkManager::removeListener(globalNetWorkManager::globalNetWorkManagerListener *listener)
{
    if (!listener) {
        return;
    }
    std::unique_lock<std::mutex> uMutex(mMutex);
    mListeners.erase(listener);
}
void globalNetWorkManager::reConnect()
{
    std::unique_lock<std::mutex> uMutex(mMutex);
    for (auto item : mListeners) {
        item->OnReconnect();
    }
}
