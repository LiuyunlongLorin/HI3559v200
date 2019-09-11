/**
 * Copyright (C), 2018, Hisilicon Tech. Co., Ltd.
 * All rights reserved.
 *
 * @file     hi_mutex.h
 * @brief    common define.
 * @author   HiMobileCam middleware develop team
 * @date     2018.01.05
 */

#ifndef _HI_CONDITION_H
#define _HI_CONDITION_H

#include <stdint.h>
#include <sys/types.h>
#include <time.h>
#include <pthread.h>
#include <hi_mutex.h>

typedef int64_t nsecs_t;

class HiCondition {
public:
    enum WakeUpType {
        WAKE_UP_ONE = 0,
        WAKE_UP_ALL = 1
    };

    HiCondition();
    virtual ~HiCondition();
    status_t wait(HiMutex& mutex);
    status_t waitRelative(HiMutex& mutex, nsecs_t reltime);
    void signal();
    void signal(WakeUpType type) {
        if (type == WAKE_UP_ONE) {
            signal();
        } else {
            broadcast();
        }
    }
    void broadcast();

private:
    pthread_cond_t mCond;
};

inline HiCondition::HiCondition() {
    (void)pthread_cond_init(&mCond, NULL);
}

inline HiCondition::~HiCondition() {
    (void)pthread_cond_destroy(&mCond);
}

inline status_t HiCondition::wait(HiMutex& mutex) {
    return -pthread_cond_wait(&mCond, &mutex.mMutex);
}

inline status_t HiCondition::waitRelative(HiMutex& mutex, nsecs_t reltime) {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += reltime/1000000000;
    ts.tv_nsec+= reltime%1000000000;
    if (ts.tv_nsec >= 1000000000) {
        ts.tv_nsec -= 1000000000;
        ts.tv_sec  += 1;
    }
    return -pthread_cond_timedwait(&mCond, &mutex.mMutex, &ts);
}

inline void HiCondition::signal() {
    (void)pthread_cond_signal(&mCond);
}

inline void HiCondition::broadcast() {
    (void)pthread_cond_broadcast(&mCond);
}

#endif //_HI_CONDITION_H
