//
//  LHLSSegment.cpp
//  CicadaPlayerSDK
//
//  Created by Ali-PC on 2020/9/21.
//

#include "LHLSSegment.h"
#include "Helper.h"
#include <utils/frame_work_log.h>

namespace Cicada {
    LHLSSegment::LHLSSegment(uint64_t seq, string uri, uint64_t filesize)
        :   segment         (seq),
            mFileSize       (filesize)
    {
        mUri = uri;
        segType = SEG_LHLS;
    }

    LHLSSegment::~LHLSSegment()
    {
        mParts.clear();
    }

    void LHLSSegment::updateParts(const vector<SegmentPart> &parts)
    {
        lock_guard<recursive_mutex> lck(mMutex);
        mParts = parts;
    }

    const vector<SegmentPart> &LHLSSegment::getSegmentParts()
    {
        return this->mParts;
    }

    void LHLSSegment::moveToNextPart()
    {
        lock_guard<recursive_mutex> lck(mMutex);
        std::string uri = "";

        if (mUri.empty()) {
            // has no complete segment, use part
            if (mParts.size() > 0 && mPartsNextIndex >= 0 && mPartsNextIndex < mParts.size()) {
                uri = mParts[mPartsNextIndex].uri;
                mOffset = extractOffsetFromURI(uri);
                ++mPartsNextIndex;
            } else {
                AF_LOGD("Not have enough segment parts [%d] [%d]", mPartsNextIndex, (int) mParts.size());
            }
        } else {
            // has complete segment
            if (mPartsNextIndex > 0) {
                // has played part, use next part
                if (mParts.size() > 0 && mPartsNextIndex >= 0 && mPartsNextIndex < mParts.size()) {
                    uri = mParts[mPartsNextIndex].uri;
                    mOffset = extractOffsetFromURI(uri);
                    ++mPartsNextIndex;
                } else {
                    AF_LOGD("Not have enough segment parts [%d] [%d]", mPartsNextIndex, (int) mParts.size());
                }
            } else {
                // has not been played, use complete segment
                uri = mUri;
                mPartsNextIndex = -1;
            }
        }

        mDownloadUri = uri;
        AF_LOGD("LHLSSegment::moveToNextPart [%d] [%s] [%lld]\n", mPartsNextIndex, mDownloadUri.c_str(), mOffset);
    }

    std::string LHLSSegment::getDownloadUrl()
    {
        lock_guard<recursive_mutex> lck(mMutex);

        return mDownloadUri;
    }

    bool LHLSSegment::isDownloadComplete(bool &bHasUnusedParts)
    {
        lock_guard<recursive_mutex> lck(mMutex);
        bool isComplete = true;

        if (mUri.empty()) {
            isComplete = false;
        } else {
            isComplete = (mPartsNextIndex == mParts.size()) || (mPartsNextIndex < 0);
        }

        if (isComplete) {
            bHasUnusedParts = false;
        } else {
            bHasUnusedParts = (mPartsNextIndex < mParts.size());
        }

        return isComplete;
    }

    void LHLSSegment::updateUriWithFileSize(string uri, uint64_t filesize)
    {
        lock_guard<recursive_mutex> lck(mMutex);
        mUri = uri;
        mFileSize = filesize;
    }

    uint64_t LHLSSegment::getFileSize()
    {
        return mFileSize;
    }

    uint64_t LHLSSegment::extractOffsetFromURI(string uri)
    {
        uint64_t ret = -1;
        list<string> firstLevelParts = Helper::tokenize(uri, '?');

        if (firstLevelParts.size() != 2) {
            return ret;
        }

        firstLevelParts.pop_front();
        list<string> secondLevelParts = Helper::tokenize(firstLevelParts.front(), '&');

        for (auto params : secondLevelParts) {
            list<string> thirdLevelParts = Helper::tokenize(params, '=');

            if (thirdLevelParts.size() != 2) {
                continue;
            }

            string key = thirdLevelParts.front();
            thirdLevelParts.pop_front();

            if (key == "lhs_range") {
                string value = thirdLevelParts.front();
                list<string> forthLeventParts = Helper::tokenize(params, '-');

                if (forthLeventParts.size() != 2) {
                    break;
                }

                forthLeventParts.pop_front();
                string strOffset = forthLeventParts.front();
                ret = static_cast<uint64_t>(atoll(strOffset.c_str()));
                break;
            }
        }

        return ret;
    }

    string LHLSSegment::appendRangeToURI(string uri, uint64_t from, uint64_t to)
    {
        string ret = uri;
        string::size_type pos = ret.find("?");
        char tempstr[256] = "";

        if (pos != ret.npos) {
            sprintf(tempstr, "&lhs_range=%lld-%lld", from, to);
        } else {
            sprintf(tempstr, "?lhs_range=%lld-%lld", from, to);
        }

        ret += tempstr;
        return ret;
    }
}
