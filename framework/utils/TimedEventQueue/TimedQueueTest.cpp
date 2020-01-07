//
// Created by moqi on 2018/5/27.
//

#include <unistd.h>
#include "TimedEventQueue.h"
#include "../frame_work_log.h"

using namespace std;


struct AwesomePlayer {
    AwesomePlayer();

    ~AwesomePlayer();

//    void setUID(uid_t uid);

//    int32_t setDataSource(int fd, int64_t offset, int64_t length);
    void reset() {};

    int32_t prepare();

    int32_t play();

    void stop();
    //  bool isPlaying() const;

private:
    friend struct AwesomeEvent;

    mutable Mutex mLock;
    mutable Mutex mStatsLock;
    TimedEventQueue mQueue;
    bool mQueueStarted;

    TimedEventQueue::Event *mVideoEvent;
    bool mVideoEventPending;

    void onVideoEvent();

    void postVideoEvent_l(int64_t delayUs);

    AwesomePlayer(const AwesomePlayer &);

    AwesomePlayer &operator=(const AwesomePlayer &);
};

//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
struct AwesomeEvent : public TimedEventQueue::Event {
    AwesomeEvent(
            AwesomePlayer *player,
            void (AwesomePlayer::*method)())
            : mPlayer(player),
              mMethod(method)
    {
    }

protected:
    virtual ~AwesomeEvent() {}

    virtual void fire(TimedEventQueue *queue, int64_t /* now_us */)
    {
        (mPlayer->*mMethod)();
    }

private:
    AwesomePlayer *mPlayer;

    void (AwesomePlayer::*mMethod)();

    AwesomeEvent(const AwesomeEvent &);

    AwesomeEvent &operator=(const AwesomeEvent &);
};


//////////////////////////////////////////////////////////////
AwesomePlayer::AwesomePlayer()
        : mQueueStarted(false)
{

    mVideoEvent = new AwesomeEvent(this, &AwesomePlayer::onVideoEvent);
    mVideoEventPending = false;

    reset();
}


void AwesomePlayer::onVideoEvent()
{
    Mutex::Autolock autoLock(mLock);

    //printf("the function %s enter\n",__func__);

    if (!mVideoEventPending) {
        // The event has been cancelled in reset_l() but had already
        // been scheduled for execution at that time.
        return;
    }
    mVideoEventPending = false;

    int64_t timeUs;



    //  for(int i=0;i<10000;i++){}
    usleep(10000);

    AF_LOGD("the video event have been run\n");

    postVideoEvent_l(100000);

    //printf("the function %s exit\n",__func__);
}

void AwesomePlayer::postVideoEvent_l(int64_t delayUs)
{
    //printf("the function %s enter\n",__func__);
    if (mVideoEventPending) {
        return;
    }

    mVideoEventPending = true;
    mQueue.postEventWithDelay(mVideoEvent, delayUs < 0 ? 1000000 : delayUs);//!< delay 1 sec
    //printf("the function %s exit\n",__func__);
}

int32_t AwesomePlayer::play()
{
    Mutex::Autolock autoLock(mLock);
    AF_LOGD("the function %s\n", __func__);

    mQueue.start();

    postVideoEvent_l(10000);

    return 0;
}

void AwesomePlayer::stop()
{
    mQueue.stop();

}


/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
int main()
{

    AwesomePlayer *play = new AwesomePlayer();

    play->play();

    AF_LOGD("!!!Time Queue!!!");

    usleep(1000000);
    play->stop();
    return 0;
}