//
// Created by pingkai on 2021/1/26.
//

#ifndef CICADAMEDIA_PIXELBUFFERCONVERTOR_H
#define CICADAMEDIA_PIXELBUFFERCONVERTOR_H


#include <CoreVideo/CoreVideo.h>
#include <base/media/PBAFFrame.h>
extern "C" {
#include <libavutil/frame.h>
#include <libswscale/swscale.h>
}
namespace Cicada {
    class pixelBufferConvertor {
    public:
        pixelBufferConvertor();
        ~pixelBufferConvertor();

        IAFFrame *convert(IAFFrame *frame);

    private:
        int init(const IAFFrame::videoInfo &src);

    private:
        struct SwsContext *sws_ctx{nullptr};
        AVFrame *mOutFrame{nullptr};
        IAFFrame::videoInfo mVideoInfo{};
    };
}// namespace Cicada


#endif//CICADAMEDIA_PIXELBUFFERCONVERTOR_H
