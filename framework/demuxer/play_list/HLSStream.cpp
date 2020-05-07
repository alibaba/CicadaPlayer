//
// Created by moqi on 2018/4/28.
//
#define LOG_TAG "HLSStream"

#include <utils/errors/framework_error.h>
#include <utils/timer.h>
#include <demuxer/play_list/segment_decrypt/AES_128Decrypter.h>
#include <cassert>
#include <demuxer/DemuxerMeta.h>
#include "../../utils/mediaFrame.h"
#include "HLSStream.h"
#include "Helper.h"
#include "segment_decrypt/SegDecryptorFactory.h"
//#define NOLOGD
#include "../../utils/frame_work_log.h"
#include <data_source/dataSourcePrototype.h>
#include "../IDemuxer.h"
#include <utils/af_string.h>

// TODO support active and no active mode

namespace Cicada {

    static const int defaultInitSegSize = 1024 * 1024;

    const char *HLSStream::hls_id3 = "id3v2_priv.com.apple.streaming.transportStreamTimestamp";

    HLSStream::HLSStream(SegmentTracker *pTracker, int id)
        : mPTracker(pTracker),
          mId(id)
    {
    }

    HLSStream::~HLSStream()
    {
        close();
        delete mThreadPtr;
        delete mPTracker;
        mStreamStartTimeMap.clear();

        if (mInitSegBuffer) {
            free(mInitSegBuffer);
        }
    }

    int HLSStream::getStreamType()
    {
        return mPTracker->getStreamType();
    }

    int HLSStream::GetRemainSegmentCount()
    {
        return mPTracker->GetRemainSegmentCount();
    }

    int HLSStream::Decrypter_read_callback(void *arg, uint8_t *buffer, int size)
    {
        auto *pHandle = static_cast<HLSStream *>(arg);

        if (pHandle->mExtDataSource) {
            return pHandle->mExtDataSource->Read(buffer, (size_t) size);
        }

        return pHandle->mPdataSource->Read(buffer, (size_t) size);
    }

    int HLSStream::read_callback(void *arg, uint8_t *buffer, int size)
    {
        auto *pHandle = static_cast<HLSStream *>(arg);
        int ret;

        if (pHandle->mInterrupted) {
            return -EIO;
        }

        if (pHandle->mIsDataEOS) {
            AF_LOGE("%s : %d stream(%d),EOS", __func__, __LINE__,
                    pHandle->mPTracker->getStreamType());
            return 0;
        }

        if (pHandle->mInitSegBuffer) {
            int initSegSize = pHandle->mInitSegSize - pHandle->mInitSegPtr;

            if (initSegSize > 0) {
                int readSize = std::min(initSegSize, size);
                memcpy(buffer, pHandle->mInitSegBuffer + pHandle->mInitSegPtr, readSize);
                pHandle->mInitSegPtr += readSize;
                return readSize;
            }
        }

        ret = pHandle->readSegment(buffer, size);

        if (pHandle->getStreamType() == STREAM_TYPE_SUB && pHandle->mVttPtsOffSet == INT64_MIN && ret > 0) {
            pHandle->mVttPtsOffSet = pHandle->mWVTTParser.addBuffer(buffer, ret);

            if (pHandle->mVttPtsOffSet != INT64_MIN) {
                AF_LOGD("WVTTParser pts is %lld\n", pHandle->mVttPtsOffSet);
            }
        }

        return ret;
    }

    int HLSStream::readSegment(const uint8_t *buffer, int size)
    {
        int ret;

        if (mSegDecrypter == nullptr) {
            if (mExtDataSource) {
                ret = mExtDataSource->Read((void *) buffer, size);
            } else {
                ret = mPdataSource->Read((void *) buffer, (size_t) size);
            }
        } else {
            ret = mSegDecrypter->Read(const_cast<uint8_t *>(buffer), size);
        }

        return ret;
    }

    int64_t HLSStream::seekSegment(off_t offset, int whence)
    {
        int64_t ret;

        if (mSegDecrypter == nullptr) {
            if (mExtDataSource) {
                ret = mExtDataSource->Seek(offset, whence);
            } else {
                ret = mPdataSource->Seek(offset, whence);
            }
        } else {
            // ret = mSegDecrypter->Read(const_cast<uint8_t *>(buffer), size);
            return -EINVAL;
        }

        return ret;
    }

//    off_t HLSStream::seek_callback(void *arg, off_t offset, int whence) {
////        HLSStream *pHandle = static_cast<HLSStream *>(arg);
////        AF_LOGE("test", "%s %lld \n", __func__, offset);
////        if (pHandle->mSeekCb)
////            return pHandle->mSeekCb(pHandle->mSeekArg, offset, whence);
////        return mPDataSource->Seek(offset, whence);
//        return 0;
//    }

    int HLSStream::open()
    {
//        mIsOpened = true;
//        mStatus = status_paused;
//        mIsEOS = false;
//        mIsDataEOS = false;
//        mStopOnSegEnd = false;
//        pthread_create(&read_thread_id, NULL, read_thread, this);
        return 0;
    }

    int HLSStream::upDateInitSection()
    {
        string uri;
        int ret;
        mInitSegPtr = 0;

        if (!mCurSeg || !mCurSeg->init_section || mCurInitSeg == mCurSeg->init_section) {
            return 0;
        }

        uri = Helper::combinePaths(mPTracker->getBaseUri(), mCurSeg->init_section->mUri);
        ret = tryOpenSegment(uri, mCurSeg->init_section->rangeStart, mCurSeg->init_section->rangeEnd);

        if (ret < 0) {
            return ret;
        }

        mCurInitSeg = mCurSeg->init_section;
        mInitSegSize = defaultInitSegSize;
        mInitSegSize = seekSegment(0, SEEK_SIZE);

        if (mInitSegSize < 0) {
            mInitSegSize = defaultInitSegSize;
        }

        if (mInitSegBuffer) {
            free(mInitSegBuffer);
        }

        mInitSegBuffer = static_cast<uint8_t *>(malloc(mInitSegSize));
        int size = 0;

        do {
// TODO: realloc mInitSegBuffer
            ret = readSegment(mInitSegBuffer + size, mInitSegSize - size);

            if (ret > 0) {
                size += ret;
            } else {
                break;
            }
        } while (size < mInitSegSize);

        mInitSegSize = size;
        return 0;
    }

    static inline uint64_t getSize(const uint8_t *data, unsigned int len, unsigned int shift)
    {
        uint64_t size(0);
        const uint8_t *dataE(data + len);

        for (; data < dataE; ++data) {
            size = size << shift | *data;
        }

        return size;
    };

    int HLSStream::open_internal()
    {
        int ret;
        AF_LOGD("mPTracker type is %d\n", mPTracker->getStreamType());
        //  mPTracker->setCurSegNum(0);
        ret = mPTracker->init();

        if (ret < 0) {
            AF_TRACE;
            return ret;
        }

        //      mPTracker->print();
//        if (mPTracker->getStreamType() == STREAM_TYPE_SUB && mPTracker->getSegSize() == 1) {
//            AF_LOGW("only one  subtitle seg");
//            //   mPTracker->setCurSegNum(0);
//            mSeekPendingUs = -1;
//        }

        if (mSeekPendingUs >= 0) {
            uint64_t num = 0;
            auto usSeeked = (uint64_t) mSeekPendingUs;
            bool b_ret = mPTracker->getSegmentNumberByTime(usSeeked, num);

            if (!b_ret) {
                AF_LOGE("getSegmentNumberByTime error us is %lld\n", mSeekPendingUs);
            } else {
                AF_LOGI("%s:%d stream (%d) usSeeked is %lld seek num is %d\n", __func__, __LINE__,
                        mPTracker->getStreamType(), usSeeked, num);
                mPTracker->setCurSegNum(num);
            }

            mSeekPendingUs = -1;
        }

        AF_TRACE;
        AF_LOGD("mPTracker->getCurSegNum is %llu", mPTracker->getCurSegNum());
        AF_LOGD("getFirstSegNum is %llu\n", mPTracker->getFirstSegNum());
        AF_LOGD("getSegSize is %llu\n", mPTracker->getSegSize());

        if (mPTracker->getCurSegNum() < mPTracker->getFirstSegNum() && mPTracker->isLive()) {
            AF_LOGW("skip seg %llu -->%llu", mPTracker->getCurSegNum(),
                    mPTracker->getFirstSegNum());
            mPTracker->setCurSegNum(mPTracker->getFirstSegNum());
        }

        mStopOnSegEnd = false;
        mCurSeg = nullptr;
        mCurSeg = mPTracker->getCurSegment();
        int trySegmentTimes = 0;

        do {
            trySegmentTimes++;

            if (mCurSeg == nullptr) {
                if (mPTracker->isLive()) {
                    AF_TRACE;
                    return -EAGAIN;
                } else {
                    AF_LOGE("can't find seg %llu\n", mPTracker->getCurSegNum());
                    return -1;
                }
            }

            ret = upDateInitSection();

            if (ret < 0) {
                return ret;
            }

            string uri;
            uri = Helper::combinePaths(mPTracker->getBaseUri(),
                                       mCurSeg->mUri);
            AF_LOGD("open uri is %s seq is %llu\n", uri.c_str(), mCurSeg->sequence);
            ret = tryOpenSegment(uri, mCurSeg->rangeStart, mCurSeg->rangeEnd);

            if (isHttpError(ret)) {
                resetSource();
                mCurSeg = mPTracker->getNextSegment();

                if (trySegmentTimes < 5 && !mInterrupted) {
                    af_msleep(20);
                    continue;
                }
            }

            if (ret < 0) {
                AF_TRACE;
                mDataSourceError = ret;
                resetSource();
                return ret;
            }
        } while (ret < 0);

        if (mInterrupted) {
            return FRAMEWORK_ERR_EXIT;
        }

        ret = createDemuxer();

        if (ret >= 0) {
            mIsOpened_internal = true;
        } else {
            AF_LOGE("open demuxer error %d\n", ret);
            return ret;
        }

        //     mStatus = status_inited;

        if (mPdataSource) {
            string info = mPdataSource->GetOption("connectInfo");
            openInfoArray.addJSON(CicadaJSONItem(info));
        }

        return ret;
    }

    int64_t HLSStream::getPackedStreamPTS()
    {
        Source_meta *meta = nullptr;
        mPDemuxer->GetSourceMeta(&meta);
        int64_t pts = INT64_MIN;
        Source_meta *meta1 = meta;

        while (meta1 != nullptr) {
            if (meta1->key && meta1->value) {
//                AF_LOGD("%s:[%s]", meta1->key, meta1->value);
                int ptr = 0;

                if (strcmp(meta1->key, hls_id3) == 0) {
                    uint8_t buf[8];
                    int v;

                    for (unsigned char &i : buf) {
                        if (sscanf(meta1->value + ptr, "\\x%02x", &v) == 1) {
                            ptr += 4;
                            i = v;
                        } else {
                            i = *(meta1->value + ptr);
                            ptr++;
                        }
                    }

                    uint64_t ps = getSize(buf, 8, 8);
                    pts = ps * 1000 / 90;
                    //                   AF_LOGD("ps is %u\n", ps);
                }
            }

            meta1 = meta1->next;
        }

        releaseSourceMeta(meta);
        return pts;
    }

    int HLSStream::createDemuxer()
    {
        int ret;
        mError = 0;
        mDataSourceError = 0;

        if (mDemuxerMeta && mDemuxerMeta->id != mCurSeg->discontinuityNum) {
            mDemuxerMeta = nullptr;
        }

        ret = updateDecrypter();

        if (ret < 0) {
            return ret;
        }

        if (getStreamType() == STREAM_TYPE_SUB) {
            mWVTTParser.rest();
            mVttPtsOffSet = INT64_MIN;
        }

        {
            std::lock_guard<std::mutex> lock(mHLSMutex);
            mPDemuxer = std::unique_ptr<demuxer_service>(new demuxer_service(nullptr));
        }

        mPDemuxer->setOptions(this->mOpts);
        unique_ptr<DemuxerMeta> demuxerMeta = unique_ptr<DemuxerMeta>(new DemuxerMeta());
        demuxerMeta->ownerUrl = mPTracker->getPlayListUri();
        mPDemuxer->setDemuxerMeta(demuxerMeta);
        mPDemuxer->SetDataCallBack(read_callback, this, nullptr, nullptr, nullptr);
        mPDemuxer->setSampleDecryptor(this->mSampeAesDecrypter.get());
        ret = mPDemuxer->createDemuxer(demuxer_type_unknown);

        if (ret < 0) {
            return ret;
        }

        if (mPDemuxer->getDemuxerHandle()) {
            mPDemuxer->getDemuxerHandle()->setBitStreamFormat(this->mMergeVideoHeader, this->mMergerAudioHeader);
        }

        //        if (mDemuxerMeta) {
        //            mPDemuxer->getDemuxerHandle()->setMeta(mDemuxerMeta.get());
        //        }

        ret = mPDemuxer->initOpen();

        if (ret >= 0) {
            int nbStream = mPDemuxer->GetNbStreams();
            AF_LOGI("file have %d streams\n", nbStream);
            bool needUpdateMeta = false;

            if (!mDemuxerMeta) {
                mDemuxerMeta = std::unique_ptr<DemuxerMetaInfo>(new DemuxerMetaInfo());
                mDemuxerMeta->id = mCurSeg->discontinuityNum;
                needUpdateMeta = true;
            }

            unique_ptr<streamMeta> meta = nullptr;

            // open all stream in demuxer
            for (int i = 0; i < nbStream; ++i) {
                mPDemuxer->GetStreamMeta(meta, i, false);
                Stream_type subType = ((Stream_meta *) (*meta))->type;
                Stream_type trackerType = (Stream_type) mPTracker->getStreamType();
                AF_LOGD("sub type is %d\n", subType);
                AF_LOGD("trackerType type is %d\n", trackerType);

                if ((trackerType == STREAM_TYPE_MIXED && subType != STREAM_TYPE_UNKNOWN) || subType == trackerType) {
                    AF_LOGW("open stream  index is %d\n", i);
                    mPDemuxer->OpenStream(i);
                    OpenedStreamIndex = i;
                }

                if (needUpdateMeta) {
                    mDemuxerMeta->meta.push_back(move(meta));
                }

                mDemuxerMeta->bContinue = true;
            }

            mPacketFirstPts = getPackedStreamPTS();
        }

        return ret;
    }

    int HLSStream::tryOpenSegment(const string &uri, int64_t start, int64_t end)
    {
        int retryTimes = 0;
        int ret;

        do {
            resetSource();
            ret = openSegment(uri, start, end);
            retryTimes++;

            if (retryTimes > 2) {
                break;
            }

            af_msleep(20);
        } while (isHttpError(ret) && !mInterrupted);

        return ret;
    }

    int HLSStream::openSegment(const string &uri, int64_t start, int64_t end)
    {
        int ret;

        if (mExtDataSource) {
            mExtDataSource->setRange(start, end);
            return mExtDataSource->Open(uri);
        }

        if (mPdataSource == nullptr) {
            recreateSource(uri);
            mPdataSource->setRange(start, end);
            ret = mPdataSource->Open(0);
        } else {
            mPdataSource->setRange(start, end);
            ret = mPdataSource->Open(uri);
        }

        return ret;
    }

    void HLSStream::resetSource()
    {
        std::lock_guard<std::mutex> lock(mHLSMutex);

        if (mPdataSource) {
            if (!mIsOpened_internal) {
                string info = mPdataSource->GetOption("connectInfo");
                openInfoArray.addJSON(CicadaJSONItem(info));
            }

//           std::lock_guard<std::mutex> lock(mHLSMutex);
//            delete mPdataSource;
//            mPdataSource = nullptr;
        }
    }

    void HLSStream::recreateSource(const string &url)
    {
        resetSource();
        std::lock_guard<std::mutex> lock(mHLSMutex);
        mPdataSource = dataSourcePrototype::create(url, mOpts);
        mPdataSource->Set_config(mSourceConfig);
        mPdataSource->Interrupt(mInterrupted);
    }

    void HLSStream::clearDataFrames()
    {
        std::unique_lock<std::mutex> locker(mDataMutex);

        while (0 < mQueue.size()) {
            mQueue.pop_front();
        }
    }


    bool HLSStream::updateKey()
    {
        string keyUrl = Helper::combinePaths(mPTracker->getBaseUri(),
                                             mCurSeg->encryption.keyUrl);

        if (mKeyUrl == keyUrl) {
            return false;
        }

        mKeyUrl = keyUrl;
        {
            std::lock_guard<std::mutex> lock(mHLSMutex);
            delete mSegKeySource;
            mSegKeySource = dataSourcePrototype::create(keyUrl, mOpts);
            mSegKeySource->Set_config(mSourceConfig);
        }
        int ret = mSegKeySource->Open(0);

        if (ret < 0) {
            AF_LOGE("open key file error\n");
            return ret;
        }

        int64_t size = 0;

        while (size < 16) {
            int len = mSegKeySource->Read(mKey + size, (size_t) (16 - size));

            if (len > 0) {
                size += len;
            } else {
                break;
            }
        }

        if (size != 16) {
            AF_LOGE("key size is %d not 16\n", size);
//                    delete mSegKeySource;
            mSegKeySource->Close();
            return -1;
        }

        mSegKeySource->Close();
        return true;
    }

    bool HLSStream::updateIV() const
    {
        if (!mCurSeg->encryption.ivStatic) {
            mCurSeg->encryption.iv.clear();
            mCurSeg->encryption.iv.resize(16);
            int number = (int) mCurSeg->getSequenceNumber();
            mCurSeg->encryption.iv[15] = static_cast<unsigned char>(
                                             (number /* - segment::SEQUENCE_FIRST*/) & 0xff);
            mCurSeg->encryption.iv[14] = static_cast<unsigned char>(
                                             ((number /* - segment::SEQUENCE_FIRST*/) >> 8) & 0xff);
            mCurSeg->encryption.iv[13] = static_cast<unsigned char>(
                                             ((number/* - segment::SEQUENCE_FIRST*/) >> 16) & 0xff);
            mCurSeg->encryption.iv[12] = static_cast<unsigned char>(
                                             ((number /* - segment::SEQUENCE_FIRST*/) >> 24) & 0xff);
            return true;
        }

        return false;
    }

    int HLSStream::updateSegDecrypter()
    {
        if (mCurSeg->encryption.method == SegmentEncryption::AES_128) {
            if (updateKey()) {
                if (mSegDecrypter == nullptr)
                    mSegDecrypter = unique_ptr<ISegDecrypter>(
                                        SegDecryptorFactory::create(mCurSeg->encryption.method, Decrypter_read_callback, this));

                mSegDecrypter->SetOption("decryption key", mKey, 16);
            }

            if (updateIV()) {
                mSegDecrypter->SetOption("decryption IV", &mCurSeg->encryption.iv[0], 16);
            }

            mSegDecrypter->flush();
        } else if (mCurSeg->encryption.method == SegmentEncryption::AES_PRIVATE) {
            memset(mKey, 0, 16);
            long length = mCurSeg->encryption.keyUrl.length();

            if (length > 16) {
                length = 16;
            }

            memcpy(mKey, mCurSeg->encryption.keyUrl.c_str(), length);

            if (mSegDecrypter == nullptr) {
                mSegDecrypter = unique_ptr<ISegDecrypter>(
                                    SegDecryptorFactory::create(mCurSeg->encryption.method,
                                            Decrypter_read_callback, this));
            }

            mCurSeg->encryption.iv.clear();
            mCurSeg->encryption.iv.resize(16);
            mSegDecrypter->SetOption("decryption key", mKey, 16);
            mSegDecrypter->SetOption("decryption IV", &mCurSeg->encryption.iv[0], 16);
            mSegDecrypter->flush();
        }

        return 0;
    }


    int HLSStream::updateSampleAesDecrypter()
    {
        if (updateKey()) {
            if (mSampeAesDecrypter == nullptr) {
                mSampeAesDecrypter = unique_ptr<HLSSampleAesDecrypter>(new HLSSampleAesDecrypter());
            }

            mSampeAesDecrypter->SetOption("decryption key", mKey, 16);
        }

        if (updateIV()) {
            assert(mSampeAesDecrypter != nullptr);

            if (mSampeAesDecrypter) {
                mSampeAesDecrypter->SetOption("decryption IV", &mCurSeg->encryption.iv[0], 16);
//                mSampeAesDecrypter->SetOption("decryption KEYFORMAT", (uint8_t *) mCurSeg->encryption.keyFormat.c_str(),
//                                              (int) mCurSeg->encryption.keyFormat.length());
            }
        }

        return 0;
    }

    void HLSStream::close()
    {
        stop();

        if (mPDemuxer) {
            mPDemuxer->close();
            std::lock_guard<std::mutex> lock(mHLSMutex);
            mPDemuxer = nullptr;
        }

        mIsOpened = false;
        mIsOpened_internal = false;
        openInfoArray.reset();
    }

    int HLSStream::read_thread()
    {
        int ret;
        // first seek is deal in open_internal

//        if (mIsOpened_internal &&
//            mPTracker->getCurSegNum() >
//            mPTracker->getFirstSegNum()) {  // reOpened and seek to, the next seg should be cur seg
//
//            mPTracker->setCurSegNum(mPTracker->getCurSegNum() - 1);
//        }
//        if (mDataSourceStatus != dataSource_status_valid)
//            mIsOpened_internal = false;

        if (mIsOpened && !mIsOpened_internal) {
            ret = open_internal();

            if (ret == -EAGAIN) {
                AF_LOGI("open_internal again\n");
                af_usleep(10000);
                ret = mPTracker->reLoadPlayList();

                if (ret == gen_framework_http_errno(403)) {
                    mError = ret;
                }

                return 0;
            } else if (ret < 0) {
                mError = ret;
                af_msleep(10);
                return 0; // continue retry
            }
        }

        {
            std::unique_lock<std::mutex> waitLock(mDataMutex);
            bool waitResult = mWaitCond.wait_for(waitLock, std::chrono::milliseconds(10), [this]() {
                return mQueue.size() <= 1 || mInterrupted || mSwitchNeedBreak;
            });

            if (!waitResult || mInterrupted || mSwitchNeedBreak) {
                return 0;
            }
        }

        unique_ptr<IAFPacket> tmp{};
        int packet_size = read_internal(tmp);

        if ((nullptr != tmp) && (nullptr != tmp->getData()) && (0 < tmp->getSize())) {
            std::unique_lock<std::mutex> waitLock(mDataMutex);
            mQueue.push_back(move(tmp));
        } else if (nullptr != tmp) {
            AF_LOGE("read_thread frame size be set as 0");
            return 0;
        }

        mWaitCond.notify_one();

        if (packet_size == 0) {
            mIsEOS = true;
            return -1;
        } else if (packet_size < 0) {
            if (packet_size == -EAGAIN) {
                //     AF_LOGD("read timed out");
                af_msleep(10);
                return 0;
            }

            if (packet_size != FRAMEWORK_ERR_EXIT) {
                AF_LOGD("read error 0x%4x %s\n", -packet_size, framework_err2_string(packet_size));
                mError = packet_size;
                af_msleep(10);
                return 0; // continue retry
            }
        }

        return 0;
    }

    int HLSStream::read(unique_ptr<IAFPacket> &packet)
    {
        int ret;
        packet = nullptr;

        if (mThreadPtr) {
            std::unique_lock<std::mutex> waitLock(mDataMutex);

            if (mLastReadSuccess && (mQueue.empty())) {
                mWaitCond.wait_for(waitLock, std::chrono::milliseconds(1), [this]() { return !mQueue.empty(); });
            }

            if (mQueue.empty()) {
                mLastReadSuccess = false;

                if (mIsEOS) {
                    AF_LOGI("return eos\n");
                    return 0;
                } else if (mError < 0) {
                    ret = mError;
                    mError = 0;
                    AF_LOGE("return error %d\n", ret);
                    return ret;
                } else {
                    return -EAGAIN;
                }
            }

            packet = move(mQueue.front());
            mQueue.pop_front();
            ret = static_cast<int>(packet->getSize());
            mWaitCond.notify_one();
            mLastReadSuccess = true;
            return ret;
        } else {
            return read_internal(packet);
        }
    }

    int HLSStream::updateDecrypter()
    {
        int ret = 0;

        if (mCurSeg->encryption.method == SegmentEncryption::AES_128 ||
                mCurSeg->encryption.method == SegmentEncryption::AES_PRIVATE) {
            ret = updateSegDecrypter();

            if (ret < 0) {
                return ret;
            }
        } else if (mCurSeg->encryption.method == SegmentEncryption::AES_SAMPLE) {
            ret = updateSampleAesDecrypter();

            if (ret < 0) {
                return ret;
            }
        }

        return ret;
    }

    FILE *file = NULL;


    int HLSStream::updateSegment()
    {
        shared_ptr<segment> seg = nullptr;
        AF_LOGD("getCurSegNum is %lld\n", mPTracker->getCurSegNum());
        seg = mPTracker->getNextSegment();
        int ret;
        mCurSeg = nullptr;

        if (seg) {
            do {
                mCurSeg = seg;
                string uri = Helper::combinePaths(mPTracker->getBaseUri(),
                                                  seg->mUri);
                ret = tryOpenSegment(uri, seg->rangeStart, seg->rangeEnd);

                if (isHttpError(ret)) {
                    resetSource();
                    seg = mPTracker->getNextSegment();

                    if (seg) {
                        af_msleep(20);
                        continue;
                    } else if (mPTracker->isLive()) {
                        return -EAGAIN;
                    } else {
                        //no more seg
                        break;
                    }
                }
            } while (isHttpError(ret));

            if (ret < 0) {
                mDataSourceError = ret;

                if (ret != FRAMEWORK_ERR_EXIT) {
                    mError = ret;
                }

                resetSource();
                return ret;
            }

            AF_LOGD("stream(%p) read seg %s seqno is %llu\n", this, seg->mUri.c_str(),
                    seg->getSequenceNumber());
            ret = updateDecrypter();

            if (ret < 0) {
                return ret;
            }

            return 0;
        } else if (mPTracker->getDuration() > 0) {
            AF_LOGE("EOS");
            mIsDataEOS = true;
            return -EAGAIN;
        }

        return -EAGAIN;
    }

    int HLSStream::read_internal(std::unique_ptr<IAFPacket> &packet)
    {
        //TODO: move read synMsgRst to class member
        int ret = mPTracker->reLoadPlayList();

        if (ret == gen_framework_http_errno(403)) {
            return ret;
        }

        packet = nullptr;
        ret = mPDemuxer->readPacket(packet);
        //AF_LOGD("mPDemuxer->readPacket ret is %d,pFrame is %p", ret, *pFrame);

        if (ret == -EAGAIN) {
            if (mInterrupted) {
                return ret;
            }
        } else if (ret == FRAMEWORK_ERR_EXIT) {
            AF_LOGW("mPDemuxer->readPacket FRAMEWORK_ERR_EXIT\n");
        }

        if (ret == 0 && mStopOnSegEnd) {
            mIsEOS = true;
            AF_LOGE("mStopOnSegEnd");
            return -EAGAIN;
        }

        if (ret == network_errno_http_range) {
            ret = 0;
        }

        if (ret == 0 || mReopen) {
            if (mReopen) {
                AF_LOGD("reopen");
                mReopen = false;
            }

            ret = updateSegment();

            if (mIsDataEOS) {
                return 0;
            }

            if (ret >= 0) {
                upDateInitSection();
                ret = createDemuxer();

                if (ret >= 0) {
                    int nbStream = mPDemuxer->GetNbStreams();
                    AF_LOGI("file have %d streams\n", nbStream);
                    // open all stream in demuxer
                    Stream_meta meta{};

                    for (int i = 0; i < nbStream; ++i) {
                        mPDemuxer->GetStreamMeta(&meta, i, false);

                        if (meta.type == mPTracker->getStreamType()
                                || (mPTracker->getStreamType() == STREAM_TYPE_MIXED && meta.type != STREAM_TYPE_UNKNOWN)) {
                            mPDemuxer->OpenStream(i);
                        }

                        releaseMeta(&meta);
                    }

                    mPacketFirstPts = getPackedStreamPTS();
                }
            }

            packet = nullptr;
            return -EAGAIN;
        }

        if (ret == -EAGAIN && mPTracker->getDuration() == 0) {
            ret = updateSegment();

            if (ret < 0) {
                return ret;
            }

            return -EAGAIN;
        }

        if (packet != nullptr) {
            //  AF_LOGD("read a frame \n");
            if (mPTracker->getStreamType() != STREAM_TYPE_MIXED) {
                packet->getInfo().streamIndex = 0;
            }

            if (mPacketFirstPts != INT64_MIN && packet->getInfo().pts != INT64_MIN) {
                packet->getInfo().pts += mPacketFirstPts;
                packet->getInfo().dts += mPacketFirstPts;
            }

            if (mVttPtsOffSet != INT64_MIN && packet->getInfo().pts != INT64_MIN) {
                packet->getInfo().pts += mVttPtsOffSet;
                packet->getInfo().dts += mVttPtsOffSet;
            }

            if (mCurSeg) {
                // mark the seg start time to first seg frame
                AF_LOGD("stream (%d) mark startTime %llu\n", mPTracker->getStreamType(),
                        mCurSeg->startTime);
                AF_LOGD("stream (%d)pFrame->pts is %lld pos is %lld flags is %d streamIndex is %d\n",
                        mPTracker->getStreamType(), packet->getInfo().pts, packet->getInfo().pos,
                        packet->getInfo().flags, packet->getInfo().streamIndex);

                if (packet->getInfo().flags == 0) {
                    AF_LOGE("not a key frame\n");
                    _hex_dump(packet->getData(), 16);
                }

                int nbStreams = GetNbStreams();

                for (int i = 0; i < nbStreams; i++) {
                    mStreamStartTimeMap[i].timePosition = mCurSeg->startTime;
                    mStreamStartTimeMap[i].seamlessPoint = true;
                }

                mCurSeg = nullptr;
            }

            int streamIndex = packet->getInfo().streamIndex;
            packet->getInfo().seamlessPoint = mStreamStartTimeMap[streamIndex].seamlessPoint;

            if (mStreamStartTimeMap[streamIndex].seamlessPoint) {
                if (packet->getInfo().pts != INT64_MIN) {
                    mStreamStartTimeMap[streamIndex].time2ptsDelta = mStreamStartTimeMap[streamIndex].timePosition - packet->getInfo().pts;
                }

                mStreamStartTimeMap[streamIndex].seamlessPoint = false;
            }

            if (packet->getInfo().duration > 0) {
                mStreamStartTimeMap[streamIndex].frameDuration = packet->getInfo().duration;
            }

            if (packet->getInfo().pts == INT64_MIN
                    && mStreamStartTimeMap[streamIndex].lastFramePts != INT64_MIN
                    && mStreamStartTimeMap[streamIndex].frameDuration != INT64_MIN) {
                packet->getInfo().pts = mStreamStartTimeMap[streamIndex].lastFramePts + mStreamStartTimeMap[streamIndex].frameDuration;
            }

            if (packet->getInfo().pts != INT64_MIN && mStreamStartTimeMap[streamIndex].time2ptsDelta != INT64_MIN) {
                packet->getInfo().timePosition = packet->getInfo().pts + mStreamStartTimeMap[streamIndex].time2ptsDelta;
            } else {
                packet->getInfo().timePosition = INT64_MIN;
            }

            if (packet->getInfo().pts != INT64_MIN) {
                mStreamStartTimeMap[streamIndex].lastFramePts = packet->getInfo().pts;
            }

//          AF_LOGE("pFrame->pts is %lld index is %d\n", (*pFrame)->pts, (*pFrame)->streamIndex);
        }

        return ret;
    }


    int HLSStream::GetNbStreams()
    {
        if (mPTracker->getStreamType() == STREAM_TYPE_MIXED) { //mediaPlayList
            if (mPDemuxer) {
                return mPDemuxer->GetNbStreams();
            }
        }

        return 1; // masterPlayList only support one es stream per stream url(ext-media-info ext-stream-info) now
    }

    int HLSStream::GetStreamMeta(Stream_meta *meta, int index, bool sub)
    {
        // if not muxed stream ,get the 0 stream default index
        meta->type = (Stream_type) mPTracker->getStreamType();

        if (meta->type != STREAM_TYPE_MIXED) {
            index = OpenedStreamIndex;
        }

        uint64_t bandwidth;
        std::string lang;
        int width;
        int height;
        mPTracker->getStreamInfo(&width, &height, &bandwidth, lang);
        {
            std::lock_guard<std::mutex> lock(mHLSMutex);

            if (mPDemuxer) {
                mPDemuxer->GetStreamMeta(meta, index, sub);
            }
        }

        // meta->type would be override, recover it
        if (!sub) {
            meta->type = (Stream_type) mPTracker->getStreamType();
        }

        if (meta->height == 0) {
            meta->height = height;
            meta->width = width;
        }

        meta->lang = strdup(lang.c_str());
        meta->bandwidth = bandwidth;
        meta->duration = mPTracker->getDuration();

        if (!mPTracker->getDescriptionInfo().empty()) {
            meta->description = strdup(mPTracker->getDescriptionInfo().c_str());
        }

        return 0;
    }

    bool HLSStream::isOpened()
    {
        return mIsOpened;
    }

    int HLSStream::start()
    {
//        demuxer_msg::StartReq start;
//        mPProxyService->SendMsg(start, mPDemuxer->GetAddr(), false);
        mIsOpened = true;
        mIsEOS = false;
        mIsDataEOS = false;
        mStopOnSegEnd = false;
        mError = 0;

        if (mThreadPtr == nullptr) {
            mThreadPtr = NEW_AF_THREAD(read_thread);
        }

        mThreadPtr->start();
        return 0;
    }

    int HLSStream::stop()
    {
        AF_TRACE;

        if (mThreadPtr) {
            AF_TRACE;
            interrupt_internal(1);
            mWaitCond.notify_one();
            AF_TRACE;
            mThreadPtr->stop();
            AF_TRACE;
            interrupt_internal(mInterrupted);
            AF_TRACE;
        }

        resetSource();
        {
            std::lock_guard<std::mutex> lock(mHLSMutex);

            if (mPdataSource) {
                mPdataSource->Close();
                delete mPdataSource;
                mPdataSource = nullptr;
            }

            if (mSegKeySource) {
                mSegKeySource->Close();
                delete mSegKeySource;
                mSegKeySource = nullptr;
            }

            mIsOpened_internal = false;
        }
        clearDataFrames();
        AF_LOGD("%s\n", __func__);
        return 0;
    }

    // TODO: deal seek in read_thread
    int64_t HLSStream::seek(int64_t us, int flags)
    {
        (void) flags;
        uint64_t num = 0;
        auto usSought = (uint64_t) us;
        bool reqReOpen = true;
        AF_LOGD("%s:%d stream (%d) seek us is %lld\n", __func__, __LINE__,
                mPTracker->getStreamType(), us);

        if (!mPTracker->isInited()) {
            mSeekPendingUs = us;
            AF_LOGI("pending seek\n");
            return us;
        }

        //   int ret = mPTracker->init();
        bool b_ret = mPTracker->getSegmentNumberByTime(usSought, num);

        if (!b_ret) {
            AF_LOGE("(%d)getSegmentNumberByTime error us is %lld\n", mPTracker->getStreamType(),
                    us);

            if (mPTracker->getStreamType() == STREAM_TYPE_SUB) {
                mIsEOS = false;
                mError = 0;

                if (mThreadPtr) {
                    mThreadPtr->start();
                }
            }

            return -1;
        }

        AF_LOGD("%s:%d stream (%d) usSeeked is %lld seek num is %d\n", __func__, __LINE__,
                mPTracker->getStreamType(), usSought, num);

        if (mPTracker->getStreamType() == STREAM_TYPE_SUB && num == mPTracker->getCurSegNum()) {
            AF_LOGW("only one  subtitle seg");
            reqReOpen = false;
            seek_internal(num, us);
        }

        {
            std::unique_lock<std::mutex> waitLock(mDataMutex);
            mSwitchNeedBreak = true;
        }

        mWaitCond.notify_one();
        interrupt_internal(1);

        if (mThreadPtr) {
            mThreadPtr->pause();
        }

        interrupt_internal(mInterrupted);
        mSwitchNeedBreak = false;
        clearDataFrames();

        if (reqReOpen) {
            resetSource();

            if (mIsOpened_internal) {
                num--;
                mReopen = true;
            }

            mPTracker->setCurSegNum(num);
        }

        mIsEOS = false;
        mIsDataEOS = false;
        mError = 0;
        mDemuxerMeta->bContinue = false;

        if (mThreadPtr) {
            mThreadPtr->start();
        }

        return usSought;
    }

    int64_t HLSStream::seek_internal(uint64_t segNum, int64_t us)
    {
        (void) segNum;

        // subtitle need us to find frame index to read
        //TODO: use flush to flash all
        if (mPDemuxer) {
            mPDemuxer->Seek(us, 0, -1);
            mPDemuxer->flush();
        }

        return 0;
    }

    uint64_t HLSStream::getCurSegNum()
    {
        return mPTracker->getCurSegNum();
    }

    int HLSStream::stopOnSegEnd(bool stop)
    {
        mStopOnSegEnd = stop;
        return 0;
    }

    int HLSStream::SetCurSegNum(uint64_t num)
    {
        return reopenSegment(num, OpenType::SegNum);
    }

    int HLSStream::reopenSegment(uint64_t num, OpenType openType)
    {
        {
            std::unique_lock<std::mutex> waitLock(mDataMutex);
            mSwitchNeedBreak = true;
        }

        mWaitCond.notify_one();

        if (mThreadPtr) {
            mThreadPtr->pause();
        }

        mSwitchNeedBreak = false;
        clearDataFrames();
        resetSource();

        if (mIsOpened_internal) {
            mReopen = true;
            num--;
        }

        if (openType == OpenType::SegNum) {
            mPTracker->setCurSegNum(num);
            AF_LOGD("setCurSegNum %llu\n", num);
        } else if (openType == OpenType::SegPosition) {
            mPTracker->setCurSegPosition(num);
            AF_LOGD("setCurSegPosition %llu\n", num);
        }

        seek_internal(num, 0);
        mIsEOS = false;
        mIsDataEOS = false;
        mError = 0;

        if (mThreadPtr) {
            mThreadPtr->start();
        }

        return 0;
    }

    uint64_t HLSStream::getCurSegPosition()
    {
        return mPTracker->getCurSegPosition();
    };

    int HLSStream::setCurSegPosition(uint64_t position)
    {
        return reopenSegment(position, OpenType::SegPosition);
    }

    bool HLSStream::isLive()
    {
        return mPTracker->isLive();
    }

    int64_t HLSStream::getDuration()
    {
        return mPTracker->getDuration();
    }

    int HLSStream::getNBStream()
    {
        if (mPDemuxer) {
            return mPDemuxer->GetNbStreams();
        }

        return 1;
    }

    void HLSStream::interrupt(int inter)
    {
        {
            std::unique_lock<std::mutex> waitLock(mDataMutex);
            mInterrupted = static_cast<bool>(inter);
        }
        interrupt_internal(inter);
    }

    void HLSStream::interrupt_internal(int inter)
    {
        {
            std::lock_guard<std::mutex> lock(mHLSMutex);

            if (mSegKeySource) {
                mSegKeySource->Interrupt(static_cast<bool>(inter));
            }

            if (mPdataSource) {
                mPdataSource->Interrupt(static_cast<bool>(inter));
            }

            if (mExtDataSource) {
                mExtDataSource->Interrupt(static_cast<bool>(inter));
            }
        }
        {
            std::lock_guard<std::mutex> lock(mHLSMutex);

            if (mPDemuxer) {
                mPDemuxer->interrupt(inter);
            }
        }

        if (mPTracker) {
            mPTracker->interrupt(inter);
        }
    }

    std::string HLSStream::GetProperty(const string &key)
    {
        if ("openJsonInfo" == key) {
            return openInfoArray.printJSON();
        } else if ("responseInfo" == key) {
            std::lock_guard<std::mutex> lock(mHLSMutex);

            if (mPdataSource) {
                return mPdataSource->GetOption(key);
            }

            return "";
        } else if ("probeInfo" == key) {
            std::lock_guard<std::mutex> lock(mHLSMutex);

            if (mPDemuxer) {
                return mPDemuxer->GetProperty(-1, key);
            }
        }

        return "";
    }

    HLSStream::WebVttParser::WebVttParser() = default;

    HLSStream::WebVttParser::~WebVttParser()
    {
        if (mBuffer) {
            free(mBuffer);
        }
    }

    static int64_t read_ts(const char *s)
    {
        int hh, mm, ss, ms;

        if (sscanf(s, "%u:%u:%u.%u", &hh, &mm, &ss, &ms) == 4) {
            return (hh * 3600LL + mm * 60LL + ss) * 1000LL + ms;
        }

        if (sscanf(s, "%u:%u.%u", &mm, &ss, &ms) == 3) {
            return (mm * 60LL + ss) * 1000LL + ms;
        }

        return INT64_MIN;
    }

    int64_t HLSStream::WebVttParser::addBuffer(uint8_t *buffer, int size)
    {
        if (bFinished) {
            return mMapPTS;
        }

        char *lend;

        if ((lend = strnstr((const char *) buffer, "\n\n", size)) != nullptr
                || (lend = strnstr((const char *) buffer, "\r\n\r\n", size)) != nullptr) {
            bFinished = true;
        }

        size_t append_size = size;

        if (lend != nullptr) {
            append_size = lend - (const char *) buffer;
        }

        uint8_t *new_data = static_cast<uint8_t *>(realloc(mBuffer, mSize + append_size));

        if (new_data) {
            mBuffer = new_data;
        } else {
            return mMapPTS;
        }

        memcpy(mBuffer + mSize, buffer, append_size);
        mSize += append_size;

        if (!bFinished) {
            return INT64_MIN;
        }

        mBuffer[mSize - 1] = 0;
        /*
        * WebVTT files in HLS streams contain a timestamp offset for
        * syncing with the main stream:
        *
        * X-TIMESTAMP-MAP=LOCAL:00:00:00.000,MPEGTS:900000
        * (LOCAL and MPEGTS can be reversed even though HLS spec
        *  does not say so)
        */
        char *hls_timestamp_map = strstr(reinterpret_cast<char *>(mBuffer), "\nX-TIMESTAMP-MAP=");

        if (hls_timestamp_map == nullptr) {
            return mMapPTS;
        }

        char *native_str = strstr(hls_timestamp_map, "LOCAL:");
        char *mpegts_str = strstr(hls_timestamp_map, "MPEGTS:");

        if (native_str && mpegts_str) {
            int64_t native_ts = read_ts(native_str + 6);
            int64_t mpegts_ts = strtoll(mpegts_str + 7, nullptr, 10);

            if (native_ts != INT64_MIN) {
                mMapPTS = mpegts_ts * 100 / 9 - native_ts * 1000;
            }
        }

        return mMapPTS;
    }

    void HLSStream::WebVttParser::rest()
    {
        mSize = 0;
        mMapPTS = INT64_MIN;
        bFinished = false;
    }
}
