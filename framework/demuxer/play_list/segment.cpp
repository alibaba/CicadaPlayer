//
// Created by moqi on 2018/4/25.
//
#include "segment.h"
#include "utils/frame_work_log.h"
#include <assert.h>

namespace Cicada {
    const int segment::SEQUENCE_INVALID = 0;
    const int segment::SEQUENCE_FIRST = 1;

    segment::segment(uint64_t seq)
    {
        sequence = seq;
    }

    segment::~segment()
    {
        //   AF_TRACE;
    }

    void segment::setSequenceNumber(uint64_t seq)
    {
        sequence = /*SEQUENCE_FIRST +*/ seq;
    }

    uint64_t segment::getSequenceNumber()
    {
        return sequence;
    }

    void segment::setSourceUrl(const std::string &value)
    {
        std::lock_guard<std::recursive_mutex> lck(mMutex);
        mUri = value;
    }

    void segment::print()
    {
        AF_LOGD("segment %llu,"
                " %s duration %lld startTime is %llu\n",
                sequence, mUri.c_str(), duration, startTime);
    }

    void segment::setEncryption(const std::vector<SegmentEncryption> &enc)
    {
        encryptions = enc;
    }

    void segment::setByteRange(int64_t start, int64_t end)
    {
        rangeStart = start;
        rangeEnd = end;
    }

    std::string segment::getDownloadUrl()
    {
        std::lock_guard<std::recursive_mutex> lck(mMutex);
        if (mSegType == SEG_NORMAL) {
            return mUri;
        } else if (mSegType == SEG_LHLS) {
            return mDownloadUri;
        } else {
            assert(0);
            return "";
        }
    }

    void segment::getDownloadRange(int64_t &start, int64_t &end)
    {
        std::lock_guard<std::recursive_mutex> lck(mMutex);
        if (mSegType == SEG_NORMAL) {
            start = rangeStart;
            end = rangeEnd;
        } else if (mSegType == SEG_LHLS) {
            start = downloadRangeStart;
            end = downloadRangeEnd;
        } else {
            assert(0);
        }
    }

    void segment::updateParts(const std::vector<SegmentPart> &parts)
    {
        std::lock_guard<std::recursive_mutex> lck(mMutex);
        mParts = parts;
        if (!mParts.empty()) {
            mSegType = SEG_LHLS;
            if (mPartsNextIndex == 0) {
                moveToNextPart();
            }
        } else {
            mSegType = SEG_NORMAL;
        }
    }

    const vector<SegmentPart> &segment::getSegmentParts()
    {
        std::lock_guard<std::recursive_mutex> lck(mMutex);
        return mParts;
    }

    void segment::moveToNextPart()
    {
        lock_guard<recursive_mutex> lck(mMutex);

        if (mUri.empty()) {
            // dose not have complete segment, use part
            if (!mParts.empty() && mPartsNextIndex >= 0 && mPartsNextIndex < mParts.size()) {
                mDownloadUri = mParts[mPartsNextIndex].uri;
                downloadRangeStart = mParts[mPartsNextIndex].rangeStart;
                downloadRangeEnd = mParts[mPartsNextIndex].rangeEnd;
                ++mPartsNextIndex;
            } else {
                AF_LOGD("Not have enough segment parts [%d] [%d]", mPartsNextIndex, (int) mParts.size());
            }
        } else {
            // has complete segment
            if (mPartsNextIndex > 0) {
                // has played part, use next part
                if (!mParts.empty() && mPartsNextIndex >= 0 && mPartsNextIndex < mParts.size()) {
                    mDownloadUri = mParts[mPartsNextIndex].uri;
                    downloadRangeStart = mParts[mPartsNextIndex].rangeStart;
                    downloadRangeEnd = mParts[mPartsNextIndex].rangeEnd;
                    ++mPartsNextIndex;
                } else {
                    AF_LOGD("Not have enough segment parts [%d] [%d]", mPartsNextIndex, (int) mParts.size());
                }
            } else {
                // has not been played, use complete segment
                mDownloadUri = mUri;
                downloadRangeStart = rangeStart;
                downloadRangeEnd = rangeEnd;
                mPartsNextIndex = -1;
            }
        }

    }

    void segment::moveToPart(int partIndex)
    {
        lock_guard<recursive_mutex> lck(mMutex);
        if (mParts.empty()) {
            return;
        }
        int fixedPartIndex = partIndex;
        if (fixedPartIndex < 0) {
            fixedPartIndex = 0;
        }
        if (fixedPartIndex >= mParts.size()) {
            fixedPartIndex = mParts.size() - 1;
        }
        if (!mUri.empty() && fixedPartIndex == 0) {
            mDownloadUri = mUri;
            downloadRangeStart = rangeStart;
            downloadRangeEnd = rangeEnd;
            mPartsNextIndex = -1;
        } else {
            mDownloadUri = mParts[fixedPartIndex].uri;
            downloadRangeStart = mParts[fixedPartIndex].rangeStart;
            downloadRangeEnd = mParts[fixedPartIndex].rangeEnd;
            mPartsNextIndex = fixedPartIndex + 1;
        }
    }

    void segment::moveToNearestIndependentPart(int partIndex)
    {
        lock_guard<recursive_mutex> lck(mMutex);
        if (mParts.empty()) {
            return;
        }
        int fixedIndex = partIndex;
        if (fixedIndex < 0) {
            fixedIndex = 0;
        }
        if (fixedIndex >= mParts.size()) {
            fixedIndex = mParts.size() - 1;
        }
        bool isFind = false;
        for (int i = fixedIndex; i >= 0; i--) {
            if (mParts.at(i).independent) {
                moveToPart(i);
                isFind = true;
                break;
            }
        }
        if (!isFind) {
            moveToPart(0);
        }
    }

    bool segment::isDownloadComplete(bool &bHasUnusedParts)
    {
        lock_guard<recursive_mutex> lck(mMutex);
        bool isComplete = !mUri.empty();
        bHasUnusedParts = (mPartsNextIndex < mParts.size());
        return isComplete;
    }

    void segment::moveToPreloadSegment(const std::string &segmentUri)
    {
        std::lock_guard<std::recursive_mutex> lck(mMutex);
        for (int i = mPartsNextIndex; i < mParts.size(); i++) {
            if (mParts[i].uri == segmentUri) {
                AF_LOGD("[lhls] move to preload partial segment, index=%d, uri=%s", i, mParts[i].uri.c_str());
                moveToPart(i);
                break;
            }
        }
    }
}// namespace Cicada
