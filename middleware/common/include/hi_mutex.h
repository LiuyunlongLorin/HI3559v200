/**
 * Copyright (C), 2018, Hisilicon Tech. Co., Ltd.
 * All rights reserved.
 *
 * @file     hi_mutex.h
 * @brief    common define.
 * @author   HiMobileCam middleware develop team
 * @date     2018.01.05
 */

#ifndef _HI_MUTEX_H
#define _HI_MUTEX_H

#include <stdint.h>
#include <sys/types.h>
#include <time.h>
#include <pthread.h>

typedef int32_t status_t;
class HiCondition;

class HiMutex {
public:
    HiMutex();
    HiMutex(const char* name);
    virtual ~HiMutex();

    status_t    lock();
    void        unlock();

    // return 0 if success, otherwise for error
    status_t    tryLock();

    class HiAutolock {
    public:
        inline HiAutolock(HiMutex& mutex) : mLock(mutex)  { mLock.lock(); }
        inline HiAutolock(HiMutex* mutex) : mLock(*mutex) { mLock.lock(); }
        inline ~HiAutolock() { mLock.unlock(); }
    private:
        HiMutex& mLock;
    };

private:
    friend class HiCondition;
    // A mutex cannot be copied
    HiMutex(const HiMutex&);
    HiMutex&      operator = (const HiMutex&);
    pthread_mutex_t mMutex;
};

// ---------------------------------------------------------------------------

inline HiMutex::HiMutex() {
    (void)pthread_mutex_init(&mMutex, NULL);
}
inline HiMutex::HiMutex(__attribute__((unused))
const char* name) {
    (void)pthread_mutex_init(&mMutex, NULL);
}

inline HiMutex::~HiMutex() {
    (void)pthread_mutex_destroy(&mMutex);
}
inline status_t HiMutex::lock() {
    return -pthread_mutex_lock(&mMutex);
}
inline void HiMutex::unlock() {
    (void)pthread_mutex_unlock(&mMutex);
}
inline status_t HiMutex::tryLock() {
    return -pthread_mutex_trylock(&mMutex);
}

typedef HiMutex::HiAutolock AutoMutex;

#endif // _HI_MUTEX_H
