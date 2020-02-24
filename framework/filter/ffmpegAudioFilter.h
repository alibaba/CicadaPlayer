//
// Created by moqi on 2018/10/30.
//

#ifndef CICADA_PLAYER_FFMPEGAUDIOFILTER_H
#define CICADA_PLAYER_FFMPEGAUDIOFILTER_H
extern "C" {
#include <libavfilter/avfilter.h>
};

#include "IAudioFilter.h"
#include <utils/afThread.h>
#include <queue>
#include <boost/lockfree/spsc_queue.hpp>

namespace Cicada {
    class ffmpegAudioFilter : public IAudioFilter {
    public:
        ffmpegAudioFilter(const format &srcFormat, const format &dstFormat);

        ~ffmpegAudioFilter() override;

        static bool isSupported(string capacity);

        bool setOption(const string &key, const string &value, const string &capacity) override;

        int init() override;


        int push(std::unique_ptr<IAFFrame> &frame, uint64_t timeOut) override;

        int pull(unique_ptr<IAFFrame> &frame, uint64_t timeOut) override;

        void flush() override;


    private:
        int FilterLoop();

        int createSrcBufferFilter();

        AVFilterContext *createFilter(const char *name, const char *opt);

    private:
        typedef boost::lockfree::spsc_queue<IAFFrame *, boost::lockfree::capacity<10>> spsc_queue;
        std::atomic<double> mRate{1.0};
        double mVolume = 1.0;
        AVFilterGraph *m_pFilterGraph{};
        afThread *mPThread{};

        AVFilterContext *mAbuffer_ctx{};
        AVFilterContext *mAbuffersink_ctx{};
        spsc_queue mInPut{};
        spsc_queue mOutPut{};
        boost::lockfree::spsc_queue<int64_t, boost::lockfree::capacity<10>> mPts{};
        std::mutex mMutexRate{};
        int64_t mFirstPts = INT64_MIN;
        int64_t mDeltaPts = 0;

        std::atomic<int64_t> mLastInputPts {INT64_MIN};
        std::atomic<int64_t> mLastInPutDuration {0};

        int addFilter(AVFilterContext **current, const char *name, const char *options_str);
    };
}


#endif //CICADA_PLAYER_FFMPEGAUDIOFILTER_H
