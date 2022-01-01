//
// Created by moqi on 2018/4/25.
//

#ifndef FRAMEWORK_REPRESENTATION_H
#define FRAMEWORK_REPRESENTATION_H

#include <list>
#include "segment.h"
#include "SegmentList.h"
#include "playList.h"
#include "AdaptationSet.h"
#include <utils/AFMediaType.h>
#include <ctime>
//#include "../../utils/frame_work_log.h"
//#include "playList_demuxer.h"

//using namespace std;
namespace Cicada{
    class AdaptationSet;

    class SegmentList;

    class playList;

    class Representation {
    public:
        Representation(AdaptationSet *adapt)
        {
            mAdapt = adapt;
        }

        ~Representation();

        void SetSegmentList(SegmentList *pSegList);

        SegmentList *GetSegmentList();

        void setPlaylistUrl(const std::string &);

        const std::string &getPlaylistUrl();

        void setBaseUrl(const std::string &url);

        const std::string &getBaseUrl();

        void setBandwidth(uint64_t bandwidth);

        void setWidth(int width);

        void setHeight(int height);

        void print();

        int getStreamInfo(int *width, int *height, uint64_t *bandwidth, std::string &language);

        playList *getPlaylist();

        AdaptationSet *getAdaptationSet();


    public:
        // TODO use set and get
        time_t targetDuration = 0;
        time_t partTargetDuration = 0;
        bool b_live = false;
        int mPlayListType{0};
        Stream_type mStreamType = STREAM_TYPE_MIXED;
        std::string mLang = "";

    private:
        SegmentList *mPSegList = nullptr;
        int id = -1;
        AdaptationSet *mAdapt = nullptr;
        std::string mBaseUrl = "";
        std::string mPlaylistUrl;
        uint64_t mBandWidth = 0;
        std::string mimeType;
        int mWidth = 0;
        int mHeight = 0;
        //std::list<std::string> lang;
    };
}


#endif //FRAMEWORK_REPRESENTATION_H
