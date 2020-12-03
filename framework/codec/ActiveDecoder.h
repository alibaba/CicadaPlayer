//
// Created by moqi on 2019-08-26.
//

#ifndef CICADA_PLAYER_ACTIVEDECODER_H
#define CICADA_PLAYER_ACTIVEDECODER_H

#include "af_config.h"

#if AF_HAVE_PTHREAD

#include <utils/afThread.h>

#endif

#include "IDecoder.h"
#include <atomic>
#include <base/media/spsc_queue.h>
#include <queue>
#include <map>
#include <drm/DrmInfo.h>

class CICADA_CPLUS_EXTERN ActiveDecoder : public Cicada::IDecoder {

public:
    ActiveDecoder();

    virtual ~ActiveDecoder() override;

    int open(const Stream_meta *meta, void *voutObsr, uint64_t flags, const Cicada::DrmInfo *drmInfo) override;

    void close() override;

    int send_packet(std::unique_ptr<IAFPacket> &packet, uint64_t timeOut) override;

    int getFrame(std::unique_ptr<IAFFrame> &frame, uint64_t timeOut) override;


    void pause(bool pause) override;

    void flush() override;

    void prePause() override;

    int holdOn(bool hold) override;

    int getRecoverQueueSize() override;

    uint32_t getInputPaddingSize() override;

private:

    virtual int enqueue_decoder(std::unique_ptr<IAFPacket> &pPacket) = 0;

    virtual int dequeue_decoder(std::unique_ptr<IAFFrame> &pFrame) = 0;

    virtual int init_decoder(const Stream_meta *meta, void *wnd, uint64_t flags, const Cicada::DrmInfo *drmInfo) = 0;

    virtual void close_decoder() = 0;

    virtual void flush_decoder() = 0;

    virtual int get_decoder_recover_size() = 0;


private:
    bool needDrop(IAFPacket *packet);

protected:

    void enqueueError(int ret, int64_t pts);

#if AF_HAVE_PTHREAD

    int decode_func();

    int extract_decoder();

    int thread_send_packet(std::unique_ptr<IAFPacket> &packet);

    int thread_getFrame(std::unique_ptr<IAFFrame> &frame);

#endif
protected:
#if AF_HAVE_PTHREAD
    afThread *mDecodeThread = nullptr;
    std::atomic_bool mRunning{false};
#endif
private:

    std::atomic_bool bInputEOS{false};
    bool bSendEOS2Decoder{};
    std::atomic_bool bDecoderEOS{false};
#if AF_HAVE_PTHREAD
    std::condition_variable mSleepCondition{};
    Cicada::SpscQueue<IAFPacket *> mInputQueue;
    Cicada::SpscQueue<IAFFrame *> mOutputQueue;
    int maxOutQueueSize = 10;
    int maxInQueueSize = 16;
    std::mutex mMutex{};
    std::mutex mSleepMutex{};
#endif
    std::atomic_bool bHolding{false};
    std::queue<std::unique_ptr<IAFPacket>> mHoldingQueue{};
    enum AFCodecID mCodecId{AF_CODEC_ID_NONE};
};


#endif //CICADA_PLAYER_ACTIVEDECODER_H
