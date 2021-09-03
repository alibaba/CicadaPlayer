//
// Created by SuperMan on 8/31/21.
//


#include "VideoFilterChain.h"
#include "filterPrototype.h"
#include <utils/frame_work_log.h>

using namespace Cicada;

VideoFilterChain::VideoFilterChain()
{}

VideoFilterChain::~VideoFilterChain()
{
    mVideoFiltersMap.clear();
}

void VideoFilterChain::addFilter(const std::string &target, std::unique_ptr<IVideoFilter> videoFilter)
{
    mVideoFiltersMap[target] = move(videoFilter);
}

int VideoFilterChain::push(std::unique_ptr<IAFFrame> &frame)
{
    if (mInPutFrame != nullptr) {
        AF_LOGW("have not pull last frame , will be covered ");
    }

    mInPutFrame = move(frame);
    return 0;
}

int VideoFilterChain::pull(std::unique_ptr<IAFFrame> &frame)
{
    bool hasSuccessFilter = false;
    int ret = 0;
    for (auto &iter : mVideoFiltersMap) {
        bool filterSuccess = true;
        std::unique_ptr<IVideoFilter> &filter = iter.second;
        ret = filter->push(mInPutFrame, 0);
        //TODO if push fail, need push next time
        if (ret < 0) {
            filterSuccess = false;
            continue;
        }
        //TODO deal with multi frames: use frame list
        ret = filter->pull(mInPutFrame, 0);
        if (ret < 0) {
            filterSuccess = false;
        }

        if (filterSuccess && !hasSuccessFilter) {
            hasSuccessFilter = true;
        }
    }

    frame = move(mInPutFrame);
    if (!hasSuccessFilter) {
        AF_LOGW("all filter fail ?! ");
    }
    return hasSuccessFilter ? 0 : -1;
}

bool VideoFilterChain::empty()
{
    return mVideoFiltersMap.empty();
}

void VideoFilterChain::updateFilter(const std::string &target, const std::string &options)
{
    auto iter = mVideoFiltersMap.find(target);
    if (iter != mVideoFiltersMap.end()) {
        iter->second->setOption(filterPrototype::FILTER_CONFIG_KEY_OPTIONS, options, "");
    }
}

void VideoFilterChain::setInvalid(const std::string &target, bool invalid)
{
    std::unique_ptr<IVideoFilter> &filter = mVideoFiltersMap.find(target)->second;
    if (filter != nullptr) {
        filter->setInvalid(invalid);
    }
}

bool VideoFilterChain::init(int type)
{
    auto iter = mVideoFiltersMap.begin();
    while (iter != mVideoFiltersMap.end()) {
        std::unique_ptr<IVideoFilter> &filter = iter->second;
        bool ret = filter->init(type);
        if (!ret) {
            //remove init failed filter
            iter = mVideoFiltersMap.erase(iter);
        } else {
            ++iter;
        }
    }
    //if all init fail , we do not pass to texture filter
    return !mVideoFiltersMap.empty();
}

bool VideoFilterChain::removeFilter(const std::string &target)
{
    auto iter = mVideoFiltersMap.find(target);
    if (iter != mVideoFiltersMap.end()) {
        return mVideoFiltersMap.erase(target);
    }
    return false;
}
