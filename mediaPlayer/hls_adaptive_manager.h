#ifndef CICADA_HLS_MANAGE_H
#define CICADA_HLS_MANAGE_H

#include <map>
#include <algorithm>

using namespace std;

#include "native_cicada_player_def.h"
#include "buffer_controller.h"

typedef void (*streamAdaptChanged)(int stream, void *userData);

namespace Cicada {
    class AdaptiveBitrateManager {
    public:

    public:
        AdaptiveBitrateManager();

        ~AdaptiveBitrateManager();

    public:
        int64_t getInterfaceBytes();

        void OnSecondTimer(int64_t curTime);

        void clear();

        void AddStreamInfo(int streamIndex, int bitrate);

        void SetBufferControlPtr(BufferController *bufferService);

        void SetCurrentBitrate(int bitrate);

        void SetCallback(streamAdaptChanged callback, void *userData);

        void reset();

        void setCanSwitch(bool canSwitch);

    private:
        void ProcessSwitchStream(int64_t curTime);

    private:
        int mMaxDownloadSpeed{0};
        int64_t mLastDownloadBytes{0};
        int mUpdateSpan{10 * 1000};
        int mSpanDownloadSpeed{0};
        int mLastSpanDownloadSpeed{0};
        int64_t mStartTime{INT64_MIN};
        int64_t mLastSwitchTime{INT64_MIN};
        int mCurrentBitrate{0};
        int64_t mLastVideoDuration{INT64_MIN};

        map<int, int> mStreamIndexBitrateMap;
        BufferController *mBufferService;
        vector<int> mBitrates;
        streamAdaptChanged mCallback{nullptr};
        void *mUserData{nullptr};
        int mDurationStatics[10];
        int mDurationCount{0};
        bool mCanSwitch{true};
    };

} // namespace Cicada
#endif // CICADA_HLS_MANAGE_H
