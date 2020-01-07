#ifndef CICADA_PLAYER_CLOCK_H_
#define CICADA_PLAYER_CLOCK_H_

#include <cstdint>
#include <cstdlib>
#include <utils/af_clock.h>

typedef int64_t (*get_clock)(void *arg);


namespace Cicada {
    class SystemReferClock {
    public:
        SystemReferClock() = default;

        ~SystemReferClock() = default;

        void start();

        void pause();

        void setTime(int64_t time);

        int64_t GetTime();

        void SetScale(float scale);

        float GetScale();

        void setReferenceClock(get_clock getClock, void *arg);

        void reset();

        bool haveMaster();

        bool isMasterValid();

    private:
        af_scalable_clock mClock;
        get_clock mGetClock{nullptr};
        void *mClockArg{nullptr};
    };

};// namespace Cicada


#endif // CICADA_PLAYER_CLOCK_H_
