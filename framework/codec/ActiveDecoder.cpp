//
// Created by moqi on 2019-08-26.
//
#define LOG_TAG "AFActiveDecoder"

#include "ActiveDecoder.h"
#include "utils/timer.h"

//TODO: can set
#define  MAX_INPUT_SIZE 16

using namespace std;

ActiveDecoder::ActiveDecoder()
{
    mFlags = 0;
}

int ActiveDecoder::open(const Stream_meta *meta, void *voutObsr, uint64_t flags)
{
    int ret = init_decoder(meta, voutObsr, flags);

    if (ret < 0) {
        close();
        return ret;
    }

    mRunning = true;
    mCodecId = meta->codec;
#if AF_HAVE_PTHREAD
    auto func = [this]() -> int { return this->decode_func(); };
    mDecodeThread = new afThread(func, LOG_TAG);
    mDecodeThread->start();
#endif
    return 0;
}

void ActiveDecoder::close()
{
#if AF_HAVE_PTHREAD
    {
        std::unique_lock<std::mutex> locker(mSleepMutex);
        mRunning = false;
    }
    mSleepCondition.notify_one();

    if (mDecodeThread) {
        mDecodeThread->pause();
    }

#else
    mRunning = false;
#endif
    close_decoder();
#if AF_HAVE_PTHREAD

    while (!mInputQueue.empty()) {
        mInputQueue.pop();
    }

    while (!mOutputQueue.empty()) {
        mOutputQueue.pop();
    }

#endif
}

int ActiveDecoder::decode_func()
{
    if (bDecoderEOS) {
        af_usleep(10000);
        return 0;
    }

    int needWait = 0;
    int ret;
    int64_t pts = INT64_MIN;

    if (!mPacket) {
        std::unique_lock<std::mutex> uMutex(mMutex);

        if (!mInputQueue.empty()) {
            mPacket = move(mInputQueue.front());
            mInputQueue.pop();
        }
    }

    if (mPacket) {
        pts = mPacket->getInfo().pts;
        ret = enqueue_decoder(mPacket);

        if (ret == -EAGAIN) {
            needWait++;
        } else {
            //  assert(mPacket == nullptr);
            mPacket = nullptr;

            if (ret == STATUS_EOS) {
                bDecoderEOS = true;
            } else if (ret < 0) {
                AF_LOGW("enqueue_decoder error %d\n", ret);
                enqueueError(ret, pts);
            }
        }
    } else if (bInputEOS) {
        // TODO: flush once only
        if (!bSendEOS2Decoder) {
            unique_ptr<IAFPacket> pPacket{};
            ret = enqueue_decoder(pPacket);

            if (ret != -EAGAIN) {
                bSendEOS2Decoder = true;
            }

            if (ret == STATUS_EOS) {
                bDecoderEOS = true;
            }
        }
    } else {
        needWait++;
    }

    ret = extract_decoder();

    if (ret == 0) {
        needWait++;
    } else if (ret < 0) {
        AF_LOGW("extract_decoder error %d\n", ret);
        enqueueError(ret, pts);
    }

    if (needWait > 0) {
        std::unique_lock<std::mutex> locker(mSleepMutex);
        mSleepCondition.wait_for(locker, std::chrono::milliseconds(5 * needWait), [this]() { return !mRunning; });
    }

    return 0;
}

bool ActiveDecoder::needDrop(IAFPacket *packet)
{
    if (packet == nullptr) {
        return false;
    }

    if (packet->getInfo().flags & AF_PKT_FLAG_CORRUPT) {
        return true;
    }

    if (bNeedKeyFrame) { // need a key frame, when first start or after seek
        if ((packet->getInfo().flags & AF_PKT_FLAG_KEY) == 0) { // drop the frame that not a key frame
            // TODO: return error?
            AF_LOGW("wait a key frame\n");
            return true;
        } else {  // get the key frame
            bNeedKeyFrame = false;
            keyPts = packet->getInfo().pts;
            return false;
        }
    } else if (packet->getInfo().flags) {
        keyPts = INT64_MIN; // get the next key frame, stop to check
    }

    // TODO: make sure HEVC not need drop also
    if (mCodecId != AF_CODEC_ID_HEVC) {
        return false;
    }

    if (packet->getInfo().pts < keyPts) { // after get the key frame, check the wrong frame use pts
        AF_LOGW("key pts is %lld,pts is %lld\n", keyPts, packet->getInfo().pts);
        AF_LOGW("drop a error frame\n");
        return true;
    }

    return false;
}

int ActiveDecoder::send_packet(unique_ptr<IAFPacket> &packet, uint64_t timeOut)
{
    if (needDrop(packet.get())) {
        packet = nullptr;
        return 0;
    }

#if AF_HAVE_PTHREAD
    return thread_send_packet(packet);
#else
    int status = 0;
    int ret = enqueue_decoder(packet.get());

    if (ret == AVERROR(EAGAIN)) {
        status = STATUS_RETRY_IN;
    } else {
        if (ret < 0) {
            AF_LOGW("enqueue_decoder error %d\n", ret);
            enqueueError(ret, packet->getInfo().pts);
        }
    }

    return status;
#endif
}

int ActiveDecoder::thread_send_packet(unique_ptr<IAFPacket> &packet)
{
    if (bInputEOS) {
        return -EINVAL;
    }

    if (packet == nullptr) {
        bInputEOS = true;
        mSleepCondition.notify_one();
        return 0;
    }

    int status = 0;
    unique_lock<mutex> uMutex(mMutex);

    if (bHolding) {
        if (packet->getInfo().flags) {
            while (!mHoldingQueue.empty()) {
                mHoldingQueue.pop();
            }

            flush_decoder();
        }

        mHoldingQueue.push(move(packet));
        return 0;
    }

    if ((mInputQueue.size() >= MAX_INPUT_SIZE)
            || (mOutputQueue.size() >= maxOutQueueSize)) {
        // TODO: wait for timeOut us
        status |= STATUS_RETRY_IN;
    } else {
        mInputQueue.push(move(packet));
        mSleepCondition.notify_one();
    }

    // TODO: don't free the packet when queue is full;
    if (!mErrorQueue.empty()) {
        status |= STATUS_HAVE_ERROR;
    }

    return status;
}

void ActiveDecoder::enqueueError(int ret, int64_t pts)
{
    decoder_error_info info = {ret, pts};
    std::lock_guard<std::mutex> lock(mVideoMutex);
    mErrorQueue.push_back(info);
}

int ActiveDecoder::getFrame(unique_ptr<IAFFrame> &frame, uint64_t timeOut)
{
#if AF_HAVE_PTHREAD
    return thread_getFrame(frame);
#else
    int ret;
    ret = dequeue_decoder();

    if (ret < 0) {
        if (ret == AVERROR(EAGAIN)) {
            return ret;
        }

        if (ret == AVERROR_EOF) {
            AF_LOGD("decoder out put eof\n");
            return STATUS_EOS;
        }

        return ret;
    } else {
        frame = unique_ptr<AVAFFrame>(new AVAFFrame(mPDecoder->avFrame));
        return 0;
    }

#endif
}

int ActiveDecoder::thread_getFrame(unique_ptr<IAFFrame> &frame)
{
    frame = nullptr;
    // TODO: wait for timeOut us
    unique_lock<mutex> uMutex(mMutex);

    if (!mOutputQueue.empty()) {
        frame = move(mOutputQueue.front());
        mOutputQueue.pop();
        return 0;
    } else if (bDecoderEOS) {
        return STATUS_EOS;
    }

    return -EAGAIN;
}

ActiveDecoder::~ActiveDecoder()
{
#if AF_HAVE_PTHREAD
    mSleepCondition.notify_one();
    delete mDecodeThread;
#endif
}

void ActiveDecoder::flush()
{
#if AF_HAVE_PTHREAD
    bool running = mDecodeThread->getStatus() == afThread::THREAD_STATUS_RUNNING;
    mDecodeThread->pause();

    while (!mInputQueue.empty()) {
        mInputQueue.pop();
    }

    while (!mOutputQueue.empty()) {
        mOutputQueue.pop();
    }

    while (!mHoldingQueue.empty()) {
        mHoldingQueue.pop();
    }

    mPacket = nullptr;
#endif
    clean_error();
    flush_decoder();
    bInputEOS = false;
    bDecoderEOS = false;
    bSendEOS2Decoder = false;
#if AF_HAVE_PTHREAD

    if (running) {
        mDecodeThread->start();
    }

#endif
    mRunning = true;
    bNeedKeyFrame = true;
}

void ActiveDecoder::preClose()
{
#if AF_HAVE_PTHREAD
    {
        std::unique_lock<std::mutex> locker(mSleepMutex);
        mRunning = false;
    }
    mSleepCondition.notify_one();

    if (mDecodeThread) {
        mDecodeThread->prePause();
    }

#endif
}

#if AF_HAVE_PTHREAD

int ActiveDecoder::extract_decoder()
{
    int count = 0;
    int size;
    {
        std::unique_lock<std::mutex> uMutex(mMutex);
        size = mOutputQueue.size();
    }

    if (size < maxOutQueueSize) {
        int ret = 0;

        do {
            unique_ptr<IAFFrame> pFrame{};
            ret = dequeue_decoder(pFrame);

            if (ret < 0 || ret == STATUS_EOS) {
                if (ret == STATUS_EOS) {
                    AF_LOGD("decoder out put eof\n");
                    bDecoderEOS = true;
                } else if (ret != -EAGAIN) {
                    AF_LOGE("decoder error %d\n", ret);
                }

                ret = 0;
                return ret;
            }

            if (pFrame) {
                std::unique_lock<std::mutex> uMutex(mMutex);
                mOutputQueue.push(move(pFrame));
                count++;
            }
        } while (ret >= 0 && mRunning);
    }

    return count;
}

int ActiveDecoder::holdOn(bool hold)
{
    if (bHolding == hold) {
        return 0;
    }

    unique_lock<mutex> uMutex(mMutex);

    if (hold) {
        while (!mInputQueue.empty()) {
            mInputQueue.front()->setDiscard(true);
            mHoldingQueue.push(move(mInputQueue.front()));
            mInputQueue.pop();
        }

        while (!mOutputQueue.empty()) {
            mOutputQueue.pop();
        }
    } else {
        AF_LOGD("mHoldingQueue size is %d\n", mHoldingQueue.size());
        int64_t pts = 0;

        while (!mHoldingQueue.empty()) {
            mHoldingQueue.front()->setDiscard(true);

            if (pts < mHoldingQueue.front()->getInfo().pts) {
                pts = mHoldingQueue.front()->getInfo().pts;
            }

            mInputQueue.push(move(mHoldingQueue.front()));
            mHoldingQueue.pop();
        }

        AF_LOGD("mHoldingQueue pts  is %d\n", pts);
    }

    bHolding = hold;
    return 0;
}

int ActiveDecoder::getRecoverQueueSize()
{
    unique_lock<mutex> uMutex(mMutex);
    return int(mHoldingQueue.size() + get_decoder_recover_size());
}

#endif

