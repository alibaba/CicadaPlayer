#ifndef VIDEO_FRAME_IOS_H
#define VIDEO_FRAME_IOS_H

#include <list>
#include <mutex>

namespace Cicada{
    extern float GetIosVersion();

    extern void PixelBuffer_Retain(void *pixelBuffer);

#if 0
    extern void* PixelBuffer_Create(int width, int height, ImageFormat format);

    extern ImageFormat  PixelBuffer_GetPixelFormat(void* pixelBuffer);

    extern ColorRange PixelBuffer_GetColorRange(void *pixelBuffer);
#endif

    extern void PixelBuffer_Release(void *pixelBuffer);

    extern int PixelBuffer_GetWidth(void *pixelBuffer);

    extern int PixelBuffer_GetHeight(void *pixelBuffer);

    extern int PixelBuffer_GetStride(void *pixelBuffer);


    enum IOSNotificationMask {
        IOSResignActive = 1 << 0,
        IOSBecomeActive = 1 << 1,
        IOSAudioInterrputed = 1 << 2,
    };

    class IOSNotificationObserver {
    public:
        virtual void AppWillResignActive()
        {};

        virtual void AppDidBecomeActive()
        {};

        virtual void AppAudioInterruptedStart()
        {};

        virtual void AppAudioInterruptedEnd()
        {};

    protected:
        ~IOSNotificationObserver() = default;
    };

    void RegisterIOSNotificationObserver(IOSNotificationObserver *observer, int mask);

    void RemoveIOSNotificationObserver(IOSNotificationObserver *observer);

    class IOSNotificationManager {
    public:
        static IOSNotificationManager *Instance();

        IOSNotificationManager();

        ~IOSNotificationManager();

        void RegisterObserver(IOSNotificationObserver *observer, int mask);

        void RemoveObserver(IOSNotificationObserver *observer);

        void HandleSystemNotification(int type);

        int GetActiveStatus() const;

    private:
        std::list<IOSNotificationObserver *> mObserverList{};
        std::mutex mObserverMutex;

        volatile int mbActive{-1};  // -1 not certain, 0 background, 1 true
    };

} // namespace Cicada

#endif // VIDEO_FRAME_IOS_H
