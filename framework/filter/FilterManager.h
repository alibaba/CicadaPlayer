//
// Created by yiliang on 2021/8/20.
//

#ifndef CMDLINE_FILTERMANAGER_H
#define CMDLINE_FILTERMANAGER_H

#include "VideoFilterChain.h"
#include <base/media/IAFPacket.h>
#include <utils/CicadaJSON.h>

namespace Cicada {
    class FilterManager {
    public:
        FilterManager(const IAFFrame::videoInfo &videoInfo, const CicadaJSONArray &config);

        ~FilterManager();

        void setStreamMeta(const Stream_meta* meta);

        bool push(std::unique_ptr<IAFFrame> &frame);

        bool pull(int format, std::unique_ptr<IAFFrame> &frame);

        void setNeedFilter(bool needFilter);

        void setInvalid(const std::string &target, bool invalid);

        bool initFilter(IVideoFilter::Feature feature, int filterType);

        bool hasFilter(IVideoFilter::Feature feature, const std::string &target);

        bool isInvalid(IVideoFilter::Feature feature, const std::string &target);

        void updateFilter(const std::string &target, const std::string &options);

        void setSpeed(float speed);

    private:
        void setupFilterChains();


    private:
        std::map<IVideoFilter::Feature, std::unique_ptr<VideoFilterChain>> mFilterChains{};

        const Stream_meta * streamMeta{nullptr};

        IAFFrame::videoInfo mVideoInfo{};
        std::string mFilterConfig{};
        bool mFilterInited{false};
        bool mNeedFilter{true};
    };
}// namespace Cicada
#endif//CMDLINE_FILTERMANAGER_H
