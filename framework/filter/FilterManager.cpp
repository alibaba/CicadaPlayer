//
// Created by yiliang on 2021/8/20.
//
#define LOG_TAG "FilterManager"

#include "FilterManager.h"
#include <filter/filterPrototype.h>
#include <utils/af_string.h>
#include <utils/frame_work_log.h>

using namespace std;
using namespace Cicada;


FilterManager::FilterManager(const IAFFrame::videoInfo &videoInfo, const CicadaJSONArray &config)
{
    mVideoInfo = videoInfo;
    mFilterConfig = config.printJSON();

}

FilterManager::~FilterManager()
{}

void FilterManager::setupFilterChains()
{
    std::unique_ptr<VideoFilterChain> textureFilterChain = std::unique_ptr<VideoFilterChain>(new VideoFilterChain());
    std::unique_ptr<VideoFilterChain> bufferFilterChain = std::unique_ptr<VideoFilterChain>(new VideoFilterChain());

    int filterFeature = IVideoFilter::Feature::Buffer;
#ifdef __ANDROID__
    // both can be used
    filterFeature = IVideoFilter::Feature::Buffer | IVideoFilter::Feature::Texture;
#else
    //use buffer filter
    filterFeature = IVideoFilter::Feature::Buffer;
#endif

    assert(streamMeta != nullptr);
    if (streamMeta->pixel_fmt == AF_PIX_FMT_YUV420P10BE || streamMeta->pixel_fmt == AF_PIX_FMT_YUV420P10LE) {
        AF_LOGD("HDR video\n");
        filterFeature |= IVideoFilter::Feature::HDR;
    }

    CicadaJSONArray config(mFilterConfig);
    int size = config.getSize();
    for (int i = 0; i < size; i++) {
        CicadaJSONItem &configItem = config.getItem(i);
        IVideoFilter *filter = filterPrototype::create(filterFeature, configItem, mVideoInfo, false);
        if (filter == nullptr) {
            AF_LOGW("not found match filter for : %s", configItem.printJSON().c_str());
            continue;
        }

        std::unique_ptr<IVideoFilter> targetFilter = std::unique_ptr<IVideoFilter>(filter);
        std::string options;
        if (configItem.hasItem(filterPrototype::FILTER_CONFIG_KEY_OPTIONS)) {
            options = configItem.getString(filterPrototype::FILTER_CONFIG_KEY_OPTIONS);
            targetFilter->setOption("options", options, "");
        } else {
            AF_LOGW("filter config has no `options`");
        }

        std::string target = configItem.getString(filterPrototype::FILTER_CONFIG_KEY_TARGET);
        if ((filterFeature & IVideoFilter::Feature::Texture) && filter->isFeatureSupported(IVideoFilter::Feature::Texture)) {
            AF_LOGI("textureFilter chain add : %s for %s", targetFilter->getName().c_str(), target.c_str());

            targetFilter->setOption("useFeature", AfString::to_string(IVideoFilter::Feature::Texture), "");
            textureFilterChain->addFilter(target, move(targetFilter));

        } else if ((filterFeature & IVideoFilter::Feature::Buffer) && filter->isFeatureSupported(IVideoFilter::Feature::Buffer)) {
            AF_LOGI("bufferFilter chain add : %s for %s", targetFilter->getName().c_str(), target.c_str());

            targetFilter->setOption("useFeature", AfString::to_string(IVideoFilter::Feature::Buffer), "");
            if (targetFilter->init(0) /* not used */) {
                bufferFilterChain->addFilter(target, move(targetFilter));
            }
        }
    }

    if (!textureFilterChain->empty()) {
        mFilterChains[IVideoFilter::Feature::Texture] = move(textureFilterChain);
    }
    if (!bufferFilterChain->empty()) {
        mFilterChains[IVideoFilter::Feature::Buffer] = move(bufferFilterChain);
    }
}

bool FilterManager::push(std::unique_ptr<IAFFrame> &frame)
{
    if (!mNeedFilter) {
        return false;
    }

    int format = frame->getInfo().video.format;
    if (format == AF_PIX_FMT_CICADA_MEDIA_CODEC) {
        return false;
    }

    IVideoFilter::Feature feature = format == AF_PIX_FMT_CICADA_TEXTURE ? IVideoFilter::Feature::Texture : IVideoFilter::Feature::Buffer;

    if (!hasFilter(feature)) {
        //        AF_LOGW("not found filter for %d ?! ", feature);
        return false;
    }
    std::unique_ptr<VideoFilterChain> &filterChain = mFilterChains.at(feature);
    int ret = filterChain->push(frame);
    return (ret >= 0);
}

bool FilterManager::pull(int format, std::unique_ptr<IAFFrame> &frame)
{
    if (format == AF_PIX_FMT_CICADA_MEDIA_CODEC) {
        return false;
    }

    IVideoFilter::Feature feature = format == AF_PIX_FMT_CICADA_TEXTURE ? IVideoFilter::Feature::Texture : IVideoFilter::Feature::Buffer;

    if (!hasFilter(feature)) {
        //        AF_LOGW("not found filter for %d ?! ", feature);
        return false;
    }
    std::unique_ptr<VideoFilterChain> &filterChain = mFilterChains.at(feature);
    int ret = filterChain->pull(frame);
    return (ret >= 0);
}

void FilterManager::setNeedFilter(bool needFilter)
{
    mNeedFilter = needFilter;
}

void FilterManager::setInvalid(const std::string &target, bool invalid)
{
    for (auto &iter : mFilterChains) {
        iter.second->setInvalid(target, invalid);
    }
}

bool FilterManager::initFilter(IVideoFilter::Feature feature, int filterType)
{
    if (!hasFilter(feature)) {
        return false;
    }

    std::unique_ptr<VideoFilterChain> &targetFilterChain = mFilterChains[feature];
    return targetFilterChain->init(filterType);
}

bool FilterManager::hasFilter(IVideoFilter::Feature feature)
{
    return mFilterChains.count(feature) > 0;
}

void FilterManager::updateFilter(const std::string &target, const std::string &options)
{
    for (auto &iter : mFilterChains) {
        iter.second->updateFilter(target, options);
    }
}

void FilterManager::setSpeed(float speed)
{
    for (auto &iter : mFilterChains) {
        iter.second->setSpeed(speed);
    }
}

void FilterManager::setStreamMeta(const Stream_meta* meta) {
    streamMeta = meta;

    if (!mFilterInited) {
        //TODO how to deal with pixel format changed
        setupFilterChains();
        mFilterInited = true;
    }
}
