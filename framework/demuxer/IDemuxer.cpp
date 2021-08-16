//
// Created by moqi on 2018/2/1.
//

#include "IDemuxer.h"

#include <algorithm>
#include <utility>

namespace Cicada {

    IDemuxer::IDemuxer()
    {
    }

    IDemuxer::~IDemuxer()
    {
    }

    IDemuxer::IDemuxer(string path) : mPath(std::move(path))
    {
    }

    void IDemuxer::SetDataCallBack(demuxer_callback_read read, demuxer_callback_seek seek, demuxer_callback_open open,
                                   demuxer_callback_interrupt_data inter, demuxer_callback_setSegmentList setSegmentList, void *arg)
    {
        mReadCb = read;
        mUserArg = arg;
        mSeekCb = seek;
        mOpenCb = open;
        mInterruptCb = inter;
        mSetSegmentList = setSegmentList;
    }

    int IDemuxer::GetStreamMeta(unique_ptr<streamMeta> &meta, int index, bool sub) const
    {
        Stream_meta Meta;
        int ret = GetStreamMeta(&Meta, index, sub);

        if (ret < 0) {
            return ret;
        }

        meta = unique_ptr<streamMeta>(new streamMeta(&Meta));
        return 0;
    }

    int64_t IDemuxer::estimateExclusiveEndPositionBytes(const string &url, int64_t timeMicSec, int64_t totalLength)
    {
        const auto &infoList = getStreamIndexEntryInfo();

        if (infoList.empty() || totalLength <= 0) {
            return -1;
        }

        int64_t duration = infoList[0].mDuration;

        if (timeMicSec >= duration) {
            return totalLength;
        }

        const streamIndexEntryInfo::entryInfo tmpEntry(0, timeMicSec, false, false, 0);

        int64_t matchPos = -1;
        for (const auto &info : infoList) {
            const auto &entryList = info.mEntry;

            if (entryList.empty()) {
                continue;
            }

            auto it =
                    std::upper_bound(entryList.begin(), entryList.end(), tmpEntry,
                                     [](const decltype(tmpEntry) &l, const decltype(tmpEntry) &r) { return l.mTimestamp < r.mTimestamp; });

            if (it != entryList.begin()) {
                it--;
            }

            const auto &entry = *it;
            int64_t pos = entry.mPos + entry.mSize;
            if (pos > matchPos) {
                matchPos = pos;
            }
        }
        return matchPos;
    }

    int64_t IDemuxer::estimatePlayTimeMicSec(const string &url, int64_t filePosition, int64_t totalLength)
    {
        const auto &infoList = getStreamIndexEntryInfo();
        if (infoList.empty()) {
            return -1;
        }

        int64_t durationMills = infoList[0].mDuration;
        if (totalLength > 0 && filePosition >= totalLength) {
            return durationMills > 0 ? durationMills : -1;
        }

        const streamIndexEntryInfo::entryInfo tmpEntry(filePosition, 0, false, false, 0);

        for (const auto &info : infoList) {
            const auto &entryList = info.mEntry;

            if (entryList.empty()) {
                continue;
            }

            auto it = std::upper_bound(entryList.begin(), entryList.end(), tmpEntry,
                                       [](const decltype(tmpEntry) &l, const decltype(tmpEntry) &r) { return l.mPos < r.mPos; });

            //TODO: we need to update the entryList during demuxing process. eg. flv mkv will update the entryList when demuxing
            if (it != entryList.begin()) {
                it--;
            }

            const auto &entry = *it;
            if (entry.mPos + entry.mSize > filePosition) {
                return entry.mTimestamp;
            }
        }

        return -1;
    }

    int IDemuxer::allowDataCache(const string &url)
    {
        //TODO:
        return -1;
    };
}
