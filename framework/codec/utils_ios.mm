#include "utils_ios.h"
#include "utils/frame_work_log.h"
#import <AVFoundation/AVFoundation.h>
#import <CoreFoundation/CoreFoundation.h>
#import <CoreVideo/CoreVideo.h>
#import <UIKit/UIKit.h>
#include <list>
#include <mutex>

namespace Cicada{
    float GetIosVersion()
    {
        return [[[UIDevice currentDevice] systemVersion] floatValue];
    }

    void PixelBuffer_Retain(void *pixelBuffer)
    {
        if (pixelBuffer == nullptr) {
            return;
        }
        CVPixelBufferRef p = (CVPixelBufferRef) pixelBuffer;
        CVPixelBufferRetain(p);
    }

#if 0
    void *PixelBuffer_Create(int width, int height, ImageFormat format) {
        int pixelFormat;
        if (format == IMAGE_FORMAT_BGRA) {
            pixelFormat = kCVPixelFormatType_32BGRA;
        } else if (format == IMAGE_FORMAT_YUVNV21) {
            pixelFormat = kCVPixelFormatType_420YpCbCr8BiPlanarFullRange;
        } else {
            return nullptr;
        }
        CFDictionaryRef empty; // empty value for attr value.
        CFMutableDictionaryRef attrs;
        empty = CFDictionaryCreate(kCFAllocatorDefault, NULL, NULL, 0,
                                   &kCFTypeDictionaryKeyCallBacks,
                                   &kCFTypeDictionaryValueCallBacks);
        attrs = CFDictionaryCreateMutable(kCFAllocatorDefault, 1,
                                          &kCFTypeDictionaryKeyCallBacks,
                                          &kCFTypeDictionaryValueCallBacks);
        CFDictionarySetValue(attrs, kCVPixelBufferIOSurfacePropertiesKey, empty);
        CVPixelBufferRef pixelBuffer;
        auto err = CVPixelBufferCreate(kCFAllocatorDefault, width, height,
                                       pixelFormat, attrs, &pixelBuffer);
        if (err) {
            return nullptr;
        }
        return (void *)pixelBuffer;
    }
#endif

    int PixelBuffer_GetWidth(void *pixelBuffer)
    {
        if (pixelBuffer == nullptr) {
            return 0;
        }
        CVPixelBufferRef p = (CVPixelBufferRef) pixelBuffer;
        size_t width = CVPixelBufferGetWidth(p);
        return (int) width;
    }

    int PixelBuffer_GetHeight(void *pixelBuffer)
    {
        if (pixelBuffer == nullptr) {
            return 0;
        }
        CVPixelBufferRef p = (CVPixelBufferRef) pixelBuffer;
        size_t height = CVPixelBufferGetHeight(p);
        return (int) height;
    }

    int PixelBuffer_GetStride(void *pixelBuffer)
    {
        if (pixelBuffer == nullptr) {
            return 0;
        }
        CVPixelBufferRef p = (CVPixelBufferRef) pixelBuffer;
        size_t stride = CVPixelBufferGetBytesPerRowOfPlane(p, 0);
        return (int) stride;
    }

#if 0
    ImageFormat PixelBuffer_GetPixelFormat(void *pixelBuffer) {
        if (pixelBuffer == nullptr) {
            return IMAGE_FORMAT_UNKNOW;
        }
        CVPixelBufferRef p = (CVPixelBufferRef)pixelBuffer;
        auto pixelFormat = CVPixelBufferGetPixelFormatType(p);

        if (pixelFormat == kCVPixelFormatType_32BGRA) {
            return IMAGE_FORMAT_BGRA;
        } else if (pixelFormat == kCVPixelFormatType_420YpCbCr8BiPlanarFullRange) {
            return IMAGE_FORMAT_YUVNV12;

        } else if (pixelFormat == kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange) {
            return IMAGE_FORMAT_YUVNV12;
        } else {
            return IMAGE_FORMAT_UNKNOW;
        }
    }

    ColorRange PixelBuffer_GetColorRange(void *pixelBuffer) {
        if (pixelBuffer == nullptr) {
            return COLOR_RANGE_LIMITIED;
        }

        CVPixelBufferRef p = (CVPixelBufferRef)pixelBuffer;
        auto pixelFormat = CVPixelBufferGetPixelFormatType(p);
        switch (pixelFormat) {
            case kCVPixelFormatType_422YpCbCr8FullRange:
            case kCVPixelFormatType_420YpCbCr8PlanarFullRange:
            case kCVPixelFormatType_420YpCbCr8BiPlanarFullRange:
            case kCVPixelFormatType_420YpCbCr10BiPlanarFullRange:
            case kCVPixelFormatType_422YpCbCr10BiPlanarFullRange:
            case kCVPixelFormatType_444YpCbCr10BiPlanarFullRange:
                return COLOR_RANGE_FULL;
            default:
                return COLOR_RANGE_LIMITIED;
        }
    }
#endif

    void PixelBuffer_Release(void *pixelBuffer)
    {
        CVPixelBufferRef p = (CVPixelBufferRef) pixelBuffer;
        if (pixelBuffer == nullptr) {
            return;
        }
        CVPixelBufferRelease(p);
    }

    static void IOSNotificationHandler(CFNotificationCenterRef center,
                                       void *observer, CFStringRef name,
                                       const void *object,
                                       CFDictionaryRef userInfo)
    {

        IOSNotificationManager *manager = (IOSNotificationManager *) observer;

        if (CFStringCompare(name, (__bridge CFStringRef) UIApplicationDidEnterBackgroundNotification, 0) == kCFCompareEqualTo) {
            manager->HandleSystemNotification(IOSResignActive);
        } else if (CFStringCompare(name, (__bridge CFStringRef) UIApplicationWillEnterForegroundNotification, 0) == kCFCompareEqualTo) {
            manager->HandleSystemNotification(IOSBecomeActive);
        }
    }

    void RegisterIOSNotificationObserver(IOSNotificationObserver *observer, int mask)
    {
        IOSNotificationManager::Instance()->RegisterObserver(observer, mask);
    }

    void RemoveIOSNotificationObserver(IOSNotificationObserver *observer)
    {
        IOSNotificationManager::Instance()->RemoveObserver(observer);
    }

    // make sure init once startup
    IOSNotificationManager *sNofiManagerInstance = new IOSNotificationManager();

    IOSNotificationManager *IOSNotificationManager::Instance()
    {
        return sNofiManagerInstance;
    };

    IOSNotificationManager::IOSNotificationManager()
    {
        CFNotificationCenterAddObserver(CFNotificationCenterGetLocalCenter(), this, &IOSNotificationHandler,
                                        (__bridge CFStringRef) UIApplicationDidEnterBackgroundNotification, NULL,
                                        CFNotificationSuspensionBehaviorDeliverImmediately);

        CFNotificationCenterAddObserver(CFNotificationCenterGetLocalCenter(), this, &IOSNotificationHandler,
                                        (__bridge CFStringRef) UIApplicationWillEnterForegroundNotification, NULL,
                                        CFNotificationSuspensionBehaviorDeliverImmediately);

        dispatch_async(dispatch_get_main_queue(), ^{
          // set active default, the status is Background when globle value init on iOS 12.1.4
            mbActive = true;//[UIApplication sharedApplication].applicationState != UIApplicationStateBackground;
        });
    }

    IOSNotificationManager::~IOSNotificationManager()
    {
        CFNotificationCenterRemoveEveryObserver(CFNotificationCenterGetLocalCenter(), this);
    }

    void IOSNotificationManager::RegisterObserver(IOSNotificationObserver *observer, int mask)
    {
        std::lock_guard<std::mutex> lock(mObserverMutex);
        for (IOSNotificationObserver *item : mObserverList) {
            if (item == observer) {
                AF_LOGE("register notiobserver failed!");
                return;
            }
        }
        mObserverList.push_back(observer);
        AF_LOGD("register notiobserver success!");
    }

    void IOSNotificationManager::RemoveObserver(IOSNotificationObserver *observer)
    {
        std::lock_guard<std::mutex> lock(mObserverMutex);
        for (IOSNotificationObserver *item : mObserverList) {
            if (item == observer) {
                mObserverList.remove(observer);
                AF_LOGD("remove notiobserver success!");
                return;
            }
        }
        AF_LOGD("remove notiobserver failed!");
    }

    void IOSNotificationManager::HandleSystemNotification(int type)
    {
        std::lock_guard<std::mutex> lock(mObserverMutex);
        if (type == IOSResignActive) {
            mbActive = 0;
            for (IOSNotificationObserver *observer : mObserverList) {
                observer->AppWillResignActive();
            }
        } else if (type == IOSBecomeActive) {
            mbActive = 1;
            for (IOSNotificationObserver *observer : mObserverList) {
                observer->AppDidBecomeActive();
            }
        }
    }

    int IOSNotificationManager::GetActiveStatus() const
    {
        return mbActive;
    }
} // namespace Cicada
