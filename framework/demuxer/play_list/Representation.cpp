//
// Created by moqi on 2018/4/25.
//
#define LOG_TAG "Representation"

#include "Representation.h"
#include "AdaptationSet.h"
#include "Helper.h"
#include "Period.h"
#include "SegmentList.h"
#include "demuxer/dash/ISegmentBase.h"
#include "demuxer/dash/SegmentBase.h"
#include "demuxer/dash/SegmentInformation.h"
#include "demuxer/dash/SegmentList.h"
#include "demuxer/dash/SegmentTemplate.h"
#include "demuxer/dash/SegmentTimeline.h"
#include "demuxer/dash/UrlTemplate.h"
#include "playList.h"
#include "utils/AFMediaType.h"
#include "utils/frame_work_log.h"

namespace Cicada {

    Representation::Representation(AdaptationSet *adapt) : Dash::SegmentInformation(adapt)
    {
        mAdapt = adapt;
        updateStreamType();
    }

    Representation::~Representation()
    {
        delete mPSegList;
    }

    void Representation::setPlaylistUrl(const std::string &url)
    {
        mPlaylistUrl = url;
    }

    void Representation::setBaseUrl(const std::string &url)
    {
        mBaseUrl = url;
    }

    void Representation::setBandwidth(uint64_t bandwidth)
    {
        mBandWidth = bandwidth;
    }

    void Representation::setWidth(int width)
    {
        mWidth = width;
    }

    void Representation::setHeight(int height)
    {
        mHeight = height;
    }

    void Representation::print()
    {
//        AF_LOGD("Representation","url is %s\n"
//                "mBandWidth is %llu\n"
//                "res is %d x %d",mPlaylistUrl.c_str(),mBandWidth,mWidth,mHeight);
        if (mStreamType == STREAM_TYPE_AUDIO || mStreamType == STREAM_TYPE_SUB) {
            AF_LOGD("%s language is %s\n",
                    mStreamType == STREAM_TYPE_AUDIO ? "Audio" : "Subtitle",
                    mLang.c_str());
        } else if (mStreamType == STREAM_TYPE_VIDEO) {
            AF_LOGD("video size  is %d x %d with bandWidth %llu\n", mHeight, mWidth, mBandWidth);
        }

        if (mPSegList) {
            mPSegList->print();
        }
    }

    void Representation::SetSegmentList(SegmentList *segList)
    {
        mPSegList = segList;
    }

    const std::string &Representation::getBaseUrl()
    {
        return mBaseUrl;
    }

    playList *Representation::getPlaylist()
    {
        return mAdapt->getPeriod()->getPlayList();
    }

    SegmentList *Representation::GetSegmentList()
    {
        return mPSegList;
    }

    const std::string &Representation::getPlaylistUrl()
    {
        return mPlaylistUrl;
    }

    int Representation::getStreamInfo(int *width, int *height, uint64_t *bandwidth, std::string &language)
    {
        if (width) {
            *width = mWidth;
        }

        if (height) {
            *height = mHeight;
        }

        if (bandwidth) {
            *bandwidth = mBandWidth;
        }
        if (mLang.empty() && mAdapt) {
            language = mAdapt->lang;
        } else {
            language = mLang;
        }
        return 0;
    }

    AdaptationSet *Representation::getAdaptationSet()
    {
        return mAdapt;
    }

    void Representation::addCodecs(const std::string &codecsString)
    {
        std::list<std::string> list = Helper::tokenize(codecsString, ',');
        std::list<std::string>::const_iterator it;
        for (it = list.begin(); it != list.end(); ++it) {
            codecs.push_back(*it);
        }
    }

    std::string Representation::contextualize(size_t number, const std::string &component, const Dash::SegmentTemplate *templ) const
    {
        std::string str(component);
        if (!templ) return str;

        std::string::size_type pos = 0;
        while (pos < str.length()) {
            Dash::UrlTemplate::Token token;
            if (str[pos] == '$' && Dash::UrlTemplate::IsDASHToken(str, pos, token)) {
                Dash::UrlTemplate::TokenReplacement replparam;
                switch (token.type) {
                    case Dash::UrlTemplate::Token::TOKEN_TIME:
                        replparam.value = templ ? getScaledTimeBySegmentNumber(number, templ) : 0;
                        break;
                    case Dash::UrlTemplate::Token::TOKEN_BANDWIDTH:
                        replparam.value = mBandWidth;
                        break;
                    case Dash::UrlTemplate::Token::TOKEN_NUMBER:
                        replparam.value = number;
                        break;
                    case Dash::UrlTemplate::Token::TOKEN_REPRESENTATION:
                        replparam.str = getID();
                        break;
                    case Dash::UrlTemplate::Token::TOKEN_ESCAPE:
                        break;

                    default:
                        pos += token.fulllength;
                        continue;
                }
                /* Replace with newvalue */
                std::string::size_type newlen = Dash::UrlTemplate::ReplaceDASHToken(str, pos, token, replparam);
                if (newlen == std::string::npos) newlen = token.fulllength;
                pos += newlen;
            } else
                pos++;
        }

        return str;
    }

    int64_t Representation::getScaledTimeBySegmentNumber(uint64_t index, const Dash::SegmentTemplate *templ) const
    {
        int64_t time = 0;
        const Dash::SegmentTimeline *tl = templ->inheritSegmentTimeline();
        if (tl) {
            time = tl->getScaledPlaybackTimeByElementNumber(index);
        } else if (templ->inheritDuration()) {
            time = templ->inheritDuration() * index;
        }
        return time;
    }

    bool Representation::needsIndex() const
    {
        Dash::SegmentBase *base = inheritSegmentBase();
        return base && base->subsegments.empty();
    }

    const std::string &Representation::getMimeType() const
    {
        if (!mimeType.empty()) {
            return mimeType;
        }
        if (mAdapt) {
            return mAdapt->getMimeType();
        }
        return "";
    }

    void Representation::updateStreamType()
    {
        std::string mimeType = getMimeType();
        if (mimeType == "audio/mp4") {
            mStreamType = Stream_type::STREAM_TYPE_AUDIO;
        } else if (mimeType == "video/mp4") {
            mStreamType = Stream_type::STREAM_TYPE_VIDEO;
        }
    }

    bool Representation::getSegmentNumberByTime(int64_t time, uint64_t *ret) const
    {
        const Dash::ISegmentBase *profile = inheritSegmentProfile();
        return profile && profile->getSegmentNumberByTime(time, ret);
    }

    int64_t Representation::getMinAheadTime(uint64_t curnum) const
    {
        Dash::ISegmentBase *profile = inheritSegmentTemplate();
        if (!profile) {
            profile = inheritSegmentList();
        }
        if (!profile) {
            profile = inheritSegmentBase();
        }
        return profile ? profile->getMinAheadTime(curnum) : 0;
    }
}