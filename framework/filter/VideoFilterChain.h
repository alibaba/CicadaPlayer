//
// Created by yiliang on 2021/8/27.
//

#ifndef SOURCE_VIDEOFILTERCHAIN_H
#define SOURCE_VIDEOFILTERCHAIN_H

#include "IVideoFilter.h"
#include <base/media/TextureFrame.h>
#include <base/media/spsc_queue.h>
#include <functional>
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

        bool isInvalid(const std::string &target);

        bool removeFilter(const std::string &target);

        void setSpeed(float speed);

        void clearBuffer();

        bool hasFilter(const std::string &target);

        void setDCACb(const std::function<void(int level, const std::string &content)> &func);

    private:
        void filterLoop();

        static void pullFrames(std::unique_ptr<IVideoFilter> &filter, SpscQueue<IAFFrame *> &outFrames);

        static void pushFrames(std::unique_ptr<IVideoFilter> &filter, SpscQueue<IAFFrame *> &inputFrames);

        static void swapFrames(SpscQueue<IAFFrame *> &dstFrames, SpscQueue<IAFFrame *> &srcFrames);

        static void clearFrames(SpscQueue<IAFFrame *> &frames);

    private:
        std::map<std::string, std::unique_ptr<IVideoFilter>> mVideoFiltersMap{};
        SpscQueue<IAFFrame *> mInPutFrames{10};
        SpscQueue<IAFFrame *> mOutPutFrames{10};
        std::function<void(int level, const std::string &content)> sendEvent;
    };
}// namespace Cicada
#endif//SOURCE_VIDEOFILTERCHAIN_H
