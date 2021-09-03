//
// Created by yiliang on 2021/8/27.
//

#ifndef SOURCE_VIDEOFILTERCHAIN_H
#define SOURCE_VIDEOFILTERCHAIN_H

#include "IVideoFilter.h"
#include <base/media/TextureFrame.h>
#include <base/media/spsc_queue.h>
#include <map>
#include <memory>

namespace Cicada {

    class VideoFilterChain {
    public:
        VideoFilterChain();

        ~VideoFilterChain();

        void addFilter(const std::string &config, std::unique_ptr<IVideoFilter> videoFilter);

        bool init(int type);

        int push(std::unique_ptr<IAFFrame> &frame);

        int pull(std::unique_ptr<IAFFrame> &frame);

        bool empty();

        void updateFilter(const std::string &target, const std::string &options);

        void setInvalid(const std::string &target, bool invalid);

        bool removeFilter(const std::string &target);

    private:
        std::map<std::string, std::unique_ptr<IVideoFilter>> mVideoFiltersMap{};
        std::unique_ptr<IAFFrame> mInPutFrame{nullptr};
    };
}// namespace Cicada
#endif//SOURCE_VIDEOFILTERCHAIN_H
