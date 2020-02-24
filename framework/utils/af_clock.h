//
// Created by moqi on 2018/9/18.
//

#ifndef SOURCE_AF_CLOCK_H
#define SOURCE_AF_CLOCK_H

#include <cstdint>
#include <atomic>

class af_clock {
public:
    af_clock();

    ~af_clock();

    void start();

    void pause();

    void set(int64_t time);

    int64_t get();

    bool isPaused();

    void reset();


private:
    std::atomic<int64_t> mStartUs{0};
    std::atomic_int mStatus{0};
    int64_t mSetUs = 0;
    int64_t mPauseUs = 0;
};

class af_scalable_clock {
public:
    af_scalable_clock();

    virtual ~af_scalable_clock();

    void start();

    void pause();

    void set(int64_t time);

    int64_t get();

    void setSpeed(float speed);

    float getSpeed();

    bool isPaused();

    virtual void reset();

private:
    std::atomic<float> mScale{1.0f};
    af_clock mClock;
    std::atomic<int64_t> mSetTime{0};
    int64_t mScaleStartTime = 0;

};


#endif //SOURCE_AF_CLOCK_H
