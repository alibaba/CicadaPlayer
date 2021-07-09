//
// Created by pingkai on 2020/11/27.
//

#import "SampleDisplayLayerRender.h"
#include "DisplayLayerImpl-interface.h"
#include <base/media/PBAFFrame.h>

#import <mutex>
#import <render/video/glRender/base/utils.h>
#include <utility>
#if TARGET_OS_IPHONE
#import <UIKit/UIKit.h>
#endif

@implementation SampleDisplayLayerRender {
    CALayer *parentLayer;
    AVLayerVideoGravity videoGravity;
    CVPixelBufferRef renderingBuffer;
    std::mutex renderingBufferMutex;
}
DisplayLayerImpl::DisplayLayerImpl()
{}
DisplayLayerImpl::~DisplayLayerImpl()
{
    if (renderHandle) {
        CFRelease(renderHandle);
    }
}
void DisplayLayerImpl::init()
{
    SampleDisplayLayerRender *object = [[SampleDisplayLayerRender alloc] init];
    renderHandle = (__bridge_retained void *) object;
}
int DisplayLayerImpl::createLayer()
{
    return [(__bridge id) renderHandle createLayer];
}
void DisplayLayerImpl::applyRotate()
{
    int rotate = static_cast<int>(llabs((mFrameRotate + mRotate) % 360));
    [(__bridge id) renderHandle setRotateMode:rotate];
}

void DisplayLayerImpl::SetVideoRenderingCallBack(videoRenderingFrameCB cb, void *userData)
{
    mRenderingCb = cb;
    mRenderingCbUserData = userData;
}

int DisplayLayerImpl::renderFrame(std::unique_ptr<IAFFrame> &frame)
{
    if (!frame) {
        return 0;
    }
    auto *pbafFrame = dynamic_cast<PBAFFrame *>(frame.get());

    if (frame->getInfo().video.rotate != mFrameRotate) {
        mFrameRotate = frame->getInfo().video.rotate;
        applyRotate();
    }
    if (frame->getInfo().video.height != mFrameHeight || frame->getInfo().video.width != mFrameWidth ||
        frame->getInfo().video.dar != mFrameDar) {

        mFrameHeight = frame->getInfo().video.height;
        mFrameWidth = frame->getInfo().video.width;
        mFrameDar = frame->getInfo().video.dar;


        if (mFrameDar) {
            mFrameDisplayWidth = static_cast<int>(frame->getInfo().video.dar * frame->getInfo().video.height);
        } else {
            mFrameDisplayWidth = frame->getInfo().video.width;
        }
        mFrameDisplayHeight = frame->getInfo().video.height;
        CGSize size;
        size.width = mFrameDisplayWidth;
        size.height = mFrameDisplayHeight;
        [(__bridge id) renderHandle setVideoSize:size];
    }

    bool rendered = false;
    if (mRenderingCb) {
        CicadaJSONItem params{};
        rendered = mRenderingCb(mRenderingCbUserData, frame.get(), params);
    }

    if (rendered) {
        return -1;
    }


    if (pbafFrame) {
        [(__bridge id) renderHandle displayPixelBuffer:pbafFrame->getPixelBuffer()];
    }
    frame = NULL;

    return 0;
}
void DisplayLayerImpl::setDisplay(void *display)
{
    return [(__bridge id) renderHandle setDisplay:display];
}
void DisplayLayerImpl::clearScreen()
{
    [(__bridge id) renderHandle clearScreen];
}

void DisplayLayerImpl::setScale(IVideoRender::Scale scale)
{
    [(__bridge id) renderHandle setVideoScale:scale];
}

void DisplayLayerImpl::setFlip(IVideoRender::Flip flip)
{
    if (mFlip != flip) {
        mFlip = flip;
        int rotate = static_cast<int>(llabs((mFrameRotate + mRotate) % 180));
        switch (flip) {
            case IVideoRender::Flip_None:
                [(__bridge id) renderHandle setMirrorTransform:CATransform3DMakeRotation(0, 0, 0, 0)];
                break;
            case IVideoRender::Flip_Horizontal:
                if (rotate == 0) {
                    [(__bridge id) renderHandle setMirrorTransform:CATransform3DMakeRotation(M_PI, 0, 1, 0)];
                } else {
                    [(__bridge id) renderHandle setMirrorTransform:CATransform3DMakeRotation(M_PI, 1, 0, 0)];
                }
                break;
            case IVideoRender::Flip_Vertical:
                if (rotate == 0) {
                    [(__bridge id) renderHandle setMirrorTransform:CATransform3DMakeRotation(M_PI, 1, 0, 0)];
                } else {
                    [(__bridge id) renderHandle setMirrorTransform:CATransform3DMakeRotation(M_PI, 0, 1, 0)];
                }
                break;
            default:
                break;
        }
        [(__bridge id) renderHandle applyTransform];
    }
}

void DisplayLayerImpl::setBackgroundColor(uint32_t color)
{
    float fColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    cicada::convertToGLColor(color, fColor);
    CGColorSpaceRef rgb = CGColorSpaceCreateDeviceRGB();
    CGColorRef cgColor = CGColorCreate(rgb, (CGFloat[]){fColor[0], fColor[1], fColor[2], fColor[3]});
    [(__bridge id) renderHandle setBGColour:cgColor];
    CGColorRelease(cgColor);
    CGColorSpaceRelease(rgb);
}

void DisplayLayerImpl::captureScreen(std::function<void(uint8_t *, int, int)> func)
{
#if TARGET_OS_IPHONE
    void *img = [(__bridge id) renderHandle captureScreen];
    func((uint8_t *) img, -1, -1);
//    CFRelease(img);
#endif
}

void DisplayLayerImpl::setRotate(IVideoRender::Rotate rotate)
{
    if (mRotate != rotate) {
        mRotate = rotate;
        applyRotate();
    }
}

- (instancetype)init
{
    videoGravity = AVLayerVideoGravityResizeAspect;
    parentLayer = nil;
    _bGColour = nullptr;
    _mirrorTransform = CATransform3DMakeRotation(0, 0, 0, 0);
    _rotateTransform = CATransform3DMakeRotation(0, 0, 0, 0);
    _scaleTransform = CATransform3DMakeRotation(0, 0, 0, 0);
    renderingBuffer = nullptr;
    return self;
}

- (CATransform3D)CalculateTransform
{
    CATransform3D transform = CATransform3DConcat(_mirrorTransform, _rotateTransform);
    if (videoGravity == AVLayerVideoGravityResizeAspect) {
        float scale = 1;
        if ((_frameSize.width > _frameSize.height && _rotateMode % 180 == 0) ||
            (_frameSize.width < _frameSize.height && _rotateMode % 180 != 0)) {
            scale = static_cast<float>(self.displayLayer.bounds.size.width / [self getVideoSize].width);
        } else {
            scale = static_cast<float>(self.displayLayer.bounds.size.height / [self getVideoSize].height);
        }
        _scaleTransform = CATransform3DMakeScale(scale, scale, 1);
    } else if (videoGravity == AVLayerVideoGravityResizeAspectFill) {
        float scale = 1;
        if (!((_frameSize.width > _frameSize.height && _rotateMode % 180 == 0) ||
              (_frameSize.width < _frameSize.height && _rotateMode % 180 != 0))) {
            scale = static_cast<float>(self.displayLayer.bounds.size.width / [self getVideoSize].width);
        } else {
            scale = static_cast<float>(self.displayLayer.bounds.size.height / [self getVideoSize].height);
        }
        _scaleTransform = CATransform3DMakeScale(scale, scale, 1);
    } else if (videoGravity == AVLayerVideoGravityResize) {
        float scalex;
        float scaley;
        if (!_isFillWidth) {
            scalex = static_cast<float>(self.displayLayer.bounds.size.width / [self getVideoSize].width);
            scaley = static_cast<float>(self.displayLayer.bounds.size.height / [self getVideoSize].height);
        } else {
            scalex = static_cast<float>(self.displayLayer.bounds.size.width / [self getVideoSize].width);
            scaley = static_cast<float>(self.displayLayer.bounds.size.height / [self getVideoSize].height);
        }
        _scaleTransform = CATransform3DMakeScale(scalex, scaley, 1);
    }
    return CATransform3DConcat(transform, _scaleTransform);
}

- (void)applyTransform
{
    if (self.displayLayer) {
        dispatch_async(dispatch_get_main_queue(), ^{
          self.displayLayer.transform = [self CalculateTransform];
          [self.displayLayer removeAllAnimations];
        });
    }
}

- (void)setDisplay:(void *)layer
{
    if (layer != (__bridge void *) parentLayer) {
        parentLayer = (__bridge CALayer *) layer;
        dispatch_async(dispatch_get_main_queue(), ^{
          if (!self.displayLayer) {
              self.displayLayer = [AVSampleBufferDisplayLayer layer];
              self.displayLayer.videoGravity = AVLayerVideoGravityResizeAspect;
          }
          [parentLayer addSublayer:self.displayLayer];
          if (_bGColour) {
              parentLayer.backgroundColor = _bGColour;
          }
          parentLayer.masksToBounds = YES;
          self.displayLayer.frame = parentLayer.bounds;
          self.displayLayer.transform = [self CalculateTransform];
          [parentLayer addObserver:self forKeyPath:@"bounds" options:NSKeyValueObservingOptionNew context:nil];
        });
    }
}

- (void)setBGColour:(CGColorRef)bGColour
{
    assert(bGColour);
    CGColorRef color = CGColorRetain(bGColour);
    dispatch_async(dispatch_get_main_queue(), ^{
      if (_bGColour) {
          CGColorRelease(_bGColour);
      }
      _bGColour = color;
      if (parentLayer) {
          parentLayer.backgroundColor = _bGColour;
      }
    });
}

- (void)setVideoScale:(IVideoRender::Scale)scale
{
    switch (scale) {
        case IVideoRender::Scale_AspectFit:
            videoGravity = AVLayerVideoGravityResizeAspect;
            break;
        case IVideoRender::Scale_AspectFill:
            videoGravity = AVLayerVideoGravityResizeAspectFill;
            break;
        case IVideoRender::Scale_Fill:
            videoGravity = AVLayerVideoGravityResize;
            break;
        default:
            return;
    }
    [self applyTransform];
}

- (void)setRotateMode:(int)rotateMode
{
    _rotateMode = rotateMode;
    switch (rotateMode) {
        case 0:
            _rotateTransform = CATransform3DMakeRotation(0, 0, 0, 0);
            break;
        case 90:
            _rotateTransform = CATransform3DMakeRotation(M_PI_2, 0, 0, 1);
            break;
        case 180:
            _rotateTransform = CATransform3DMakeRotation(M_PI, 0, 0, 1);
            break;
        case 270:
            _rotateTransform = CATransform3DMakeRotation(M_PI_2, 0, 0, -1);
            break;

        default:
            break;
    }
    [self applyTransform];
}
#if TARGET_OS_IPHONE
- (void *)captureScreen
{
    CGSize newSize = CGSizeMake(10, 10);
    UIImage *uiImage = nil;
    CGFloat scale = [UIScreen mainScreen].scale;
    if (self.displayLayer) {
        newSize = CGSizeMake(self.displayLayer.bounds.size.width * scale, self.displayLayer.bounds.size.height * scale);
    }
    {
        if (renderingBuffer && self.displayLayer) {
            CGSize imageSize = CGSizeMake(_videoSize.width * scale, _videoSize.height * scale);
            if (_rotateMode % 180) {
                imageSize = CGSizeMake(_videoSize.height * scale, _videoSize.width * scale);
            }
            CIImage *ciImage = nullptr;
            {
                std::lock_guard<std::mutex> lock(renderingBufferMutex);
                if (renderingBuffer) {
                    ciImage = [CIImage imageWithCVPixelBuffer:renderingBuffer];
                }
            }
            if (ciImage) {
                uiImage = [UIImage imageWithCIImage:ciImage];
                UIGraphicsBeginImageContext(imageSize);

                [uiImage drawInRect:CGRectMake(0, 0, imageSize.width, imageSize.height)];
                uiImage = UIGraphicsGetImageFromCurrentImageContext();
                UIGraphicsEndImageContext();
            }
        }
    }

    if (uiImage) {
        uiImage = [self imageTransform:uiImage inSize:newSize];
    }

    return (void *) CFBridgingRetain(uiImage);
}

- (UIImage *)imageTransform:(UIImage*)img inSize:(CGSize)newSize
{
    UIGraphicsBeginImageContext(newSize);
    CGContextRef ctx = UIGraphicsGetCurrentContext();
    CGImageRef cgImage = img.CGImage;
    
    if (!_bGColour) {
        _bGColour = [UIColor blackColor].CGColor;
    }

    //draw the background color
    CGRect bgRect = CGRectMake(0.0f, 0.0f, newSize.width, newSize.height);
    CGContextSetFillColorWithColor(ctx, _bGColour);
    CGContextSetBlendMode(ctx, kCGBlendModeNormal);
    CGContextFillRect(ctx, bgRect);

    if (ctx == NULL || cgImage == NULL) {
        UIImage *image = UIGraphicsGetImageFromCurrentImageContext();
        UIGraphicsEndImageContext();
        return image ?: img;
    }

    CGContextTranslateCTM(ctx, newSize.width / 2.0, newSize.height / 2.0);
    CGContextConcatCTM(ctx, CATransform3DGetAffineTransform(self.displayLayer.transform));
    CGContextScaleCTM(ctx, 1, -1);
    CGPoint origin = CGPointMake(-(img.size.width / 2.0), -(img.size.height / 2.0));
    CGRect rect = CGRectZero;
    rect.origin = origin;
    rect.size = img.size;
    CGContextDrawImage(ctx, rect, cgImage);

    UIImage *image = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();
    return image ?: img;
}

#endif

- (void)clearScreen
{
    [self.displayLayer flushAndRemoveImage];
    std::lock_guard<std::mutex> lock(renderingBufferMutex);
    if (renderingBuffer) {
        CVPixelBufferRelease(renderingBuffer);
    }
    renderingBuffer = nullptr;
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary<NSString *, id> *)change context:(void *)context
{
    if ([keyPath isEqualToString:@"bounds"]) {
        CGRect bounds = [change[NSKeyValueChangeNewKey] CGRectValue];
        self.displayLayer.frame = CGRectMake(0, 0, bounds.size.width, bounds.size.height);
        self.displayLayer.bounds = CGRectMake(0, 0, bounds.size.width, bounds.size.height);
        [self getVideoSize];
        [self applyTransform];
    }
}

- (int)createLayer
{
    NSLog(@"createLayer");
    return 0;
};

- (void)setVideoSize:(CGSize)videoSize
{
    _frameSize = videoSize;
    [self getVideoSize];
    [self applyTransform];
}

- (CGSize)getVideoSize
{
    float scale = 1;

    _isFillWidth = self.displayLayer.bounds.size.width / self.displayLayer.bounds.size.height < _frameSize.width / _frameSize.height;

    if (_isFillWidth) {
        scale = static_cast<float>(self.displayLayer.bounds.size.width / _frameSize.width);
    } else {
        scale = static_cast<float>(self.displayLayer.bounds.size.height / _frameSize.height);
    }
    if (_rotateMode % 180) {
        _videoSize = CGSizeMake(_frameSize.height * scale, _frameSize.width * scale);
    } else {
        _videoSize = CGSizeMake(_frameSize.width * scale, _frameSize.height * scale);
    }
    return _videoSize;
}

- (void)displayPixelBuffer:(CVPixelBufferRef)pixelBuffer
{
    if (!pixelBuffer || !self.displayLayer) {
        return;
    }
    {
        std::lock_guard<std::mutex> lock(renderingBufferMutex);
        if (renderingBuffer) {
            CVPixelBufferRelease(renderingBuffer);
        }

        renderingBuffer = CVPixelBufferRetain(pixelBuffer);
    }

    CMSampleTimingInfo timing = {kCMTimeInvalid, kCMTimeInvalid, kCMTimeInvalid};

    CMVideoFormatDescriptionRef videoInfo = nullptr;
    OSStatus result = CMVideoFormatDescriptionCreateForImageBuffer(nullptr, pixelBuffer, &videoInfo);
    NSParameterAssert(result == 0 && videoInfo != nullptr);

    CMSampleBufferRef sampleBuffer = nullptr;
    result = CMSampleBufferCreateForImageBuffer(kCFAllocatorDefault, pixelBuffer, true, NULL, NULL, videoInfo, &timing, &sampleBuffer);
    NSParameterAssert(result == 0 && sampleBuffer != nullptr);
    CFRelease(videoInfo);

    CFArrayRef attachments = CMSampleBufferGetSampleAttachmentsArray(sampleBuffer, YES);
    auto dict = (CFMutableDictionaryRef) CFArrayGetValueAtIndex(attachments, 0);
    CFDictionarySetValue(dict, kCMSampleAttachmentKey_DisplayImmediately, kCFBooleanTrue);

    if (self.displayLayer.status == AVQueuedSampleBufferRenderingStatusFailed) {
        [self.displayLayer flush];
    }
    [self.displayLayer enqueueSampleBuffer:sampleBuffer];
    CFRelease(sampleBuffer);
}

- (void)dealloc
{
    if (strcmp(dispatch_queue_get_label(DISPATCH_CURRENT_QUEUE_LABEL), dispatch_queue_get_label(dispatch_get_main_queue())) == 0) {
        if (self.displayLayer) {
            [self.displayLayer removeFromSuperlayer];
            [parentLayer removeObserver:self forKeyPath:@"bounds" context:nil];
        }
    } else {
        // FIXME: use async
        dispatch_sync(dispatch_get_main_queue(), ^{
          if (self.displayLayer) {
              [self.displayLayer removeFromSuperlayer];
              [parentLayer removeObserver:self forKeyPath:@"bounds" context:nil];
          }
        });
    }
    if (_bGColour) {
        CGColorRelease(_bGColour);
    }
    if (renderingBuffer) {
        CVPixelBufferRelease(renderingBuffer);
    }
}

@end
