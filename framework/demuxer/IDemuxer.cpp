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
                                   demuxer_callback_interrupt_data inter, demuxer_callback_setSegmentList setSegmentList,
                                   demuxer_callback_getBufferDuration getBufferDuration, demuxer_callback_enableCache enableCache,
                                   void *arg)
    {
        mReadCb = read;
        mUserArg = arg;
        mSeekCb = seek;
        mOpenCb = open;
        mInterruptCb = inter;
        mSetSegmentList = setSegmentList;
        mGetBufferDuration = getBufferDuration;
        mEnableCache = enableCache;
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

        int64_t durationMicSec = infoList[0].mDuration;
        if (totalLength > 0 && filePosition >= totalLength) {
            return durationMicSec > 0 ? durationMicSec : -1;
        }

        const streamIndexEntryInfo::entryInfo tmpEntry(filePosition, 0, false, false, 0);
        auto compFunc = [](const decltype(tmpEntry) &l, const decltype(tmpEntry) &r) { return l.mPos < r.mPos; };

        for (const auto &info : infoList) {
            const auto &entryList = info.mEntry;

            if (entryList.empty()) {
                continue;
            }

            auto it = std::upper_bound(entryList.begin(), entryList.end(), tmpEntry, compFunc);

            //TODO: we need to update the entryList during demuxing process. eg. flv mkv will update the entryList when demuxing
            if (it != entryList.begin()) {
                it--;
            }

            const auto &entry = *it;
            if (entry.mPos + entry.mSize > filePosition) {
                return entry.mTimestamp;
            }
        }

        return 0;   //NOT in any entry, regard it as meta info, return 0
    }

    std::pair<int64_t, int64_t> IDemuxer::estimatePlayTimeMicSecRange(const pair<int64_t, int64_t> &fileRange)
    {
        int64_t rangeStart = 0, rangeEnd = 0;

        const auto &infoList = getStreamIndexEntryInfo();

        if (!infoList.empty()) {
            int64_t duration = infoList[0].mDuration;
            rangeEnd = duration;

            const streamIndexEntryInfo::entryInfo tmpEntryStart(fileRange.first, 0, false, false, 0);
            const streamIndexEntryInfo::entryInfo tmpEntryEnd(fileRange.second, 0, false, false, 0);

            auto compFun = [](const decltype(tmpEntryStart) &l, const decltype(tmpEntryStart) &r) { return l.mPos < r.mPos; };

            for (const auto &info : infoList) {
                const auto &entryList = info.mEntry;

                if (entryList.empty()) {
                    continue;
                }


                auto leftMaxIt = std::lower_bound(entryList.begin(), entryList.end(), tmpEntryStart, compFun);

                if (leftMaxIt == entryList.end()) { //no more entry, no valid range
                    rangeStart = duration;
                    break;
                }
                rangeStart = std::max(rangeStart, leftMaxIt->mTimestamp);

                auto rightMinIt = std::upper_bound(entryList.begin(), entryList.end(), tmpEntryEnd, compFun);
                if (rightMinIt == entryList.begin()) { //no entry left to this pos, no valid range
                    rangeEnd = 0;
                    break;
                }
                rightMinIt--;

                rangeEnd = std::min(rangeEnd, rightMinIt->mTimestamp);
            }

        }

        return pair<int64_t, int64_t>(rangeStart, rangeEnd);
    }

    bool IDemuxer::isStreamsEncodedSeparately(const string &url)
    {
        const auto &infoList = getStreamIndexEntryInfo();
        if (infoList.size() <= 1) {
            return false;
        }
        const auto MAX_GAP_MIC_SEC = 100000L;   //100ms
        int64_t duration = -1, lastDts = -1, lastPosition = -1;
        int32_t lastSize = -1;
        for (auto &info : infoList) {
            if (info.mEntry.empty()) {
                continue;
            }

            auto &back = info.mEntry.back();
            if (duration < 0) {
                duration = info.mDuration;
                lastDts = back.mTimestamp;
                lastPosition = back.mPos;
                lastSize = back.mSize;
            } else if (abs(info.mDuration - duration) < MAX_GAP_MIC_SEC
            && abs(back.mTimestamp - lastDts) <= MAX_GAP_MIC_SEC
            && abs(back.mPos - lastPosition) >= max(lastSize, back.mSize) * 10) {
                return true;
            }
        }

        return false;
    }

}
