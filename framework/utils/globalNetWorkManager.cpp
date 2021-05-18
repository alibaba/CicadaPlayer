//
// Created by 平凯 on 2021/5/17.
//

#include "globalNetWorkManager.h"
#include <mutex>

using namespace Cicada;
using namespace std;
globalNetWorkManager globalNetWorkManager::sInstance{};
globalNetWorkManager *globalNetWorkManager::getGlobalNetWorkManager()
{
    return &sInstance;
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
