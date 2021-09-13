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

    bool needTextureFilter = (feature & IVideoFilter::Feature::Texture);
    bool needBufferFilter = (feature & IVideoFilter::Feature::Buffer);
    bool needHDRFilter = (feature & IVideoFilter::Feature::HDR);

    IVideoFilter *filter{nullptr};
    int maxScore = 0;
    for (IVideoFilter *filterItem : supportFilters) {
        int score = 0;

        if(needHDRFilter && !filterItem->isFeatureSupported(IVideoFilter::Feature::HDR)) {
            AF_LOGD("filter %s not support HDR" , filterItem->getName().c_str());
            continue;
        }

        if (needTextureFilter && filterItem->isFeatureSupported(IVideoFilter::Feature::Texture)) {
            score += 100;
        }

        if (needBufferFilter && filterItem->isFeatureSupported(IVideoFilter::Feature::Buffer)) {
            score += 50;
        }

        if (score == 0) {
            AF_LOGD("filter %s can not match the feature %d", filterItem->getName().c_str(), feature);
            continue;
        }

        if (filterItem->isFeatureSupported(IVideoFilter::Feature::PassThrough)) {
            score += 1;
        }

        if (score >= maxScore) {
            filter = filterItem;
            maxScore = score;
        }
    }

    if (filter == nullptr) {
        AF_LOGE("not found match filter for feature : %d", feature);
        return nullptr;
    }

    IVideoFilter *finalFilter = filter->clone(videoInfo, videoInfo, active);
    return finalFilter;
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
