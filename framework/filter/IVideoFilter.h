//
// Created by moqi on 2020/5/28.
//

#ifndef CICADAMEDIA_IVIDEOFILTER_H
#define CICADAMEDIA_IVIDEOFILTER_H
#include <base/media/IAFPacket.h>
#include <memory>
#include <string>
#include <utils/AFMediaType.h>

namespace Cicada {
    class IVideoFilter {
    public:
        typedef IAFFrame::videoInfo format;

        IVideoFilter(const format &srcFormat, const format &dstFormat, bool active)
            : mSrcFormat(srcFormat), mDstFormat(dstFormat), mActive(active){};
        virtual ~IVideoFilter() = default;

        //        virtual bool beSupported(const char* capacity) = 0;
        virtual bool setOption(const std::string &key, const std::string &value, const std::string &capacity) = 0;

        attribute_warn_unused_result virtual int init() = 0;

        virtual int push(std::unique_ptr<IAFFrame> &frame, uint64_t timeOut) = 0;

        virtual int pull(std::unique_ptr<IAFFrame> &frame, uint64_t timeOut) = 0;

        virtual void flush() = 0;

    protected:
        format mSrcFormat;
        format mDstFormat;

        bool mActive;
    };
}// namespace Cicada


#endif//CICADAMEDIA_IVIDEOFILTER_H
