
#define LOG_TAG "AlivcPlayerClock"

#include <utils/timer.h>
#include "system_refer_clock.h"
#include "utils/frame_work_log.h"

#define CLOCK_DEVIATION_TIME_US (100 * 1000)

namespace Cicada {

    int64_t SystemReferClock::GetTime()
    {
        int64_t timeRet = mClock.get();

        if (mGetClock && !mClock.isPaused()) {
            int64_t referTime = mGetClock(mClockArg);

            // if the value is valid and gap is big
            if ((INT64_MIN != referTime)
                    && (llabs(referTime - timeRet) > CLOCK_DEVIATION_TIME_US)) {
                mClock.set(referTime);
                AF_LOGW("TIMEPOS reSync time %lld to  %lld\n", timeRet, referTime);
                timeRet = referTime;
            }

            //AF_LOGD("TIMEPOS timeRet: %lld", timeRet);
        }

        return timeRet;
    }


    void SystemReferClock::reset()
    {
        //AF_LOGD("TIMEPOS reset reset");
        mClock.reset();
        mGetClock = nullptr;
        mClockArg = nullptr;
    }

    void SystemReferClock::SetScale(float scale)
    {
        //AF_LOGD("TIMEPOS SetScale :%f", scale);
        mClock.setSpeed(scale);
    }

    void SystemReferClock::pause()
    {
        //AF_LOGD("TIMEPOS pause");
        mClock.pause();
    }

    void SystemReferClock::start()
    {
        //AF_LOGD("TIMEPOS start");
        mClock.start();
    }

    void SystemReferClock::setTime(int64_t time)
    {
        //AF_LOGD("TIMEPOS setTime:%lld", time);
        mClock.set(time);
    }

    void SystemReferClock::setReferenceClock(get_clock getClock, void *arg)
    {
        //AF_LOGD("TIMEPOS setReferenceClock");
        mGetClock = getClock;
        mClockArg = arg;
    }

    bool SystemReferClock::haveMaster()
    {
        return mGetClock != nullptr;
    }

    bool SystemReferClock::isMasterValid()
    {
        if (mGetClock) {
            if (INT64_MIN != mGetClock(mClockArg)) {
                return true;
            }
        }

        return false;
    }

    float SystemReferClock::GetScale()
    {
        return mClock.getSpeed();
    }
};
