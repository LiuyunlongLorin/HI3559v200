//=============================================================================
//
//      types.h
//
//      POSIX types header
//
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):
// Contributors:
// Date:          2015-02-28
// Purpose:       POSIX types header
// Description:   This header contains various POSIX type definitions. These types
//                are implementation defined.
//
// Usage:         #include <sys/types.h>
//
//
//####DESCRIPTIONEND####
//
//=============================================================================
#ifndef _LITEOS_BITS_PTHREAD_TYPES_H
#define _LITEOS_BITS_PTHREAD_TYPES_H

#include <sched.h>

typedef long pthread_t;

typedef struct __pthread_attr_s
{
    unsigned int    detachstate;
    unsigned int    schedpolicy;
    struct          sched_param schedparam;
    unsigned int    inheritsched;
    unsigned int    scope;
    unsigned int    stackaddr_set;
    void            *stackaddr;
    unsigned int    stacksize_set;
    size_t          stacksize;
} pthread_attr_t;

#endif
