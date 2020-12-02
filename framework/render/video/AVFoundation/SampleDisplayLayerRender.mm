//
// Created by pingkai on 2020/11/27.
//

#import "SampleDisplayLayerRender.h"
#include "DisplayLayerImpl-interface.h"
#include <base/media/PBAFFrame.h>
#if TARGET_OS_IPHONE
#import <UIKit/UIKit.h>
#endif

typedef void (^RetainSelfBlock)(void);
@implementation SampleDisplayLayerRender {
    RetainSelfBlock _retainBlock;
}
DisplayLayerImpl::DisplayLayerImpl() : self(NULL)
{}
DisplayLayerImpl::~DisplayLayerImpl()
{
    [(__bridge id) self breakRetainCycly];
}
void DisplayLayerImpl::init()
{
    SampleDisplayLayerRender *object = [[SampleDisplayLayerRender alloc] init];
    object->_retainBlock = ^{
      [object class];
    };
    self = (__bridge void *) object;
}
int DisplayLayerImpl::createLayer()
{
    return [(__bridge id) self createLayer];
}

int DisplayLayerImpl::renderFrame(std::unique_ptr<IAFFrame> &frame)
{
    auto *pbafFrame = dynamic_cast<PBAFFrame *>(frame.get());
    if (pbafFrame) {
        [(__bridge id) self displayPixelBuffer:pbafFrame->getPixelBuffer()];
    }
    frame = NULL;

    return 0;
}
void DisplayLayerImpl::setDisplay(void *display)
{
    return [(__bridge id) self setDisplay:display];
}

- (void)setDisplay:(void *)layer
{
    if (layer != self._layer) {
        dispatch_async(dispatch_get_main_queue(), ^{
          self._layer = static_cast<CALayer *>(layer);
          self.displayLayer = [AVSampleBufferDisplayLayer layer];
          self.displayLayer.videoGravity = AVLayerVideoGravityResizeAspect;
          [self._layer addSublayer:self.displayLayer];
          self._layer.masksToBounds = YES;
          self.displayLayer.frame = self._layer.bounds;
          [self._layer addObserver:self forKeyPath:@"bounds" options:NSKeyValueObservingOptionNew context:nil];
        });
    }
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary<NSString *, id> *)change context:(void *)context
{
    if ([keyPath isEqualToString:@"bounds"]) {
        CGRect bounds = [change[NSKeyValueChangeNewKey] CGRectValue];
        self.displayLayer.frame = CGRectMake(0, 0, bounds.size.width, bounds.size.height);
    }
}

- (int)createLayer
{
    NSLog(@"createLayer");
    return 0;
};

#if 0
- (CVPixelBufferRef)copyPixelBuffer:(CVPixelBufferRef)pixelBuffer
{
    CVPixelBufferRef copyBuffer = nil;
    NSDictionary *pixelAttributes = @{(id) kCVPixelBufferIOSurfacePropertiesKey: @{}};


    size_t width = CVPixelBufferGetWidth(pixelBuffer);
    size_t height = CVPixelBufferGetHeight(pixelBuffer);
    OSType pix_fmt = CVPixelBufferGetPixelFormatType(pixelBuffer);

    CVReturn result =
            CVPixelBufferCreate(kCFAllocatorDefault, width, height, pix_fmt, (__bridge CFDictionaryRef)(pixelAttributes), &copyBuffer);

    if (result != kCVReturnSuccess) {
        return nullptr;
    }
    uint8_t *DestPlane;
    uint8_t *SrcPlane;
    size_t line_size;

    for (int j = 0; j < CVPixelBufferGetPlaneCount(pixelBuffer); j++) {
        CVPixelBufferLockBaseAddress(copyBuffer, j);
        CVPixelBufferLockBaseAddress(pixelBuffer, j);
        DestPlane = (uint8_t *) CVPixelBufferGetBaseAddressOfPlane(copyBuffer, j);
        SrcPlane = (uint8_t *) CVPixelBufferGetBaseAddressOfPlane(pixelBuffer, j);
        line_size = CVPixelBufferGetBytesPerRowOfPlane(pixelBuffer, j);
        height = CVPixelBufferGetHeightOfPlane(pixelBuffer, j);
        width = CVPixelBufferGetWidthOfPlane(pixelBuffer, j);


        for (int i = 0; i < height; i++) {
            memcpy(DestPlane + i * line_size, SrcPlane + i * line_size, line_size);
        }
        CVPixelBufferUnlockBaseAddress(copyBuffer, j);
        CVPixelBufferUnlockBaseAddress(pixelBuffer, j);
    }

    return copyBuffer;
}
#endif

- (void)displayPixelBuffer:(CVPixelBufferRef)pixelBuffer
{
    if (!pixelBuffer || !self.displayLayer) {
        return;
    }

    CMSampleTimingInfo timing = {kCMTimeInvalid, kCMTimeInvalid, kCMTimeInvalid};

    CMVideoFormatDescriptionRef videoInfo = NULL;
    OSStatus result = CMVideoFormatDescriptionCreateForImageBuffer(NULL, pixelBuffer, &videoInfo);
    NSParameterAssert(result == 0 && videoInfo != NULL);

    CMSampleBufferRef sampleBuffer = NULL;
    result = CMSampleBufferCreateForImageBuffer(kCFAllocatorDefault, pixelBuffer, true, NULL, NULL, videoInfo, &timing, &sampleBuffer);
    NSParameterAssert(result == 0 && sampleBuffer != NULL);
    CFRelease(videoInfo);

    CFArrayRef attachments = CMSampleBufferGetSampleAttachmentsArray(sampleBuffer, YES);
    CFMutableDictionaryRef dict = (CFMutableDictionaryRef) CFArrayGetValueAtIndex(attachments, 0);
    CFDictionarySetValue(dict, kCMSampleAttachmentKey_DisplayImmediately, kCFBooleanTrue);

    if (self.displayLayer.status == AVQueuedSampleBufferRenderingStatusFailed) {
        [self.displayLayer flush];
    }
    [self.displayLayer enqueueSampleBuffer:sampleBuffer];
    CFRelease(sampleBuffer);
}
- (void)breakRetainCycly
{
    _retainBlock = nil;
}

- (void)dealloc
{
    [super dealloc];
}

@end
