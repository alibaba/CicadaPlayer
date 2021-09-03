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

        bool doFilter(std::unique_ptr<IAFFrame> &frame);

        void setNeedFilter(bool needFilter);

        void setInvalid(const std::string &target, bool invalid);

        bool initFilter(IVideoFilter::Feature feature, int filterType);

        bool hasFilter(IVideoFilter::Feature feature);

        void updateFilter(const std::string &target, const std::string &options);

    private:
        void setupFilterChains();

        bool doFilter(IVideoFilter::Feature feature, std::unique_ptr<IAFFrame> &frame);

    private:
        std::map<IVideoFilter::Feature, std::unique_ptr<VideoFilterChain>> mFilterChains{};

        IAFFrame::videoInfo mVideoInfo{};
        std::string mFilterConfig{};
        bool mNeedFilter{true};
    };
}// namespace Cicada
#endif//CMDLINE_FILTERMANAGER_H
