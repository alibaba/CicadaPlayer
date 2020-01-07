//
// Created by moqi on 2018/5/27.
//

#ifndef FRAMEWORK_THREAD_H
#define FRAMEWORK_THREAD_H

#include <pthread.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>

#undef NULL
#ifdef __cplusplus
#define NULL 0
#else
#define NULL ((void *)0)
#endif

/*****************************************************************************/

/*
 * Simple mutex class.  The implementation is system-dependent.
 *
 * The mutex must be unlocked by the thread that locked it.  They are not
 * recursive, i.e. the same thread can't lock it multiple times.
 */
class Mutex {
public:
    enum {
        PRIVATE = 0, SHARED = 1
    };

    Mutex();

    Mutex(const char *name);

    Mutex(int type, const char *name = NULL);

    ~Mutex();

    // lock or unlock the mutex
    int32_t lock();

    void unlock();

    // lock if possible; returns 0 on success, error otherwise
    int32_t tryLock();

    // Manages the mutex automatically. It'll be locked when Autolock is
    // constructed and released when Autolock goes out of scope.

    class Autolock {
    public:
        inline Autolock(Mutex &mutex) :
                mLock(mutex)
        {
            mLock.lock();
        }

        inline Autolock(Mutex *mutex) :
                mLock(*mutex)
        {
            mLock.lock();
        }

        inline ~Autolock()
        {
            mLock.unlock();
        }

    private:
        Mutex &mLock;
    };

private:
    friend class Condition;

    // A mutex cannot be copied
    Mutex(const Mutex &);

    Mutex &operator=(const Mutex &);

    pthread_mutex_t mMutex;

};

inline Mutex::Mutex()
{
    pthread_mutex_init(&mMutex, NULL);
}

inline Mutex::Mutex(const char *name)
{
    pthread_mutex_init(&mMutex, NULL);
}

inline Mutex::Mutex(int type, const char *name)
{
    if (type == SHARED) {
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
        pthread_mutex_init(&mMutex, &attr);
        pthread_mutexattr_destroy(&attr);
    } else {
        pthread_mutex_init(&mMutex, NULL);
    }
}

inline Mutex::~Mutex()
{
    pthread_mutex_destroy(&mMutex);
}

inline int32_t Mutex::lock()
{
    return -pthread_mutex_lock(&mMutex);
}

inline void Mutex::unlock()
{
    pthread_mutex_unlock(&mMutex);
}

inline int32_t Mutex::tryLock()
{
    return -pthread_mutex_trylock(&mMutex);
}

/*
 * Automatic mutex.  Declare one of these at the top of a function.
 * When the function returns, it will go out of scope, and release the
 * mutex.
 */

typedef Mutex::Autolock AutoMutex;

/*****************************************************************************/
/*
 * Condition variable class.  The implementation is system-dependent.
 *
 * Condition variables are paired up with mutexes.  Lock the mutex,
 * call wait(), then either re-wait() if things aren't quite what you want,
 * or unlock the mutex and continue.  All threads calling wait() must
 * use the same mutex for a given Condition.
 */
class Condition {
public:
    enum {
        PRIVATE = 0, SHARED = 1
    };

    Condition();

    Condition(int type);

    ~Condition();

    // Wait on the condition variable.  Lock the mutex before calling.
    int32_t wait(Mutex &mutex);

    // same with relative timeout
    int32_t waitRelative(Mutex &mutex, int64_t reltime);

    // Signal the condition variable, allowing one thread to continue.
    void signal();

    // Signal the condition variable, allowing all threads to continue.
    void broadcast();

private:
    pthread_cond_t mCond;

};

inline Condition::Condition()
{
    pthread_cond_init(&mCond, NULL);
}

inline Condition::Condition(int type)
{
    if (type == SHARED) {
        pthread_condattr_t attr;
        pthread_condattr_init(&attr);
        pthread_condattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
        pthread_cond_init(&mCond, &attr);
        pthread_condattr_destroy(&attr);
    } else {
        pthread_cond_init(&mCond, NULL);
    }
}

inline Condition::~Condition()
{
    pthread_cond_destroy(&mCond);
}

inline int32_t Condition::wait(Mutex &mutex)
{
    return -pthread_cond_wait(&mCond, &mutex.mMutex);
}

inline int32_t Condition::waitRelative(Mutex &mutex, int64_t reltime)
{

    struct timespec ts;

    // we don't support the clocks here.
    struct timeval t;
    gettimeofday(&t, NULL);
    ts.tv_sec = t.tv_sec;
    ts.tv_nsec = t.tv_usec * 1000;

    ts.tv_sec += reltime / 1000000000;
    ts.tv_nsec += reltime % 1000000000;
    if (ts.tv_nsec >= 1000000000) {
        ts.tv_nsec -= 1000000000;
        ts.tv_sec += 1;
    }
    return -pthread_cond_timedwait(&mCond, &mutex.mMutex, &ts);

}

inline void Condition::signal()
{
    pthread_cond_signal(&mCond);
}

inline void Condition::broadcast()
{
    pthread_cond_broadcast(&mCond);
}


#endif //FRAMEWORK_THREAD_H
