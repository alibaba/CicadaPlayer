//
// Created by moqi on 2018/4/25.
//

#ifndef FRAMEWORK_PLAYLIST_DEMUXER_H
#define FRAMEWORK_PLAYLIST_DEMUXER_H

#include <string>
#include "data_source/proxyDataSource.h"

using namespace std;

#include "demuxer/IDemuxer.h"
#include "demuxer/demuxerPrototype.h"
#include "playListParser.h"
#include "PlaylistManager.h"
#include "HlsParser.h"

namespace Cicada{

    class playList_demuxer : public IDemuxer, private demuxerPrototype {

    public:
        enum playList_type {
            playList_type_unknown = 0,
            playList_type_hls,

        };

        explicit playList_demuxer();

        playList_demuxer(const string& path, playList_type type);

        ~playList_demuxer() override;

        const playList *GetPlayList() override
        {
            return mPPlayList;
        }

        int Open() override;

        int ReadPacket(std::unique_ptr<IAFPacket> &packet, int index) override;

        void Close() override;

        void Start() override;

        void Stop() override;

        int64_t Seek(int64_t us, int flags, int index) override;

        int GetNbStreams() const override;

        int GetNbSubStreams(int index) const override;

        int GetRemainSegmentCount(int index) override;
        
        int GetSourceMeta(Source_meta **meta) const override;

        int GetStreamMeta(Stream_meta *meta, int index, bool sub) const override;

        int OpenStream(int index) override;

        void CloseStream(int index) override;

        void interrupt(int inter) override;

        int SwitchStreamAligned(int from, int to) override;

        int64_t getMaxGopTimeUs() override;

        void flush() override
        {
            // TODO:
        };

        bool isPlayList() const override
        {
            return true;
        }

        const std::string GetProperty(int index, const string &key) const override;
        
        bool isRealTimeStream(int index) override;

    private:
        explicit playList_demuxer(int dummy) : IDemuxer("")
        {
            addPrototype(this);
        }

        Cicada::IDemuxer *clone(const string &uri, int type, const Cicada::DemuxerMeta *meta) override
        {
            return new playList_demuxer(uri, static_cast<playList_type>(type));
        }

        bool is_supported(const string &uri, const uint8_t *buffer, int64_t size, int *type, const Cicada::DemuxerMeta *meta,
                          const Cicada::options *opts) override
        {
            // TODO: check the description
            int ret = HlsParser::probe(buffer, size);
            *type = playList_type_hls;
            if (ret > 0)
                return true;
            return false;
        }

        int getType() override
        {
            return demuxer_type_playlist;
        }

        static playList_demuxer se;

    private:

        playList *mPPlayList = nullptr;
        playListParser *mParser = nullptr;
        playList_type mType{playList_type_unknown};
        PlaylistManager *mPPlaylistManager = nullptr;
        proxyDataSource *mProxySource = nullptr;

        int64_t mFirstSeekPos = INT64_MIN;
    };
}


#endif //FRAMEWORK_PLAYLIST_DEMUXER_H
