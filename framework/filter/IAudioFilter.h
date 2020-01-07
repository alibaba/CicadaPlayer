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
    public:
        typedef IAFFrame::audioInfo format;

        IAudioFilter(const format &srcFormat, const format &dstFormat) :
                mSrcFormat(srcFormat), mDstFormat(dstFormat)
        {
        };
        virtual ~IAudioFilter() = default;

//        virtual bool beSupported(const char* capacity) = 0;
        virtual bool setOption(const string &key, const string &value, const string &capacity) = 0;

        attribute_warn_unused_result
        virtual int init() = 0;

        virtual int push(std::unique_ptr<IAFFrame> &frame, uint64_t timeOut) = 0;

        virtual int pull(unique_ptr<IAFFrame> &frame, uint64_t timeOut) = 0;

        virtual void flush() = 0;

        void enableReferInputPts(bool value) {mReferInputPTS = value;};
    protected:

        bool mReferInputPTS = false;

        format mSrcFormat;
        format mDstFormat;
    };
}


#endif //CICADA_PLAYER_IAUDIOFILTER_H
