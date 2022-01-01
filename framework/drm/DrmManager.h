//
// Created by SuperMan on 11/27/20.
//

#ifndef SOURCE_DRMMANAGER_H
#define SOURCE_DRMMANAGER_H

#include <mutex>
#include <map>
#include "DrmHandler.h"
#include "DrmInfo.h"
#include <memory>

namespace Cicada {

    class DrmManager {
    public:

        DrmManager();

        ~DrmManager();

        void setDrmCallback(const std::function<DrmResponseData*(const DrmRequestParam& drmRequestParam)>  &callback) {
            mDrmCallback = callback;
        }

        DrmHandler *require(const DrmInfo &drmInfo);

        void clearErrorItems();

    private:
        std::mutex mDrmMutex{};
        std::map<DrmInfo, std::unique_ptr<DrmHandler>  , DrmInfo::DrmInfoCompare> mDrmMap{};
        std::function<DrmResponseData*(const DrmRequestParam& drmRequestParam)>  mDrmCallback{nullptr};
    };

}
#endif //SOURCE_DRMMANAGER_H
