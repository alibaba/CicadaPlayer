//
// Created by moqi on 2020/5/28.
//

#ifndef CICADAMEDIA_FFMPEGVIDEOFILTER_H
#define CICADAMEDIA_FFMPEGVIDEOFILTER_H

extern "C" {
#include <libavfilter/avfilter.h>
}

#include "IVideoFilter.h"
#include "filterPrototype.h"
#include <base/media/spsc_queue.h>
#include <utils/afThread.h>
namespace Cicada {
    class ffmpegVideoFilter : public IVideoFilter, protected filterPrototype {
    public:
        ffmpegVideoFilter();

        ffmpegVideoFilter(const IAFFrame::videoInfo &srcFormat, const IAFFrame::videoInfo &dstFormat, bool active);
        ~ffmpegVideoFilter() override;

        bool setOption(const std::string &key, const std::string &value, const std::string &capacity) override;

        attribute_warn_unused_result bool init(int type) override;

        int push(std::unique_ptr<IAFFrame> &frame, uint64_t timeOut) override;

        int pull(std::unique_ptr<IAFFrame> &frame, uint64_t timeOut) override;

        void flush() override;

        std::string getName() override;

        bool is_supported(const std::string &target, int width, int height, int format) override;

        Cicada::IVideoFilter *clone(IAFFrame::videoInfo srcFormat, IAFFrame::videoInfo dstFormat, bool active) override;

        bool isFeatureSupported(Feature feature) override;

    private:
        int FilterLoop();

        explicit ffmpegVideoFilter(int dummy)
        {
            addPrototype(this);
        }

        static ffmpegVideoFilter se;

    private:
        AVFilterContext *buffersink_ctx{nullptr};
        AVFilterContext *buffersrc_ctx{nullptr};
        AVFilterGraph *m_pFilterGraph{nullptr};
        std::string mOpt{};
        afThread *mPThread{nullptr};

        SpscQueue<IAFFrame *> mOutPut{10};
        SpscQueue<IAFFrame *> mInPut{10};
    };
}// namespace Cicada


#endif//CICADAMEDIA_FFMPEGVIDEOFILTER_H
