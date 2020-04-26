//
// Created by moqi on 2019/12/11.
//

#ifndef CICADAMEDIA_IEVENTRECEIVER_H
#define CICADAMEDIA_IEVENTRECEIVER_H

#include <cstdint>
#include <memory>
#include <queue>
#include <mutex>

class IEventReceiver {
public:
    class Listener {
    public:
        virtual ~Listener() = default;

        virtual void onPausePlay() = 0;

        virtual void onExit() = 0;

        virtual void onStepSeek(bool forward) = 0;

        virtual void onPercentageSeek(int percent) = 0;

        virtual void onChangeVolume(bool large) = 0;

        virtual void onSetView(void *view) = 0;

        virtual void onSpeedUp(bool up) = 0;
        virtual void onPrePare() = 0;

        virtual void onFullScreen(bool full) = 0;
    };

    class IEvent {
    public:
        enum EType {
            TYPE_UNKNOWN = 0,
            TYPE_SET_VIEW,
            TYPE_EXIT,

        };
    public:
        explicit IEvent(EType type) : mType(type)
        {

        };

        EType getType()
        {
            return mType;
        }

        virtual ~IEvent() = default;

    private:
        EType mType;

    };


public:
    explicit IEventReceiver(Listener &listener) : mListener(listener)
    {

    };

    ~IEventReceiver() = default;

    virtual void poll(bool &exit) = 0;

    void push(std::unique_ptr<IEvent> event)
    {
        std::lock_guard<std::mutex> lock(mQueueMutex);
        mEventQueue.push(std::move(event));
    }

protected:
    std::unique_ptr<IEvent> pop()
    {
        std::lock_guard<std::mutex> lock(mQueueMutex);
        if (mEventQueue.empty())
            return nullptr;
        std::unique_ptr<IEvent> event = std::move(mEventQueue.front());
        mEventQueue.pop();
        return event;
    }


protected:
    Listener &mListener;

private:
    std::mutex mQueueMutex;
    std::queue<std::unique_ptr<IEvent>> mEventQueue{};


};


#endif //CICADAMEDIA_IEVENTRECEIVER_H
