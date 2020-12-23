//
// Created by lifujun on 2019/7/23.
//

#ifndef FRAMEWORK_IVideoRender_H
#define FRAMEWORK_IVideoRender_H


#include <base/media/IAFPacket.h>
#include <functional>

class IVideoRender {

public:

    enum Rotate {
        Rotate_None = 0,
        Rotate_90 = 90,
        Rotate_180 = 180,
        Rotate_270 = 270
    };

    enum Flip {
        Flip_None,
        Flip_Horizontal,
        Flip_Vertical,
        Flip_Both,
    };

    enum Scale {
        Scale_AspectFit,
        Scale_AspectFill,
        Scale_Fill
    };

    static Rotate getRotate(int value)
    {
        switch (value) {
            case 90:
                return Rotate_90;
            case 180:
                return Rotate_180;
            case 270:
                return Rotate_270;
            default:
                return Rotate_None;
        }
    };

    class ScreenShotInfo {
    public:
        enum Format {
            UNKNOWN, RGB888,
        };
    public:
        Format format = UNKNOWN;

        int width = 0;
        int height = 0;

        int64_t bufLen = 0;
        char *buf = nullptr;

        ~ScreenShotInfo()
        {
            if (buf != nullptr) {
                free(buf);
                buf = nullptr;
            }
        }
    };

    class IVideoRenderFilter {
    public:
        virtual ~IVideoRenderFilter() = default;

        // a fbo or a frame
        virtual int push(void *) = 0;

        virtual int pull(void *) = 0;

    };

    class IVideoRenderListener {
    public:
        virtual void onFrameInfoUpdate(IAFFrame::AFFrameInfo &info) = 0;
        virtual ~IVideoRenderListener() = default;
    };

public:
    virtual ~IVideoRender() = default;

    /**
     * init render
     * @return
     */
    virtual int init() = 0;

    /**
     * clear screen to black.
     */
    virtual int clearScreen() = 0;

    /*
     * set background color
     */
    virtual void setBackgroundColor(uint32_t color) {

    };

    /**
     * set want draw frame.
     * @param frame
     */
    virtual int renderFrame(std::unique_ptr<IAFFrame> &frame) = 0;

    /**
     * NOTE: will callback in render thread.
     * @param renderedCallback
     */
    virtual void setRenderResultCallback(std::function<void(int64_t, bool)> renderedCallback)
    {
        mRenderResultCallback = renderedCallback;
    }

    virtual void setListener(IVideoRenderListener *listener)
    {
        mListener = listener;
    }

    /**
     * set render rotate.
     * @param rotate
     */
    virtual int setRotate(Rotate rotate) = 0;

    /**
     * set render flip.
     * @param flip
     */
    virtual int setFlip(Flip flip) = 0;

    /**
     * set render scale.
     * @param scale
     */
    virtual int setScale(Scale scale) = 0;

    /**
     * set the playback speed, rend use it to improve render smooth
     * @param speed
     */
    virtual void setSpeed(float speed) = 0;

    /**
     * set window size when window size changed.
     * @param windWith
     * @param windHeight
     */
    virtual void setWindowSize(int windWith, int windHeight) {

    }

    virtual void surfaceChanged() {

    }

    virtual void enterForeground(){

    }

    virtual void enterBackground(){

    }

    virtual void setFilter(IVideoRenderFilter *filter)
    {
        mFilter = filter;
    }

    /**
     * set display view
     * @param view
     */
    virtual int setDisPlay(void *view)
    {
        return 0;
    }


    virtual void captureScreen(std::function<void(uint8_t *, int, int)> func)
    {
        func(nullptr,0,0);
    }


    virtual void *getSurface(bool cached)
    {
        return nullptr;
    }

    virtual float getRenderFPS() = 0;

    virtual void invalid(bool invalid)
    {
        mInvalid = invalid;
    }


protected:
    IVideoRenderFilter *mFilter{};
    bool mInvalid{false};
    std::function<void(int64_t, bool)> mRenderResultCallback = nullptr;
    IVideoRenderListener *mListener{nullptr};
};


#endif //FRAMEWORK_IVideoRender_H
