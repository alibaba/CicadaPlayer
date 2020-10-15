//
// Created by moqi on 2018/10/30.
//

#ifndef CICADA_PLAYER_IAUDIOFILTER_H
#define CICADA_PLAYER_IAUDIOFILTER_H

#include <string>
#include <utils/AFMediaType.h>
#include <base/media/IAFPacket.h>

using namespace std;
namespace Cicada{
    class IAudioFilter {
#define A_FILTER_FLAG_TEMPO (1 << 1)
#define A_FILTER_FLAG_VOLUME (1 << 2)
    public:
        typedef IAFFrame::audioInfo format;

        IAudioFilter(const format &srcFormat, const format &dstFormat, bool active)
            : mSrcFormat(srcFormat), mDstFormat(dstFormat), mActive(active){};
        virtual ~IAudioFilter() = default;

//        virtual bool beSupported(const char* capacity) = 0;
        virtual bool setOption(const string &key, const string &value, const string &capacity) = 0;

        attribute_warn_unused_result virtual int init(uint64_t flags) = 0;

        virtual int push(std::unique_ptr<IAFFrame> &frame, uint64_t timeOut) = 0;

        virtual int pull(unique_ptr<IAFFrame> &frame, uint64_t timeOut) = 0;

        virtual void flush() = 0;

        void enableReferInputPts(bool value) {mReferInputPTS = value;};
    protected:

        bool mReferInputPTS = false;

        format mSrcFormat;
        format mDstFormat;

        bool mActive;
    };
}


#endif //CICADA_PLAYER_IAUDIOFILTER_H
