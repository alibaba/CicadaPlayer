//
// Created by lifujun on 2019/7/23.
//

#ifndef FRAMEWORK_IVideoRender_H
#define FRAMEWORK_IVideoRender_H


#include <base/media/IAFPacket.h>
#include <functional>
#include <utils/CicadaJSON.h>

typedef bool (*videoRenderingFrameCB)(void *userData, IAFFrame *frame, const CicadaJSONItem &params);


class IVideoRender {

public:
    static const uint64_t FLAG_HDR = (1 << 0);
    static const uint64_t FLAG_DUMMY = (1 << 1);

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
        virtual void onFrameInfoUpdate(IAFFrame::AFFrameInfo &info, bool rendered) = 0;
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
    virtual void setBackgroundColor(uint32_t color) = 0;

    /**
     * set want draw frame.
     * @param frame
     */
    virtual int renderFrame(std::unique_ptr<IAFFrame> &frame) = 0;

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
    virtual uint64_t getFlags() = 0;

    virtual void setVideoRenderingCb(videoRenderingFrameCB cb, void *userData)
    {
        mRenderingCb = cb;
        mRenderingCbUserData = userData;
    }

    class videoProcessTextureCb {
    public:
        videoProcessTextureCb() = default;

        virtual ~videoProcessTextureCb() = default;

        /**
         * @param type      TEXTURE_YUV 0, TEXTURE_RGBA 1
         * @return
         */
        virtual bool init(int type) = 0;

        virtual bool needProcess() = 0;

        virtual bool processTexture(std::unique_ptr<IAFFrame> &textureFrame) = 0;
    };

    virtual void setVideoProcessTextureCb(videoProcessTextureCb *cb)
    {
        mProcessTextureCb = cb;
    }

protected:
    bool mInvalid{false};
    IVideoRenderListener *mListener{nullptr};

    videoRenderingFrameCB mRenderingCb{nullptr};
    void *mRenderingCbUserData{nullptr};

    videoProcessTextureCb *mProcessTextureCb{nullptr};
};


#endif //FRAMEWORK_IVideoRender_H
