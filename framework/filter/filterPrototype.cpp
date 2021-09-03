//
// Created by yiliang on 2021/8/4.
//
#define LOG_TAG "filterPrototype"

#include "filterPrototype.h"
#include <cassert>
#include <utils/frame_work_log.h>

#include "ffmpegVideoFilter.h"

using namespace std;
using namespace Cicada;
Cicada::IVideoFilter *filterPrototype::filterQueue[];
int filterPrototype::_nextSlot;
const string filterPrototype::FILTER_CONFIG_KEY_TARGET = "target";
const string filterPrototype::FILTER_CONFIG_KEY_ID = "id";
const string filterPrototype::FILTER_CONFIG_KEY_NAME = "name";
const string filterPrototype::FILTER_CONFIG_KEY_OPTIONS = "options";

void filterPrototype::addPrototype(Cicada::IVideoFilter *se)
{
    if (_nextSlot == 10) {
        AF_LOGW("filterPrototype has reach max items, add fail !");
        return;
    }

    filterQueue[_nextSlot++] = se;
}

Cicada::IVideoFilter *filterPrototype::create(int feature, CicadaJSONItem &item, const IAFFrame::videoInfo &videoInfo, bool active)
{

    AF_LOGI("create filter for: %s", item.printJSON().c_str());

    std::list<Cicada::IVideoFilter *> supportFilters = getTargetSupportFilters(item, videoInfo);
    if (supportFilters.empty()) {
        AF_LOGE("not found match filter for : %s", item.printJSON().c_str());
        return nullptr;
    }

    std::list<IVideoFilter *> bufferSupportedFilters{};
    std::list<IVideoFilter *> textureSupportedFilters{};
    for (IVideoFilter *filter : supportFilters) {
        bool textureSupport = filter->isFeatureSupported(IVideoFilter::Feature::Texture);
        if (textureSupport) {
            textureSupportedFilters.push_back(filter);
        }

        bool bufferSupport = filter->isFeatureSupported(IVideoFilter::Feature::Buffer);
        if (bufferSupport) {
            bufferSupportedFilters.push_back(filter);
        }
    }

    bool needTextureFilter = (feature & IVideoFilter::Feature::Texture);
    bool needBufferFilter = (feature & IVideoFilter::Feature::Buffer);

    IVideoFilter *filter{nullptr};

    {
        //TODO add feature to filter when probe score
        if (needTextureFilter && needBufferFilter) {
            //we choose textureFilter first
            filter = getMaxScoreFilter(textureSupportedFilters);
            if (filter == nullptr) {
                filter = getMaxScoreFilter(bufferSupportedFilters);
            }
        } else if (needTextureFilter) {
            filter = getMaxScoreFilter(textureSupportedFilters);
        } else if (needBufferFilter) {
            filter = getMaxScoreFilter(bufferSupportedFilters);
        }
    }

    if (filter == nullptr) {
        AF_LOGE("not found match filter for feature : %d", feature);
        return nullptr;
    }

    AF_LOGI("use filter %s", filter->getName().c_str());

    std::string options;
    if (item.hasItem(FILTER_CONFIG_KEY_OPTIONS)) {
        options = item.getString(FILTER_CONFIG_KEY_OPTIONS);
    } else {
        AF_LOGW("filter config has no `options`");
    }

    IVideoFilter *finalFilter = filter->clone(videoInfo, videoInfo, active);
    finalFilter->setOption("options", options, "");

    return finalFilter;
}


Cicada::IVideoFilter *filterPrototype::getMaxScoreFilter(const std::list<Cicada::IVideoFilter *> &filterList)
{

    if (filterList.empty()) {
        AF_LOGW("filter list is empty");
        return nullptr;
    }

    IVideoFilter *targetFilter = nullptr;
    int max_score = 0;

    for (IVideoFilter *item : filterList) {
        int score = 1;
        if (item->isFeatureSupported(IVideoFilter::Feature::PassThrough)) {
            score += 1;
        }

        if (score > max_score) {
            max_score = score;
            targetFilter = item;
        }
    }

    return targetFilter;
}

std::list<Cicada::IVideoFilter *> filterPrototype::getTargetSupportFilters(const CicadaJSONItem &item, const IAFFrame::videoInfo &videoInfo)
{
    list<Cicada::IVideoFilter *> resultList{};
    if (!item.hasItem(FILTER_CONFIG_KEY_TARGET)) {
        AF_LOGE("filter config has no `target` : %s", item.printJSON().c_str());
        return resultList;
    }

    std::string target = item.getString(FILTER_CONFIG_KEY_TARGET);

    std::string name{};
    if (item.hasItem(FILTER_CONFIG_KEY_NAME)) {
        name = item.getString(FILTER_CONFIG_KEY_NAME);
        AF_LOGD("force filter use : %s", name.c_str());
    }

    for (int i = 0; i < _nextSlot; ++i) {
        Cicada::IVideoFilter *filter = filterQueue[i];
        const string &filterName = filter->getName();
        assert(!filterName.empty());

        bool supported = filter->is_supported(target, videoInfo.width, videoInfo.height, videoInfo.format);
        if (supported && (name.empty() || name == filterName)) {
            resultList.push_back(filter);
        } else {
            AF_LOGD("filter %s not support target : %s", filterName.c_str(), target.c_str());
        }
    }

    return resultList;
}

void filterPrototype::dummy(int dummy)
{
    Cicada::ffmpegVideoFilter dummyFfmpegVideoFilter{};
}
