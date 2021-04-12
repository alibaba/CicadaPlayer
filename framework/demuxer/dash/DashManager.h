//
// Created by yuyuan on 2021/03/17.
//

#ifndef DEMUXER_DASH_DASH_MANAGER_H
#define DEMUXER_DASH_DASH_MANAGER_H

#include "demuxer/play_list/PlaylistManager.h"

namespace Cicada {

    class DashStream;
    class AdaptationSet;
    class Period;

    class DashManager : public PlaylistManager {

        class DashStreamInfo {
        public:
            DashStream *mPStream = nullptr;
            std::unique_ptr<IAFPacket> mPFrame{};
            bool selected = false;
            bool stopOnSegEnd = false;
            int toStreamId = -1;
            bool eos = false;
        };

    public:
        explicit DashManager(playList *pList);

        ~DashManager() override;

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
        bool isWallclockTimeSyncStream(int index) override;

        int64_t getTargetDuration() override;

    private:
        std::list<AdaptationSet *> FindSuitableAdaptationSets(Period* period);
        std::list<DashStreamInfo *> mStreamInfoList{};
        DashStream *mMuxedStream = nullptr;
        bool mStarted = false;
        int64_t mFirstSeekPos = INT64_MIN;
    };
}// namespace Cicada


#endif//DEMUXER_DASH_DASH_MANAGER_H
