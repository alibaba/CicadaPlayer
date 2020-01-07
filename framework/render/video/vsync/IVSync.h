#include <utility>

//
// Created by moqi on 2019-08-05.
//

#ifndef FRAMEWORK_IVSYNC_H
#define FRAMEWORK_IVSYNC_H

#include <functional>
#include <cerrno>


class IVSync {
public:
    class Listener {
    public:
        virtual ~Listener() = default;

        virtual int VSyncOnInit() = 0;

        virtual int onVSync(int64_t tick) = 0;

        virtual void VSyncOnDestroy() = 0;
    };

    explicit IVSync(Listener &listener) : mListener(listener)
    {
    }

    virtual ~IVSync() = default;

    virtual int setHz(float Hz)
    {
        return -ENOTSUP;
    };

    virtual float getHz() = 0;

    virtual void start() = 0;

    virtual void pause() = 0;

protected:
    Listener &mListener;


};


#endif //FRAMEWORK_IVSYNC_H
