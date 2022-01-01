//
// Created by moqi on 2018/4/27.
//

#ifndef FRAMEWORK_HLSMANAGER_H
#define FRAMEWORK_HLSMANAGER_H

#include "PlaylistManager.h"
#include "HLSStream.h"
#include <queue>

namespace Cicada{
    class HLSManager : public PlaylistManager {

        class HLSStreamInfo {
        public:
            HLSStream *mPStream = nullptr;
            std::unique_ptr<IAFPacket> mPFrame{};
            bool selected = false;
            bool stopOnSegEnd = false;
            int toStreamId = -1;
            bool eos = false;
        };

    public:
        explicit HLSManager(playList *pList);

        ~HLSManager() override;

        int init() override;

        void stop() override;

        int GetNbStreams() const override;

        int GetStreamMeta(Stream_meta *meta, int index, bool sub) const override;

        int ReadPacket(std::unique_ptr<IAFPacket> &packet, int index) override;

        int OpenStream(int id) override;

        void CloseStream(int id) override;

        int start() override;

        int64_t seek(int64_t us, int flags, int index) override;

        int SwitchStreamAligned(int from, int to) override;

        int getNBSubStream(int index) const override;

        void interrupt(int inter) override;

        const std::string GetProperty(int index, const string &key) override;

        int GetRemainSegmentCount(int index) override;
        
        bool isRealTimeStream(int index) override;

        int64_t getTargetDuration() override;

    private:
        std::list<HLSStreamInfo*> mStreamInfoList{};
        HLSStream *mMuxedStream = nullptr;
        bool mStarted = false;
        int64_t mFirstSeekPos = INT64_MIN;
    };
}


#endif //FRAMEWORK_HLSMANAGER_H
