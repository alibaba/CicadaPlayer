//
// Created by moqi on 2019-07-19.
//

#ifndef FRAMEWORK_WABITSTREAMDECODER_H
#define FRAMEWORK_WABITSTREAMDECODER_H

#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

typedef enum frameType_t {
    frameTypeVideo,
    frameTypeAudio,
} frameType;
/**
 * BSDecoder data read call back type
 * @param arg    user arg, not be used for now, set it to null
 * @param buffer the buffer to hold data
 * @param size   the buffer size max read
 *
 * @return
 *      0        eof
 *      -EAGAIN  retry later (-11 on emscripten platform)
 *      > 0      size be filled
 *      other < 0 error (tobe impl)
 */
typedef int (*data_callback_read)(void *arg, uint8_t *buffer, int size);
/**
 * BSDecoder eos call back, notify the demuxer and the decoders are all eos,
 * you should stop the decoder now.
 *
 */

typedef void (*eos_call_back)();

/**
 * BSDecoder will call it to get a buffer to hold the raw data(yuv or pcm)
 * @param    size   the size required.
 * @return   the buffer addr, null if no memory.
 */

typedef uint8_t *(*getBuffer_call_back)(int size);

/**
 * BSDecoder will call it when filled a media raw data frame
 * @param buffer  the frame's buffer addr, the buffer will be freed on return,
 *                 if the buffer was not got from getBuffer_call_back, otherwise you
 *                 must free it.
 * @param size    the buffer size
 *
 * @param pts     the presentation time stamp of the frame in ms
 *
 * @param type    mark the type of the frame video/audio
 *
 * @param consumeTimeUs the time in us consume by decoder to decode the frame
 *
 */

typedef void (*frame_call_back)(uint8_t *buffer, int size, int pts, frameType type, int consumeTimeUs);

/**
 * BSDecoder use this to tell out the media meta data.
 * this will be called before any frame call back,
 * and be called when the format was changed.
 * there is no this type stream if not be called before frame call back.
 *
 * @param duration in ms
 *
 */
// TODO: mark the format of the frame,use yv12 for now

typedef void (*video_info_call_back)(int width, int height, int duration);

//TODO: mark the pcm format, use float32 for now

typedef void (*audio_info_call_back)(int channels, int sample_rate, int duration);


/**
 * BSDecoder poll the size of data have ready, BSDecoder use the size decide whether to
 * read data use demuxer.
 * @param
 *        arg     user data
 *
 * @return
 *        0        the file was finished (eof)
 *        -EAGAIN  no any data (-11 on emscripten platform)
 *        > 0      the size have buffered
 */

typedef int (*poll_data)(void *arg);

/**
 * BSDecoder poll if there are enough raw buffer(pcm or yuv) to fill decoder out put,
 * if no buffer, the decoder will not to decode for this loop
 *
 * @param
 *        arg     user data
 *        type    not used for now
 * @return
 *        0       buffer is enough
 *        -EAGAIN no buffer
 */

typedef int (*poll_raw_buffer)(void *arg, int type);


/**
 * BSDecoder error
 */


typedef void (*error_call_back)(int error);

#ifdef __EMSCRIPTEN__
/*
 * data_callback_read
 */

void BSDecoderSetReadCallBack(long func);

/*
 * eos_call_back
 */

void BSDecoderEOSCallBack(long func);

/*
 * getBuffer_call_back
 */

void BSDecoderSetGetBuffer(long func);

/*
 * frame_call_back
 */

void BSDecoderSetFrameCallBack(long func);

/*
 * video_info_call_back
 */
void BSDecoderSetInfoCallBackVideo(long func);

/*
 * audio_info_call_back
 */
void BSDecoderSetInfoCallBackAudio(long func);

/*
 * poll_data
 */
void BSDecoderSetPollDataCallBack(long func);


void BSDecoderSetErrorCallBack(long func);


void BSDecoderSetPollRawBufferCallback(long func);

int test();
#else

void BSDecoderSetReadCallBack(data_callback_read func);

void BSDecoderEOSCallBack(eos_call_back func);

void BSDecoderSetGetBuffer(getBuffer_call_back func);

void BSDecoderSetFrameCallBack(frame_call_back func);

void BSDecoderSetInfoCallBackVideo(video_info_call_back func);

void BSDecoderSetInfoCallBackAudio(audio_info_call_back func);

void BSDecoderSetPollDataCallBack(poll_data func);


#endif

/**
 * init the decoder
 * @return
 *        0 success
 *        < 0 error
 *  you must set all the call back before call it.
 */

int BSDecoderPrepare();


/**
 * start the main loop thread.
 */

void BSDecoderStart();

/**
 * flush the decoder.
 * call it when seek
 */

void BSDecoderFlush();

/**
 * stop the main loop thread
 */

void BSDecoderStop();
#ifdef __cplusplus
};
#endif

#endif //FRAMEWORK_WABITSTREAMDECODER_H
