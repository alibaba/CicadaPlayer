//
// Created by moqi on 2018/8/10.
//

#ifndef FRAMEWORK_VIDEO_DECODER_H
#define FRAMEWORK_VIDEO_DECODER_H

#include <vector>
#include <utils/AFMediaType.h>
#include <string>
#include <mutex>
#include <af_config.h>

enum decoder_status {
    got_pic,
    retry_input,
    new_format,
    decoder_eos,
    decoder_have_error,
    vt_session_invalid,
    decoder_background,
    decoder_create_fail,
};

typedef enum DECODER_FRAME_STATUS {
    DECODER_FRAME_INPUT_NORMAL,
    DECODER_FRAME_INPUT_EOF,
    DECODER_FRAME_INPUT_FLUSH,
    DECODER_FRAME_OUTPUT_EOF,
} DECODER_FRAME_STATUS;

#define STATUS_GOT_PIC    (1 << got_pic)
#define STATUS_RETRY_IN   1 << retry_input
#define STATUS_NEWFORMAT  1 << new_format
#define STATUS_EOS        1 << decoder_eos
#define STATUS_HAVE_ERROR 1 << decoder_have_error
#define STATUS_VT_SESSION_INVALID  1 << vt_session_invalid
#define STATUS_VT_DECODER_BACKGROUND  1 << decoder_background
#define STATUS_CREATE_FAIL 1 << decoder_create_fail

#include <base/media/IAFPacket.h>
#include "IVideoFrame.h"

namespace Cicada {

    class IDecoder {
    public:
        typedef struct decoder_error_info_t {
            int error;
            int64_t pts;
        } decoder_error_info;
        enum status {
            OK,
            EOS,
            RETRY = EAGAIN
        };

        IDecoder() = default;

        virtual ~IDecoder()
        {
            clean_error();
        }

        virtual int open(const Stream_meta *meta, void *voutObsr, uint64_t flags) = 0;

        virtual void flush() = 0;

        virtual void close() = 0;

        virtual void preClose() = 0;

        virtual void setEOF()
        {

        }

        /*
         * @param packet the packet send to decoder, use null to signals eos
         * @param timeOut the max time us to wait when packet is full in decoder
         *
         * @return
         *       -AGAIN time out was reached
         */

        virtual int send_packet(std::unique_ptr<IAFPacket> &packet, uint64_t timeOut)
        {
            return -ENOTSUP;
        };

        /*
         * @param[out] frame output the frame decoded
         * @param timeOut the max time us to wait when no frame in decoder
         * @return
         *        -AGAIN time out was reached
         */

        virtual int getFrame(std::unique_ptr<IAFFrame> &frame, uint64_t timeOut) = 0;

#ifdef ENABLE_HWDECODER
        virtual void dec_set_hw_device_content(CICADAHWDeviceType type, void *cont)
        {
            mHw_device_cont = cont;
            mHwDevicetype = type;
        }
#endif

        /*
         * return the number of error packet in decoding progress
         * you can use this to decide stop or continue the decoding
         * it will be reset to zero when flush or close
         */
        virtual int get_error_frame_no()
        {
#if AF_HAVE_PTHREAD
            std::lock_guard<std::mutex> lock(mVideoMutex);
#endif
            return static_cast<int>(mErrorQueue.size());
        }

        /*
         *  get the error info of one packet
         */
        virtual decoder_error_info get_error_frame_info(int index)
        {
#if AF_HAVE_PTHREAD
            std::lock_guard<std::mutex> lock(mVideoMutex);
#endif
            return mErrorQueue[index];
        }

        /*
         * clean the errors
         *
         */
        virtual void clean_error()
        {
#if AF_HAVE_PTHREAD
            std::lock_guard<std::mutex> lock(mVideoMutex);
#endif
            mErrorQueue.clear();
        }

        /*
         * get the decoder flags after open, not equal to the flags passed by open
         */

        virtual int getFlags()
        {
            return mFlags;
        }

        // return true if could recover
        virtual bool enterBackground(bool back)
        {
            mInBackground = back;
            return true;
        }

        std::string getName()
        {
            return mName;
        }

        virtual int holdOn(bool hold) = 0;

        virtual int getRecoverQueueSize() = 0;

    protected:
        std::string mName;
        int mFlags = 0; // VFLAG_HW,VFLAG_OUT
#if AF_HAVE_PTHREAD
        std::mutex mVideoMutex;
#endif
        std::vector<decoder_error_info> mErrorQueue;
        bool mInBackground = false;
        bool bNeedKeyFrame{true};
        int64_t keyPts = INT64_MIN;
    };
}


#endif //FRAMEWORK_VIDEO_DECODER_H
