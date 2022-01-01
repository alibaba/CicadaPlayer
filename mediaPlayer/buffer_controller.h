#ifndef CICADA_BUFFER_CONTROLLER_H
#define CICADA_BUFFER_CONTROLLER_H

#include <deque>
#include <mutex>

#include "media_packet_queue.h"

namespace Cicada {

    typedef enum BUFFER_TYPE {
        BUFFER_TYPE_VIDEO = 1,
        BUFFER_TYPE_AUDIO = (1 << 1),
        BUFFER_TYPE_SUBTITLE = (1 << 2),

        BUFFER_TYPE_AV = (BUFFER_TYPE_VIDEO | BUFFER_TYPE_AUDIO),
        BUFFER_TYPE_ALL = (BUFFER_TYPE_VIDEO | BUFFER_TYPE_AUDIO
                           | BUFFER_TYPE_SUBTITLE),
    } BUFFER_TYPE;

    class BufferController {
    public:
        BufferController();

        ~BufferController();

    public:
        int64_t GetPacketDuration(BUFFER_TYPE type);

        void SetOnePacketDuration(BUFFER_TYPE type, int64_t duration);

        int64_t GetOnePacketDuration(BUFFER_TYPE type);

        int GetPacketSize(BUFFER_TYPE type);

        bool IsPacketEmtpy(BUFFER_TYPE type);

        std::unique_ptr<IAFPacket> getPacket(BUFFER_TYPE type);

        void AddPacket(std::unique_ptr<IAFPacket> packet, BUFFER_TYPE type);

        void ClearPacket(BUFFER_TYPE type);

        int64_t GetPacketPts(BUFFER_TYPE type);

        int64_t GetPacketLastKeyTimePos(BUFFER_TYPE type);

        int64_t ClearPacketBeforeTimePos(BUFFER_TYPE type, int64_t pts);

        int64_t ClearPacketBeforePts(BUFFER_TYPE type, int64_t pts);

        int64_t GetKeyTimePositionBefore(BUFFER_TYPE type, int64_t pts);

        int64_t GetKeyPTSBefore(BUFFER_TYPE type, int64_t pts);

        int64_t GetPacketLastTimePos(BUFFER_TYPE type);

        int64_t GetPacketLastPTS(BUFFER_TYPE type);

        int64_t FindSeamlessPointTimePosition(BUFFER_TYPE type, int &count);

        void ClearPacketAfterTimePosition(BUFFER_TYPE type, int64_t pts);

//       std::deque<std::shared_ptr<IAFPacket>> CopyVideoCacheQueue();

    private:
        MediaPacketQueue mVideoPacketQueue;
        MediaPacketQueue mAudioPacketQueue;
        MediaPacketQueue mSubtitlePacketQueue;
    };

} // namespace Cicada
#endif // CICADA_BUFFER_CONTROL_H
