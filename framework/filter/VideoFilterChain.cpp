//
// Created by SuperMan on 8/31/21.
//

#define LOG_TAG "VideoFilterChain"

#include "VideoFilterChain.h"
#include "filterPrototype.h"
#include <utils/frame_work_log.h>

using namespace Cicada;

VideoFilterChain::VideoFilterChain()
{}

VideoFilterChain::~VideoFilterChain()
{
    mVideoFiltersMap.clear();

    clearFrames(mInPutFrames);
    clearFrames(mOutPutFrames);
}

void VideoFilterChain::addFilter(const std::string &target, std::unique_ptr<IVideoFilter> videoFilter)
{
    mVideoFiltersMap[target] = move(videoFilter);
}

int VideoFilterChain::push(std::unique_ptr<IAFFrame> &frame)
{
    if (frame == nullptr) {
        AF_LOGD("filter push eof frame");
    }

    mInPutFrames.push(frame.release());

    filterLoop();

    return 0;
}

void VideoFilterChain::filterLoop()
{
    AF_LOGD("filterLoop in....");

    if (mVideoFiltersMap.empty()) {
        AF_LOGD("mVideoFiltersMap empty");
        swapFrames(mOutPutFrames, mInPutFrames);
    } else {
        SpscQueue<IAFFrame *> inputFrames{10};
        SpscQueue<IAFFrame *> outFrames{10};

        swapFrames(inputFrames, mInPutFrames);

        for (auto &iter : mVideoFiltersMap) {

            swapFrames(inputFrames, outFrames);

            std::unique_ptr<IAFFrame> frame = nullptr;
            std::unique_ptr<IVideoFilter> &filter = iter.second;

            AF_LOGD("before filter %s , input frame size = %d , out frames size = %d", filter->getName().c_str(), inputFrames.size(),
                    outFrames.size());

            pullFrames(filter, outFrames);
            pushFrames(filter, inputFrames);
            pullFrames(filter, outFrames);

            AF_LOGD("after filter %s , input frame size = %d , out frames size = %d", filter->getName().c_str(), inputFrames.size(),
                    outFrames.size());
        }

        swapFrames(mOutPutFrames, outFrames);
    }

    AF_LOGD("filterLoop out....");
}


void VideoFilterChain::pullFrames(std::unique_ptr<IVideoFilter> &filter, SpscQueue<IAFFrame *> &outFrames)
{
    int ret = 0;
    std::unique_ptr<IAFFrame> frame = nullptr;
    while (true) {
        ret = filter->pull(frame, 0);
        if (ret >= 0) {
            AF_LOGD("filter %s pull success pts= %lld", filter->getName().c_str(), frame->getInfo().pts);
            outFrames.push(frame.release());
        } else if (ret == -EAGAIN) {
            AF_LOGD("filter %s pull  EAGAIN", filter->getName().c_str());
            break;
        } else {
            AF_LOGW("filter %s  pull Error = %d ", filter->getName().c_str(), ret);
            break;
        }
    }
}

void VideoFilterChain::swapFrames(SpscQueue<IAFFrame *> &dstFrames, SpscQueue<IAFFrame *> &srcFrames)
{
    while (!srcFrames.empty()) {
        dstFrames.push(srcFrames.front());
        srcFrames.pop();
    }
    AF_LOGD("swapFrames  dstFrames frame size = %d , srcFrames frames size = %d", dstFrames.size(), srcFrames.size());
}

void VideoFilterChain::clearFrames(SpscQueue<IAFFrame *> &frames)
{
    while (!frames.empty()) {
        std::unique_ptr<IAFFrame> frame = std::unique_ptr<IAFFrame>(frames.front());
        frames.pop();
    }
}

void VideoFilterChain::pushFrames(std::unique_ptr<IVideoFilter> &filter, SpscQueue<IAFFrame *> &inputFrames)
{
    int ret = 0;
    std::unique_ptr<IAFFrame> frame = nullptr;
    while (true) {
        if (inputFrames.empty()) {
            //TODO eof
            break;
        } else {
            frame = std::unique_ptr<IAFFrame>(inputFrames.front());

            AF_LOGD("filter %s push pts = %lld", filter->getName().c_str(), frame->getInfo().pts);

            ret = filter->push(frame, 0);
            if (ret >= 0) {
                inputFrames.pop();
            } else if (ret == -EAGAIN) {
                AF_LOGD("filter %s push EAGAIN", filter->getName().c_str());
                break;
            } else {
                AF_LOGW("filer %s push Error = %d ", filter->getName().c_str(), ret);
                inputFrames.pop();
                break;
            }
        }
    }
}

int VideoFilterChain::pull(std::unique_ptr<IAFFrame> &frame)
{
    IAFFrame *framePtr = nullptr;
    if (!mOutPutFrames.empty()) {
        framePtr = mOutPutFrames.front();
        mOutPutFrames.pop();
    }

    if (framePtr == nullptr) {
        return -1;
    }

    frame.reset(framePtr);
    return 0;
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
