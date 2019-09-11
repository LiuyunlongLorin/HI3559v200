/*----------------------------------------------------------------------------
 * Copyright (c) <2013-2015>, <Huawei Technologies Co., Ltd>
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific prior written
 * permission.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *---------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------
 * Notice of Export Control Law
 * ===============================================
 * Huawei LiteOS may be subject to applicable export control laws and regulations, which might
 * include those applicable to Huawei LiteOS of U.S. and the country in which you are located.
 * Import, export and usage of Huawei LiteOS in any manner by you shall be in compliance with such
 * applicable export control laws and regulations.
 *---------------------------------------------------------------------------*/

/**@defgroup mqueue Message queue
 * @ingroup posix
 */

#ifndef _HWLITEOS_POSIX_MQUEUE_H
#define _HWLITEOS_POSIX_MQUEUE_H

/* CONFIGURATION */

/* INCLUDES */
#include "stdarg.h"
#include "stdlib.h"
#include "limits.h"
#include "los_typedef.h"
#include "time.h"

#include "los_queue.ph"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */


/* TYPES */

/**
 * @ingroup mqueue
 * Maximum number of messages in a message queue
 */
#define MQ_MAX_MSG_NUM    16

/**
 * @ingroup mqueue
 * Maximum size of a single message in a message queue
 */
#define MQ_MAX_MSG_LEN    64


/* CONSTANTS */

#define MQ_USE_MAGIC  0x89abcdef
/* not suppurt prio */
#define MQ_PRIO_MAX 1

/* TYPE DEFINITIONS */
struct mqarray{
    UINT32 mq_id;
    QUEUE_CB_S *mqcb;
    struct mqpersonal *mq_personal;
    char mq_name[PATH_MAX];
    BOOL unlinkflag;
};

struct mqpersonal {
    struct mqarray *mq_posixdes;
    struct mqpersonal *mq_next;
    int mq_flags;
    UINT32 mq_status;
};

#ifndef MQ_DEFINE_DEBUG
#define  MQ_POSIX_DEBUG(a)
#else
#define  MQ_POSIX_DEBUG(a) \
    PRINT_ERR("%s, %d, errno: %d, a: %d\n", __FUNCTION__, __LINE__, errno, a)
#endif

/**
 * @ingroup mqueue
 * Message queue attribute structure
 */
struct mq_attr {
    long mq_flags;    /**<Message queue flags*/
    long mq_maxmsg;   /**<Maximum number of messages*/
    long mq_msgsize;  /**<Maximum size of a message*/
    long mq_curmsgs;  /**<Number of messages in the current message queue*/
};

/**
 * @ingroup mqueue
 * Handle type of a message queue
 */
typedef AARCHPTR   mqd_t;

/* FUNCTIONS */

/**
 *@ingroup mqueue
 *@brief Create a new posix message queue or open an existed queue.
 *
 *@par Description:
 *<ul>
 *<li>This API is used to open an existed message queue that has a specified name or create a new message queue.</li>
 *</ul>
 *@attention
 *<ul>
 *<li>A message queue does not restrict the read and write permissions.</li>
 *<li>The length of mqueue name must less than 256.</li>
 *<li>This operation and closed mqueue scheduling must be used in coordination to release the resource.</li>
 *</ul>
 *
 *@param mq_name        [IN] Message queue name.
 *@param oflag       [IN] Permission attributes of the message queue. The value range is [O_RDONLY, O_WRONLY, O_RDWR, O_CREAT, O_EXCL, O_NONBLOCK].
 *@param mode        [IN] Message queue mode (variadic argument). When oflag is O_CREAT, it requires two additional arguments: mode, which shall be of type mode_t, and attr, which shall be a pointer to an mq_attr structure.
 *@param attr        [IN] Message queue attribute (variadic argument).
 *
 *@retval  mqd_t  The message queue is successfully opened or created.
 *@retval  (mqd_t)-1      The message queue fails to be opened or created, with any of the following error codes in errno:
                   #ENOENT    : O_CREAT flag is not set for oflag, and the message queue specified by name does not exist.
                   #EEXIST    : Both O_CREAT and O_EXCL are set for oflag, but the message queue specified by name already exists.
                   #EINVAL    : invalid parameter.
                   #ENFILE    : The number of opened message queues exceeds the maximum limit.
                   #ENOSPC    : insufficient memory.
                   #ENAMETOOLONG    : The message queue name specified by name is too long.
 *@par Dependency:
 *<ul><li>mqueue.h: the header file that contains the API declaration.</li></ul>
 *@see mq_close
 *@since Huawei LiteOS V100R001C00
 */
extern mqd_t mq_open(const char *mq_name, int oflag, ...);

/**
 *@ingroup mqueue
 *@brief Close a message queue.
 *
 *@par Description:
 *<ul>
 *<li>This API is used to close a message queue that has a specified descriptor.</li>
 *</ul>
 *@attention
 *<ul>
 *<li> If the message queue is empty, it will be reclaimed, which is similar to when mq_unlink is called.</li>
 *</ul>
 *
 *@param personal   [IN]Message queue descriptor.
 *
 *@retval  0    The message queue is successfully closed.
 *@retval -1    The message queue fails to be closed, with either of the following error codes in errno:
                #EBADF  : Invalid message queue descriptor.
                #EAGAIN : Failed to delete the message queue.
                #EFAULT : Failed to free the message queue.
                #EINVAL : Invalid parameter.
 *@par Dependency:
 *<ul><li>mqueue.h: the header file that contains the API declaration.</li></ul>
 *@see mq_open
 *@since Huawei LiteOS V100R001C00
 */
extern int mq_close(mqd_t personal);

/**
 *@ingroup mqueue
 *@brief Remove a message queue.
 *
 *@par Description:
 *<ul>
 *<li>This API is used to remove a message queue that has a specified name.</li>
 *</ul>
 *@attention
 *<ul>
 *<li> If the message queue is empty, it will be reclaimed, which is similar to when mq_close is called.</li>
 *<li> The length of mqueue name must less than 256.</li>
 *</ul>
 *
 *@param mq_name   [IN]Message queue name.
 *
 *@retval  0    The message queue is successfully removed.
 *@retval -1    The message queue fails to be removed, with any of the following error codes in errno:
                #ENOENT : The message queue specified by name does not exist.
                #EAGAIN : Failed to delete the message queue.
                #EBUSY  : The message queue to be removed is being used.
                #EINVAL : Invalid parameter.
                #ENAMETOOLONG : The name of mqueue is too long.
 *@par Dependency:
 *<ul><li>mqueue.h: the header file that contains the API declaration.</li></ul>
 *@see mq_close
 *@since Huawei LiteOS V100R001C00
 */
extern int mq_unlink(const char *mq_name);

/**
 *@ingroup mqueue
 *@brief Send a message.
 *
 *@par Description:
 *<ul>
 *<li>This API is used to put a message with specified message content and length into a message queue that has a specified descriptor.</li>
 *</ul>
 *@attention
 *<ul>
 *<li> Priority-based message processing is not supported.</li>
 *<li> The msg_len should be same to the length of string which msg_ptr point to.</li>
 *</ul>
 *
 *@param personal      [IN]Message queue descriptor.
 *@param msg_ptr    [IN]Pointer to the message content to be sent.
 *@param msg_len    [IN]Length of the message to be sent.
 *@param msg_prio   [IN]Priority of the message to be sent (the value of this parameter must be 0 because priority-based message sending is not supported. If the value is not 0, this API will cease to work.)
 *
 *@retval  0    The message is successfully sent.
 *@retval -1    The message fails to be sent, with any of the following error codes in errno:
                #EINTR      : An interrupt is in progress while the message is being sent.
                #EBADF      : The message queue is invalid or not writable.
                #EAGAIN     : The message queue is full.
                #EINVAL     : Invalid parameter.
                #ENOSPC     : Insufficient memory.
                #EMSGSIZE   : The message to be sent is too long.
                #EOPNOTSUPP : The operation is not supported.
                #ETIMEDOUT  : The operation times out.
 *@par Dependency:
 *<ul><li>mqueue.h: the header file that contains the API declaration.</li></ul>
 *@see mq_receive
 *@since Huawei LiteOS V100R001C00
 */
extern int mq_send(mqd_t personal, const char *msg_ptr, size_t msg_len, unsigned int msg_prio);

/**
 *@ingroup mqueue
 *@brief Receive a message.
 *
 *@par Description:
 *<ul>
 *<li>This API is used to remove the oldest message from the message queue that has a specified descriptor, and puts it in the buffer pointed to by msg_ptr.</li>
 *</ul>
 *@attention
 *<ul>
 *<li> Priority-based message processing is not supported.</li>
 *<li> The msg_len should be same to the length of string which msg_ptr point to.</li>
 *</ul>
 *
 *@param personal      [IN]Message queue descriptor.
 *@param msg_ptr    [IN]Pointer to the message content to be received.
 *@param msg_len    [IN]Length of the message to be received.
 *@param msg_prio    [IN]Priority of the message to be received (the value of this parameter must be 0 because priority-based message processing is not supported. If the value is not 0, this API will cease to work).
 *
 *@retval  0    The message is successfully received.
 *@retval -1    The message fails to be received, with any of the following error codes in the errno:
                #EINTR      : An interrupt is in progress while the message is being received.
                #EBADF      : The message queue is invalid or not readable.
                #EAGAIN     : The message queue is empty.
                #EINVAL     : invalid parameter.
                #EOPNOTSUPP : The operation is not supported.
                #EMSGSIZE   : The message to be received is too long.
                #ETIMEDOUT  : The operaton times out.
 *@par Dependency:
 *<ul><li>mqueue.hthe header file that contains the API declaration.</li></ul>
 *@see mq_send
 *@since Huawei LiteOS V100R001C00
 */
extern ssize_t mq_receive(mqd_t personal, char *msg_ptr, size_t msg_len, unsigned int *msg_prio);

#ifdef LITEOS_POSIX_REALTIME_SIGNALS

struct sigevent;

extern int
mq_notify(mqd_t mqdes, const struct sigevent *notification);
#endif

/**
 *@ingroup mqueue
 *@brief Set message queue attributes.
 *
 *@par Description:
 *<ul>
 *<li>This API is used to modify attributes of the message queue that has a specified descriptor.</li>
 *</ul>
 *@attention
 *<ul>
 *<li> The mq_maxmsg, mq_msgsize, and mq_curmsgs attributes are not modified in the message queue attribute setting.</li>
 *</ul>
 *
 *@param personal       [IN]Message queue descriptor.
 *@param mqstat      [IN]New attribute of the message queue.
 *@param omqstat     [OUT]Old attribute of the message queue.
 *
 *@retval   0    The message queue attributes are successfully set.
 *@retval   -1   The message queue attributes fail to be set, with either of the following error codes in the errno:
                 #EBADF     : Invalid message queue.
                 #EINVAL    : Invalid parameter.
 *@par Dependency:
 *<ul><li>mqueue.h: the header file that contains the API declaration.</li></ul>
 *@see mq_getattr
 *@since Huawei LiteOS V100R001C00
 */
extern int mq_setattr(mqd_t personal, const struct mq_attr *mqstat, struct mq_attr *omqstat);

/**
 *@ingroup mqueue
 *@brief Obtain message queue attributes.
 *
 *@par Description:
 *<ul>
 *<li>This API is used to obtain attributes of the message queue that has a specified descriptor.</li>
 *</ul>
 *@attention
 *<ul>
 *<li> </li>
 *</ul>
 *
 *@param personal       [IN]Message queue descriptor.
 *@param mqstat      [OUT]Message queue attribute.
 *
 *@retval   0   The message queue attributes are successfully obtained.
 *@retval  -1   The message queue attributes fail to be obtained, with any of the following error codes in errno:
                #EBADF  : Invalid message queue.
                #EINVAL : Invalid parameter.
 *@par Dependency:
 *<ul><li>mqueue.h: the header file that contains the API declaration.</li></ul>
 *@see mq_setattr
 *@since Huawei LiteOS V100R001C00
 */
extern int mq_getattr(mqd_t personal, struct mq_attr *mqstat);


/* POSIX 1003.1d Draft functions - FIXME: should be conditionalized */

/**
 *@ingroup mqueue
 *@brief Send a message at a scheduled time.
 *
 *@par Description:
 *<ul>
 *<li>This API is used to put a message with specified message content and length into a message queue that has a descriptor at a scheduled time.</li>
 *</ul>
 *@attention
 *<ul>
 *<li> Priority-based message processing is not supported.</li>
 *<li> The expiry time must be later than the current time.</li>
 *<li> The wait time is a relative time.</li>
 *<li> The msg_len should be same to the length of string which msg_ptr point to.</li>
 *</ul>
 *
 *@param mqdes           [IN]Message queue descriptor.
 *@param msg_ptr         [IN]Pointer to the message content to be sent.
 *@param msg_len         [IN]Length of the message to be sent.
 *@param msg_prio        [IN]Priority of the message to be sent (the value of this parameter must be 0 because priority-based message processing is not supported).
 *@param abs_timeout     [IN]Scheduled time at which the message will be sent. If the value is 0, the message is an instant message.
 *
 *@retval   0   The message is successfully sent.
 *@retval  -1   The message fails to be sent, with any of the following error codes in errno:
                #EINTR        : An interrupt is in progress while the message is being sent.
                #EBADF        : The message queue is invalid or not writable.
                #EAGAIN       : The message queue is full.
                #EINVAL       : Invalid parameter.
                #EMSGSIZE     : The message to be sent is too long.
                #EOPNOTSUPP   : The operation is not supported.
                #ETIMEDOUT    : The operation times out.
 *@par Dependency:
 *<ul><li>mqueue.h: the header file that contains the API declaration.</li></ul>
 *@see mq_receive
 *@since Huawei LiteOS V100R001C00
 */
extern int mq_timedsend(mqd_t mqdes, const char *msg_ptr, size_t msg_len, unsigned int msg_prio, const struct timespec *abs_timeout);

/**
 *@ingroup mqueue
 *@brief Receive a message at a scheduled time.
 *
 *@par Description:
 *<ul>
 *<li>This API is used to obtain a message with specified message content and length from a message queue message that has a specified descriptor.</li>
 *</ul>
 *@attention
 *<ul>
 *<li> Priority-based message processing is not supported.</li>
 *<li> The expiry time must be later than the current time.</li>
 *<li> The wait time is a relative time.</li>
 *<li> The msg_len should be same to the length of string which msg_ptr point to.</li>
 *</ul>
 *
 *@param mqdes           [IN]Message queue descriptor.
 *@param msg_ptr         [IN]Pointer to the message content to be received.
 *@param msg_len         [IN]Length of the message to be received.
 *@param msg_prio        [IN]Priority of the message to be received (defaulted to 0 because priority-based processing is not supported).
 *@param abs_timeout     [IN]Scheduled time at which the messagewill be received. If the value is 0, the message is an instant message.
 *
 *@retval  0    The message is successfully received.
 *@retval -1    The message fails to be received, with any of the following error codes in errno:
                #EINTR      : An interrupt is in progress while the message is being received.
                #EBADF      : The message queue is invalid or not readable.
                #EAGAIN     : The message queue is empty.
                #EINVAL     : invalid parameter.
                #EOPNOTSUPP : The operation is not supported.
                #EMSGSIZE   : The message to be received is too long.
                #ETIMEDOUT  : The operation times out.
 *@par Dependency:
 *<ul><li>mqueue.h: the header file that contains the API declaration.</li></ul>
 *@see mq_send
 *@since Huawei LiteOS V100R001C00
 */
extern ssize_t mq_timedreceive(mqd_t mqdes, char *msg_ptr, size_t msg_len, unsigned int *msg_prio, const struct timespec *abs_timeout);


#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif
