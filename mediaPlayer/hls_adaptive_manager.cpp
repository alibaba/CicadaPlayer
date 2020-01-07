#include "hls_adaptive_manager.h"
#include "general.h"

#ifdef __APPLE__

    #include <ifaddrs.h>
    #include <arpa/inet.h>
    #include <net/if.h>

#else
    #include "TrafficStats.h"
#endif

#define LOWER_SWITCH_VALUE (10*1000*1000)
#define UPPER_SWITCH_VALUE (30*1000*1000)

#include <utils/frame_work_log.h>

namespace Cicada {
    AdaptiveBitrateManager::AdaptiveBitrateManager()
    {
        mBufferService = nullptr;
    }

    AdaptiveBitrateManager::~AdaptiveBitrateManager()
    {
    }

    void AdaptiveBitrateManager::SetCurrentBitrate(int bitrate)
    {
        mCurrentBitrate = bitrate;
    }

    void AdaptiveBitrateManager::SetBufferControlPtr(BufferController *bufferService)
    {
        mBufferService = bufferService;
    }

    void AdaptiveBitrateManager::clear()
    {
        mLastDownloadBytes = 0;
        mMaxDownloadSpeed = 0;
        mSpanDownloadSpeed = 0;
        mLastSpanDownloadSpeed = 0;
        mStreamIndexBitrateMap.clear();
        mCurrentBitrate = 0;
        mStartTime = INT64_MIN;
        mLastSwitchTime = INT64_MIN;
        mLastVideoDuration = INT64_MIN;
        memset(mDurationStatics, 0, 10 * sizeof(int));
        mDurationCount = 0;
        mBitrates.clear();
    }

    int64_t AdaptiveBitrateManager::getInterfaceBytes()
    {
#ifdef __APPLE__
        struct ifaddrs *ifa_list = 0, *ifa;

        if (getifaddrs(&ifa_list) == -1) {
            return 0;
        }

        uint32_t iBytes = 0;
        uint32_t oBytes = 0;

        for (ifa = ifa_list; ifa; ifa = ifa->ifa_next) {
            if (AF_LINK != ifa->ifa_addr->sa_family) {
                continue;
            }

            if (!(ifa->ifa_flags & IFF_UP) && !(ifa->ifa_flags & IFF_RUNNING)) {
                continue;
            }

            if (ifa->ifa_data == 0) {
                continue;
            }

            /* Not a loopback device. */
            if (strncmp(ifa->ifa_name, "lo", 2)) {
                struct if_data *if_data = (struct if_data *) ifa->ifa_data;
                iBytes += if_data->ifi_ibytes;
                oBytes += if_data->ifi_obytes;
            }
        }

        freeifaddrs(ifa_list);
        return iBytes;
#else
        //TODO: android get bytes
#ifndef WIN32
        uint64_t  byteNums = getIfaceStatType(NULL, IfaceStatType::RX_BYTES);
        AF_LOGD("android get bytes .. %lld", byteNums);
        return byteNums;
#endif
#endif
        return 0;
    }

    void AdaptiveBitrateManager::reset()
    {
        memset(mDurationStatics, 0, 10 * sizeof(int));
        mDurationCount = 0;
        mLastVideoDuration = INT64_MIN;
    }

    void AdaptiveBitrateManager::AddStreamInfo(int streamIndex, int bitrate)
    {
        mStreamIndexBitrateMap.insert(pair<int, int>(bitrate, streamIndex));
        mBitrates.push_back(bitrate);
        sort(mBitrates.begin(), mBitrates.end(), std::less<int>());
    }

    void AdaptiveBitrateManager::SetCallback(streamAdaptChanged callback, void *userData)
    {
        mCallback = callback;
        mUserData = userData;
    }


    void AdaptiveBitrateManager::setCanSwitch(bool canSwitch)
    {
        mCanSwitch = canSwitch;
        reset();
    }

    void AdaptiveBitrateManager::ProcessSwitchStream(int64_t curTime)
    {
        if (mBufferService == nullptr) {
            return;
        }

        if (!mCanSwitch) {
            return;
        }

        int64_t duration = mBufferService->GetPacketDuration(BUFFER_TYPE_VIDEO);

        if (duration > mLastVideoDuration) {
            mDurationStatics[mDurationCount++] = 1;
        } else {
            mDurationStatics[mDurationCount++] = -1;
        }

        mDurationCount = mDurationCount % 10;
        mLastVideoDuration = duration;

        if (mLastSwitchTime != INT64_MIN && curTime - mLastSwitchTime < 10 * 1000) {
            return;
        }

        int totalUp = 0;

        for (int i = 0; i < 10; i++) {
            totalUp += mDurationStatics[i];
        }

        AF_LOGD("mDurationStatics is %d,duration is %lld,last duration is %lld", totalUp, duration, mLastVideoDuration);

        if ((duration < LOWER_SWITCH_VALUE && totalUp <= -8)) {
            int count = (int) mBitrates.size();

            if (count != 0) {
                int bitrate = -1;

                for (int i = count - 1; i >= 0; i--) {
                    if (mBitrates[i] <= mLastSpanDownloadSpeed) {
                        bitrate = mBitrates[i];
                        break;
                    }
                }

                if (bitrate == -1) {
                    bitrate = mBitrates[0];
                }

                if (mCurrentBitrate != bitrate) {
                    mCurrentBitrate = bitrate;

                    if (mCallback) {
                        map<int, int>::iterator iter = mStreamIndexBitrateMap.find(mCurrentBitrate);

                        if (iter != mStreamIndexBitrateMap.end()) {
                            int index = iter->second;
                            mCallback(index, mUserData);
                            mCanSwitch = false;
                            mLastSwitchTime = curTime;
                        }
                    }
                }
            }
        } else if (duration >= UPPER_SWITCH_VALUE && totalUp >= 8) {
            int bitrate = -1;
            int currentIndex = -1;
            int count = (int) mBitrates.size();

            for (int i = 0; i < count; i++) {
                if (mBitrates[i] == mCurrentBitrate) {
                    currentIndex = i;
                    break;
                }
            }

            if (currentIndex == count - 1) {
                return;
            }

            if (mLastSpanDownloadSpeed > mBitrates[currentIndex + 1]) {
                for (int i = currentIndex + 2; i < count; i++) {
                    if (mLastSpanDownloadSpeed <= mBitrates[i]) {
                        currentIndex = i;
                        bitrate = mBitrates[i];
                        break;
                    }
                }
            } else {
                bitrate = mBitrates[currentIndex + 1];
            }

            if (bitrate != -1 && mCurrentBitrate != bitrate) {
                mCurrentBitrate = bitrate;

                if (mCallback) {
                    map<int, int>::iterator iter = mStreamIndexBitrateMap.find(mCurrentBitrate);

                    if (iter != mStreamIndexBitrateMap.end()) {
                        int index = iter->second;
                        mCallback(index, mUserData);
                        mCanSwitch = false;
                        mLastSwitchTime = curTime;
                    }
                }
            }
        }
    }

    void AdaptiveBitrateManager::OnSecondTimer(int64_t curTime)
    {
        if (mStartTime == INT64_MIN) {
            mStartTime = curTime;
            mLastSwitchTime = curTime;
        }

        if (mLastDownloadBytes == 0) {
            mLastDownloadBytes = getInterfaceBytes();
        } else {
            int64_t downloadBytes = getInterfaceBytes() - mLastDownloadBytes;
            int downloadSpeed = (int) downloadBytes / (1024);

            if (mMaxDownloadSpeed < downloadSpeed) {
                mMaxDownloadSpeed = downloadSpeed;
            }

            if (mSpanDownloadSpeed < downloadSpeed) {
                mSpanDownloadSpeed = downloadSpeed;
            }

            if (curTime - mStartTime > mUpdateSpan) {
                mLastSpanDownloadSpeed = mSpanDownloadSpeed;
                mSpanDownloadSpeed = 0;
                mStartTime = curTime;
            }

            AF_LOGD("download speed is %d,download speed is %d", mMaxDownloadSpeed, downloadSpeed);
            mLastDownloadBytes = getInterfaceBytes();
            ProcessSwitchStream(curTime);
        }
    }

}//namespace Cicada
