//
// Created by SuperMan on 11/27/20.
//

#include <cassert>
#include "DrmManager.h"
#include "DrmHandlerPrototype.h"

using namespace Cicada;

DrmManager::DrmManager() {

}

DrmManager::~DrmManager() {
    mDrmMap.clear();
}

DrmHandler *DrmManager::require(const DrmInfo &drmInfo) {

    std::lock_guard<std::mutex> drmLock(mDrmMutex);

    if (!mDrmMap.empty()) {
        for (auto &item : mDrmMap) {
            auto &drmItem = (DrmInfo &) item.first;
            if (drmItem == drmInfo) {
                return item.second.get();
            }
        }
    }

    DrmHandler *pDrmHandler = DrmHandlerPrototype::create(drmInfo);

    assert(pDrmHandler != nullptr);

    if (pDrmHandler == nullptr) {
        return nullptr;
    }

    pDrmHandler->setDrmCallback(mDrmCallback);
    mDrmMap[drmInfo] = std::unique_ptr<DrmHandler>(pDrmHandler);

    return pDrmHandler;
}

void DrmManager::clearErrorItems() {
    std::lock_guard<std::mutex> drmLock(mDrmMutex);

    if (!mDrmMap.empty()) {
        for (auto iter = mDrmMap.begin(); iter != mDrmMap.end();) {
            DrmHandler *handler = iter->second.get();
            if (handler->isErrorState()) {
                iter = mDrmMap.erase(iter);
            } else {
                iter++;
            }
        }
    }
}
