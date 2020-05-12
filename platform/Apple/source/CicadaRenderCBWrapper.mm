#import "CicadaRenderCBWrapper.h"
#import <base/media/PBAFFrame.h>
#import "CicadaRenderDelegate.h"

bool CicadaRenderCBWrapper::OnRenderFrame(void *userData, IAFFrame *frame)
{
    id<CicadaRenderDelegate> delegate = (__bridge id<CicadaRenderDelegate>)userData;
    if (nullptr == delegate) {
        return false;
    }

    if (frame->getType() == IAFFrame::FrameTypeVideo) {
        switch (frame->getInfo().format) {
            case AF_PIX_FMT_APPLE_PIXEL_BUFFER: {
                auto *ppBFrame = dynamic_cast<PBAFFrame *>(frame);
                CVPixelBufferRef pixelBuffer = ppBFrame->getPixelBuffer();
                if ([delegate respondsToSelector:@selector(onVideoPixelBuffer:pts:)]){
                    return [delegate onVideoPixelBuffer:pixelBuffer pts:frame->getInfo().pts];
                }
                break;
            }
            case AF_PIX_FMT_YUV420P:
                if ([delegate respondsToSelector:@selector(onVideoPixelBuffer:pts:)]){
                    return [delegate onVideoRawBuffer:frame->getData()
                                             lineSize:frame->getLineSize()
                                                  pts:frame->getInfo().pts
                                                width:frame->getInfo().video.width
                                               height:frame->getInfo().video.height];
                }
                break;
            default:
                break;
        }
    }

    return false;
}
