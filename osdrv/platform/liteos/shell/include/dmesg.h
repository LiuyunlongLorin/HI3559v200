/*----------------------------------------------------------------------------
 * Copyright (c) <2013-2017>, <Huawei Technologies Co., Ltd>
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

#ifndef _HWLITEOS_SHELL_DMESG_H__
#define _HWLITEOS_SHELL_DMESG_H__
#include "los_config.h"
#ifdef LOSCFG_SHELL_DMESG

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/**
 * @ingroup los_dmesg
 * Defalut dmesg buffer size.
 */
#define    KERNEL_LOG_BUF_SIZE    (8*1024)

/**
 * @ingroup los_dmesg
 * Information of dmesg buffer.
 *
 * The dmesg buffer is start with this info structure, then the log.
 */
typedef struct dmesg_info {
    UINT32 uwLogSize;   /**< The size of log in buffer               */
    UINT32 uwLogHead;  /**< The index of the first log data. Data_out_flag               */
    UINT32 uwLogTail;  /**< The index where to write, write in and plus one. Data_it_flag               */
    CHAR   *pBuf;   /**< The log buffer addr               */
}DMESG_INFO;

/**
 * @ingroup  los_dmesg
 * @brief Set dmesg buffer.
 *
 * @par Description:
 * This API is used to set dmesg buffer to memory-assigned, or just change the buffer size.
 *
 * @attention
 * <ul>
 * <li>If the pAddr is NULL, this function will only change the buffer size.</li>
 * </ul>
 *
 * @param  pAddr    [IN] Type #VOID* The addr of memory space to set.
 * @param  uwSize   [IN] Type #UINT32 The dmesg buffer size to set, depend on the app user, should be a valid value, otherwise system will crash.
 *
 * @retval #LOS_NOK                      Set dmesg buffer fail.
 * @retval #LOS_OK                       Set dmesg buffer success.
 * @par Dependency:
 * <ul><li>dmesg.h: the header file that contains the API declaration.</li></ul>
 * @see LOS_DmesgMemSet
 * @since Huawei LiteOS V200R001C10
 */
extern UINT32 LOS_DmesgMemSet(VOID *pAddr,UINT32 uwSize);

 /**
 * @ingroup  los_dmesg
 * @brief Read log from dmesg buffer.
 *
 * @par Description:
 * This API is used to get log from dmesg buffer in core and copy to the point buffer.
 *
 * @attention
 * <ul>
 * <li>Length of log been read may be less than the uwLen if log in dmesg buffer is not enough.</li>
 * </ul>
 *
 * @param  pBuf     [IN] Type #CHAR* The buffer expected copy to.
 * @param  uwLen    [IN] Type #UINT32 The maximum number of bytes that can be accommodated in the pBuf.
 *
 * @retval #-1                      Read log from dmesg buffer fail.
 * @retval #0                       Nothing has been read.
 * @retval #INT32                   The length of log has been read.
 * @par Dependency:
 * <ul><li>dmesg.h: the header file that contains the API declaration.</li></ul>
 * @see LOS_DmesgRead
 * @since Huawei LiteOS V200R001C10
 */
extern INT32 LOS_DmesgRead(CHAR *pBuf, UINT32 uwLen);

 /**
 * @ingroup  los_dmesg
 * @brief Clear dmesg log.
 *
 * @par Description:
 * This API is used to clear dmesg log.
 *
 * @attention None.
 *
 * @param  None.
 *
 * @retval None.
 * @par Dependency:
 * <ul><li>dmesg.h: the header file that contains the API declaration.</li></ul>
 * @see LOS_DmesgClear
 * @since Huawei LiteOS V200R001C10
 */
extern VOID LOS_DmesgClear(VOID);

 /**
 * @ingroup  los_dmesg
 * @brief Copy log to file.
 *
 * @par Description:
 * This API is used to copy all log from dmesg buffer and write it to the file.
 *
 * @attention
 * <ul>
 * <li>Files rely on file system, the file system of pFilename dependent must be already mounted.</li>
 * </ul>
 *
 * @param  pFilename   [IN] Type #CHAR* The buffer expected copy to.
 *
 * @retval #-1                      Copy log to file fail.
 * @retval #0                       Maybe there is no log in the buffer.
 * @retval #INT32                   The length of log has been written to file.
 * @par Dependency:
 * <ul><li>dmesg.h: the header file that contains the API declaration.</li></ul>
 * @see LOS_DmesgToFile
 * @since Huawei LiteOS V200R001C10
 */
extern INT32 LOS_DmesgToFile(CHAR *pFilename);

 /**
 * @ingroup  los_dmesg
 * @brief Set the dmesg level
 *
 * @par Description:
 * This API is used to set the level of log that want to stored in dmesg buffer.
 *
 * @attention
 * <ul>
 * <li>It would be useless if the level is less than print level.</li>
 * </ul>
 *
 * @param  uwLevel   [IN] Type #UINT32 The level expected to set, range from 0 to 5.
 *
 * @retval #1                       Set dmesg level fail.
 * @retval #0                       Set dmesg level success.
 * @par Dependency:
 * <ul><li>dmesg.h: the header file that contains the API declaration.</li></ul>
 * @see LOS_DmesgLvSet
 * @since Huawei LiteOS V200R001C10
 */
extern UINT32 LOS_DmesgLvSet(UINT32 uwLevel);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
#endif
#endif    //_HWLITEOS_SHELL_DMESG_H__
