//
// Created by 平凯 on 2021/5/17.
//

#include "globalNetWorkManager.h"
#include <mutex>

using namespace Cicada;
using namespace std;
static int alive = 1;
globalNetWorkManager *globalNetWorkManager::getGlobalNetWorkManager()
{
    static globalNetWorkManager sInstance{};
    if (alive) {
        return &sInstance;
    }
    return nullptr;
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

globalNetWorkManager::~globalNetWorkManager()
{
    alive = 0;
}
