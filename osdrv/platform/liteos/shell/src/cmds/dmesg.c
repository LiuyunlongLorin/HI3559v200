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

/*
   +-------------------------------------------------------+
   | Info |          log_space                             |
   +-------------------------------------------------------+
   |
   |__buffer_space

Case A:
   +-------------------------------------------------------+
   |           |#############################|             |
   +-------------------------------------------------------+
               |                             |
              Head                           Tail
Case B:
   +-------------------------------------------------------+
   |##########|                                    |#######|
   +-------------------------------------------------------+
              |                                    |
              Tail                                 Head
*/

#include "dmesg.ph"
#include "unistd.h"
#include "shcmd.h"
#include "stdlib.h"
#include "stdio.h"
#include "fcntl.h"
#include "los_task.h"
#include "semaphore.h"
#include "securec.h"
#include "show.h"
#ifdef LOSCFG_SHELL_DMESG
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#define  BUF_MAX_INDEX    (uwLogBufSize - 1)

extern CHAR uart_putc(CHAR c);
extern CHAR uart_fputc(CHAR c,VOID *f);
extern INT32 vfs_normalize_path(const CHAR *directory, const CHAR *filename, CHAR **pathname);

static DMESG_INFO *pstDmesgInfo = NULL;
static UINT32 uwLogBufSize = 0;
static VOID *pMallocAddr = NULL;
static UINT32 uwDmesgLogLevel = 3;
static UINT32 console_lock = 0;
static UINT32 uart_lock = 0;
static const CHAR *acLevelString[] = {
    "EMG",
    "COMMOM",
    "ERR",
    "WARN",
    "INFO",
    "DEBUG",
};

static VOID osLockConsole(VOID)
{
    console_lock = 1;
}

static VOID osUnlockConsole(VOID)
{
    console_lock = 0;
}

static VOID osLockUart(VOID)
{
    uart_lock = 1;
}

static VOID osUnlockUart(VOID)
{
    uart_lock = 0;
}

static UINT32 osCheckError(VOID)
{
    if (pstDmesgInfo->uwLogSize > uwLogBufSize)
    {
        goto ERROR_END;
    }

    if (((pstDmesgInfo->uwLogSize == uwLogBufSize) || (pstDmesgInfo->uwLogSize == 0))
        && (pstDmesgInfo->uwLogTail != pstDmesgInfo->uwLogHead))
    {
        goto ERROR_END;
    }

    return LOS_OK;

ERROR_END:
    return LOS_NOK;
}

static INT32 osCopyToNew(VOID *pAddr,UINT32 uwSize)
{
    INT32 copy_start = 0, copy_len = 0;
    CHAR *pTemp = NULL ,*pNewBuf = (CHAR*)pAddr + sizeof(DMESG_INFO);
    UINT32 buf_size = uwSize - sizeof(DMESG_INFO);

    if(pstDmesgInfo->uwLogSize == 0)
    {
        return 0;
    }

    if((pTemp = (CHAR*)malloc(pstDmesgInfo->uwLogSize)) == NULL)
    {
        return -1;
    }

    (VOID)memset_s(pTemp, pstDmesgInfo->uwLogSize,0,pstDmesgInfo->uwLogSize);
    copy_len = ((buf_size < pstDmesgInfo->uwLogSize) ? buf_size : pstDmesgInfo->uwLogSize);
    if(buf_size < pstDmesgInfo->uwLogSize)
    {
        copy_start = pstDmesgInfo->uwLogSize - buf_size ;
    }

    (VOID)LOS_DmesgRead(pTemp, pstDmesgInfo->uwLogSize);

    /*  if new buf size smaller than logSize  */
    (VOID)memcpy_s(pNewBuf, buf_size, pTemp + copy_start, copy_len);
    free(pTemp);

    return copy_len;
}

static UINT32 osDmesgResetMem(VOID *pAddr,UINT32 uwSize)
{
    VOID *pTemp = 0;
    INT32  copy_len = 0;

    pTemp = pstDmesgInfo;
    if(uwSize <= sizeof(DMESG_INFO) || (copy_len = osCopyToNew(pAddr, uwSize)) < 0)
    {
        return LOS_NOK;
    }

    uwLogBufSize = uwSize - sizeof(DMESG_INFO);
    pstDmesgInfo = (DMESG_INFO *)pAddr;
    pstDmesgInfo->pBuf = (CHAR *)pAddr + sizeof(DMESG_INFO);
    pstDmesgInfo->uwLogSize = copy_len;
    pstDmesgInfo->uwLogTail = ((copy_len == uwLogBufSize) ? 0 : copy_len);
    pstDmesgInfo->uwLogHead = 0;

    /*  if old mem came from malloc */
    if(pTemp == pMallocAddr)
    {
        free(pTemp);
    }

    return LOS_OK;
}
static UINT32 osDmesgChangeSize(UINT32 uwSize)
{
    VOID *pTemp = pstDmesgInfo;
    INT32  copy_len = 0;
    CHAR *pNew = NULL;
    if(uwSize == 0 || (pNew = (CHAR *)malloc(uwSize + sizeof(DMESG_INFO))) == NULL)
    {
        return LOS_NOK;
    }

    if((copy_len = osCopyToNew(pNew,uwSize + sizeof(DMESG_INFO))) < 0)
    {
        free(pNew);
        return LOS_NOK;
    }

    uwLogBufSize = uwSize;
    pstDmesgInfo = (DMESG_INFO *)pNew;
    pstDmesgInfo->pBuf = (CHAR *)pNew + sizeof(DMESG_INFO);
    pstDmesgInfo->uwLogSize = copy_len;
    pstDmesgInfo->uwLogTail = ((copy_len == uwLogBufSize) ? 0 : copy_len);
    pstDmesgInfo->uwLogHead = 0;

    if(pTemp == pMallocAddr)
    {
        free(pTemp);
    }
    pMallocAddr = pNew;

    return LOS_OK;
}

UINT32 osCheckConsoleLock(VOID)
{
    return console_lock;
}

UINT32 osCheckUartLock(VOID)
{
    return uart_lock;
}

UINT32 osDmesgInit(VOID)
{
    CHAR* pBuffer = NULL;
    pBuffer = (CHAR*)malloc(KERNEL_LOG_BUF_SIZE + sizeof(DMESG_INFO));
    if(pBuffer == NULL)
    {
        return LOS_NOK;
    }
    pMallocAddr = pBuffer;
    pstDmesgInfo = (DMESG_INFO *)pBuffer;
    pstDmesgInfo->uwLogHead = 0;
    pstDmesgInfo->uwLogTail = 0;
    pstDmesgInfo->uwLogSize = 0;
    pstDmesgInfo->pBuf = pBuffer + sizeof(DMESG_INFO);
    uwLogBufSize = KERNEL_LOG_BUF_SIZE;

    return LOS_OK;
}

CHAR osLogRecord(CHAR c,VOID *f)
{
    UINTPTR uvIntSave = LOS_IntLock();

    *(pstDmesgInfo->pBuf + pstDmesgInfo->uwLogTail++) = c;

    if(pstDmesgInfo->uwLogTail > BUF_MAX_INDEX)
    {
        pstDmesgInfo->uwLogTail = 0;
    }

    if(pstDmesgInfo->uwLogSize < uwLogBufSize)
    {
        (pstDmesgInfo->uwLogSize)++;
    }
    else
    {
        pstDmesgInfo->uwLogHead = pstDmesgInfo->uwLogTail;
    }
    (VOID)LOS_IntRestore(uvIntSave);
    return c;
}

static VOID osBufFullWrite(const CHAR *pDst, UINT32 uwLogLen)
{
    UINT32 uwBufSize = uwLogBufSize;
    UINT32 uwTail = pstDmesgInfo->uwLogTail;
    CHAR *pBuf = pstDmesgInfo->pBuf;

    if(!uwLogLen || pDst == NULL)
    {
        return ;
    }
    if(uwLogLen > uwBufSize)//full re-write
    {
        (VOID)memcpy_s(pBuf + uwTail, uwBufSize - uwTail, pDst, uwBufSize - uwTail);
        (VOID)memcpy_s(pBuf,uwBufSize, pDst + uwBufSize - uwTail, uwTail);
        osBufFullWrite(pDst + uwBufSize, uwLogLen - uwBufSize);
    }
    else
    {
        if(uwLogLen > uwBufSize - uwTail)//need cycle back to start
        {
            (VOID)memcpy_s(pBuf + uwTail, uwBufSize - uwTail, pDst, uwBufSize - uwTail);
            (VOID)memcpy_s(pBuf,uwBufSize, pDst + uwBufSize - uwTail, uwLogLen - (uwBufSize - uwTail));
            pstDmesgInfo->uwLogTail = uwLogLen - (uwBufSize - uwTail);
            pstDmesgInfo->uwLogHead = pstDmesgInfo->uwLogTail;
        }
        else//no need cycle back to start
        {
            (VOID)memcpy_s(pBuf + uwTail, uwBufSize - uwTail, pDst, uwLogLen);
            pstDmesgInfo->uwLogTail += uwLogLen;
            if(pstDmesgInfo->uwLogTail > BUF_MAX_INDEX)
            {
                pstDmesgInfo->uwLogTail = 0;
            }
            pstDmesgInfo->uwLogHead = pstDmesgInfo->uwLogTail;
        }
    }
}

static VOID osWriteTailToHead(const CHAR *pDst, UINT32 uwLogLen)
{
    UINT32 uwWriteLen = 0;
    UINT32 uwBufSize = uwLogBufSize;
    UINT32 uwLogSize = pstDmesgInfo->uwLogSize;
    UINT32 uwTail = pstDmesgInfo->uwLogTail;
    CHAR *pBuf = pstDmesgInfo->pBuf;

    if(!uwLogLen || pDst == NULL)
    {
        return ;
    }
    if(uwLogLen > uwBufSize - uwLogSize)//space-need > space-remain
    {
        uwWriteLen = uwBufSize - uwLogSize;
        (VOID)memcpy_s(pBuf + uwTail, uwBufSize - uwTail, pDst, uwWriteLen);
        pstDmesgInfo->uwLogTail = pstDmesgInfo->uwLogHead;
        pstDmesgInfo->uwLogSize = uwLogBufSize;
        osBufFullWrite(pDst + uwWriteLen, uwLogLen - uwWriteLen);
    }
    else
    {
        (VOID)memcpy_s(pBuf + uwTail, uwBufSize - uwTail, pDst, uwLogLen);
        pstDmesgInfo->uwLogTail += uwLogLen;
        pstDmesgInfo->uwLogSize += uwLogLen;
    }
}

static VOID osWriteTailToEnd(const CHAR *pDst, UINT32 uwLogLen)
{
    UINT32 uwWriteLen = 0;
    UINT32 uwBufSize = uwLogBufSize;
    UINT32 uwTail = pstDmesgInfo->uwLogTail;
    CHAR *pBuf = pstDmesgInfo->pBuf;

    if(!uwLogLen || pDst == NULL)
    {
        return ;
    }
    if(uwLogLen >= uwBufSize - uwTail)//need cycle to start ,then became B
    {
        uwWriteLen = uwBufSize - uwTail;
        (VOID)memcpy_s(pBuf + uwTail, uwWriteLen, pDst, uwWriteLen);
        pstDmesgInfo->uwLogSize += uwWriteLen;
        pstDmesgInfo->uwLogTail = 0;
        if(pstDmesgInfo->uwLogSize == uwLogBufSize)//Tail == Head == 0
        {
            osBufFullWrite(pDst + uwWriteLen, uwLogLen - uwWriteLen);
        }
        else
        {
            osWriteTailToHead(pDst + uwWriteLen, uwLogLen - uwWriteLen);
        }
    }
    else//just do serial copy
    {
        (VOID)memcpy_s(pBuf + uwTail, uwBufSize - uwTail, pDst,uwLogLen);
        pstDmesgInfo->uwLogTail += uwLogLen;
        pstDmesgInfo->uwLogSize += uwLogLen;
    }
}


INT32 osLogMemcpyRecord(const CHAR *pBuf, UINT32 uwLogLen)
{
    UINTPTR uvIntSave = LOS_IntLock();

    if(osCheckError())
    {
        (VOID)LOS_IntRestore(uvIntSave);
        return -1;
    }

    if(pstDmesgInfo->uwLogSize < uwLogBufSize)
     {
        if(pstDmesgInfo->uwLogHead <= pstDmesgInfo->uwLogTail)
        {
            osWriteTailToEnd(pBuf, uwLogLen);
        }
        else
        {
            osWriteTailToHead(pBuf, uwLogLen);
        }
    }
    else
    {
        osBufFullWrite(pBuf, uwLogLen);
    }

    (VOID)LOS_IntRestore(uvIntSave);

    return LOS_OK;
}

VOID osLogShow(VOID)
{
    UINTPTR uvIntSave = LOS_IntLock();

    UINT32 uwIndex = pstDmesgInfo->uwLogHead;
    UINT32 i = 0;
    while(i < pstDmesgInfo->uwLogSize)
    {
        (void)uart_fputc(*(pstDmesgInfo->pBuf + uwIndex++), NULL);
        if(uwIndex > BUF_MAX_INDEX)
        {
            uwIndex = 0;
        }
        if(uwIndex == pstDmesgInfo->uwLogTail)
        {
            break;
        }
        i++;
    }
    (VOID)LOS_IntRestore(uvIntSave);
}

static INT32 osDmesgLvSet(CHAR *pLevel)
{
    UINT32 level = 0,uwRet = 0;
    CHAR *p;
    level = strtoul(pLevel, &p, 0);
    if(*p != 0)
    {
        PRINTK("dmesg: invalid option or parameter.\n");
        return -1;
    }

    uwRet = LOS_DmesgLvSet(level);

    if(uwRet == LOS_OK)
    {
        PRINTK("Set current dmesg log level %s\n", acLevelString[uwDmesgLogLevel]);
        return LOS_OK;
    }
    else
    {
        PRINTK("current dmesg log level %s\n", acLevelString[uwDmesgLogLevel]);
        PRINTK("dmesg -l [num] can access as 0:EMG 1:COMMOM 2:ERROR 3:WARN 4:INFO 5:DEBUG\n");
        return -1;
    }
}

static UINT32 osDmesgMemSizeSet(CHAR *pSize)
{
    UINT32 uwSize = 0;
    CHAR *p = NULL;
    uwSize = strtoul(pSize, &p, 0);
    if(!(LOS_DmesgMemSet(NULL, uwSize)))
    {
        PRINTK("Set dmesg buf size %u success\n", uwSize);
        return LOS_OK;
    }
    else
    {
        PRINTK("Set dmesg buf size %u fail\n", uwSize);
        return LOS_NOK;
    }
}
UINT32 osDmesgLvGet(VOID)
{
    return uwDmesgLogLevel;
}

UINT32 LOS_DmesgLvSet(UINT32 uwLevel)
{
    if(uwLevel > 5)
    {
        return LOS_NOK;
    }

    uwDmesgLogLevel = uwLevel;
    return LOS_OK;

}

VOID LOS_DmesgClear(VOID)
{
    UINTPTR uvIntSave = LOS_IntLock();
    (VOID)memset_s(pstDmesgInfo->pBuf, uwLogBufSize, 0, uwLogBufSize);
    pstDmesgInfo->uwLogHead = 0;
    pstDmesgInfo->uwLogTail = 0;
    pstDmesgInfo->uwLogSize = 0;
    (VOID)LOS_IntRestore(uvIntSave);
}

UINT32 LOS_DmesgMemSet(VOID *pAddr, UINT32 uwSize)
{
    UINT32 uwRet = 0;
    UINTPTR uvIntSave = LOS_IntLock();
    if(pAddr == NULL)
    {
        uwRet = osDmesgChangeSize(uwSize);
    }
    else
    {
        uwRet = osDmesgResetMem(pAddr, uwSize);
    }
    (VOID)LOS_IntRestore(uvIntSave);
    return uwRet;
}

INT32 LOS_DmesgRead(CHAR *pBuf, UINT32 uwLen)
{
    UINTPTR uvIntSave = LOS_IntLock();
    UINT32 uwReadLen = 0;
    UINT32 uwLogSize = pstDmesgInfo->uwLogSize;
    UINT32 uwHead = pstDmesgInfo->uwLogHead;
    UINT32 uwTail = pstDmesgInfo->uwLogTail;
    CHAR *pLogBuf = pstDmesgInfo->pBuf;

    if(pBuf == NULL || osCheckError())
    {
        (VOID)LOS_IntRestore(uvIntSave);
        return -1;
    }

    if(uwLogSize == 0 || uwLen == 0)
    {
        (VOID)LOS_IntRestore(uvIntSave);
        return 0;
    }

    uwReadLen = (uwLen < uwLogSize ? uwLen : uwLogSize);

    if(uwHead < uwTail)//Case A
    {
        (VOID)memcpy_s(pBuf,uwLen, pLogBuf + uwHead, uwReadLen);
        pstDmesgInfo->uwLogHead += uwReadLen;
        pstDmesgInfo->uwLogSize -= uwReadLen;
    }
    else//Case B
    {
        if(uwReadLen < (uwLogBufSize - uwHead))
        {
            (VOID)memcpy_s(pBuf,uwLen, pLogBuf + uwHead, uwReadLen);
            pstDmesgInfo->uwLogHead += uwReadLen;
            pstDmesgInfo->uwLogSize -= uwReadLen;
        }
        else{
            (VOID)memcpy_s(pBuf,uwLen, pLogBuf + uwHead, uwLogBufSize - uwHead);
            (VOID)memcpy_s(pBuf + uwLogBufSize - uwHead, uwLen - (uwLogBufSize - uwHead),\
                pLogBuf,uwReadLen - (uwLogBufSize - uwHead));
            pstDmesgInfo->uwLogHead = uwReadLen - (uwLogBufSize - uwHead);
            pstDmesgInfo->uwLogSize -= uwReadLen;
        }
    }
    (VOID)LOS_IntRestore(uvIntSave);
    return (INT32)uwReadLen;
}

INT32 LOS_DmesgToFile(CHAR* pFilename)
{
#ifdef LOSCFG_FS_VFS
    CHAR  *fullpath = NULL, *pBuf = NULL;
    INT32 fd = 0,swRet = 0;
    char * shell_working_directory = NULL;

    UINTPTR uvIntSave = LOS_IntLock();
    UINT32 uwLogSize = pstDmesgInfo->uwLogSize;
    UINT32 uwBufSize = uwLogBufSize;
    UINT32 uwHead = pstDmesgInfo->uwLogHead;
    UINT32 uwTail = pstDmesgInfo->uwLogTail;
    CHAR *pLogBuf = pstDmesgInfo->pBuf;
    (VOID)LOS_IntRestore(uvIntSave);

    shell_working_directory = osShellGetWorkingDirtectory();

    if(osCheckError())
    {
        goto ERR_OUT1;
    }

    if(vfs_normalize_path(shell_working_directory, pFilename, &fullpath))
    {
        goto ERR_OUT1;
    }

    if((fd = open(fullpath, O_CREAT|O_RDWR|O_APPEND, 0644)) < 0){
        goto ERR_OUT2;
    }

    if ((0 == uwLogSize))
    {
        free(fullpath);
        close(fd);
        return 0;
    }

    if((pBuf = (CHAR *)malloc(uwLogSize)) == NULL)
    {
        goto ERR_OUT3;
    }

    if(uwHead < uwTail)
    {
        (VOID)memcpy_s(pBuf,uwLogSize, pLogBuf + uwHead, uwLogSize);
    }
    else
    {
        (VOID)memcpy_s(pBuf,uwLogSize, pLogBuf + uwHead, uwBufSize - uwHead);
        (VOID)memcpy_s(pBuf + uwBufSize - uwHead, uwLogSize - (uwBufSize - uwHead), pLogBuf, uwTail);
    }

    if((swRet = write(fd, pBuf, uwLogSize)) <= 0){
        goto ERR_OUT4;
    }

    free(fullpath);
    free(pBuf);
    close(fd);

    return swRet;

ERR_OUT4:
    free(pBuf);
ERR_OUT3:
    close(fd);

ERR_OUT2:
    free(fullpath);
ERR_OUT1:
    return -1;
#else

    PRINTK("File operation need VFS\n");
    return -1;
#endif
}

INT32 osShellCmdDmesg(INT32 argc, CHAR **argv)
{
    if(argc == 1){
        PRINTK("\n");
        osLogShow();
        return LOS_OK;
    }
    else if(argc == 2){
        if(!(strcmp(argv[1], "-c")))
        {
            PRINTK("\n");
            osLogShow();
            LOS_DmesgClear();
            return LOS_OK;
        }
        else if(!(strcmp(argv[1], "-C")))
        {
            LOS_DmesgClear();
            return LOS_OK;
        }
        else if(!(strcmp(argv[1], "-D")))
        {
            osLockConsole();
            return LOS_OK;
        }
        else if(!(strcmp(argv[1], "-E")))
        {
            osUnlockConsole();
            return LOS_OK;
        }
        else if(!(strcmp(argv[1], "-L")))
        {
            osLockUart();
            return LOS_OK;
        }
        else if(!(strcmp(argv[1], "-U")))
        {
            osUnlockUart();
            return LOS_OK;
        }
    }
    else if(argc == 3)
    {
        if(!(strcmp(argv[1], ">")))
        {
            if(LOS_DmesgToFile(argv[2]) < 0)
            {
                PRINTK("Dmesg write log to %s fail \n", argv[2]);
                return -1;
            }
            else
            {
                PRINTK("Dmesg write log to %s success \n", argv[2]);
                return LOS_OK;
            }
        }
        else if(!(strcmp(argv[1], "-l")))
        {
            return osDmesgLvSet(argv[2]);
        }
        else if(!(strcmp(argv[1], "-s")))
        {
            return osDmesgMemSizeSet(argv[2]);
        }
    }

    PRINTK("dmesg: invalid option or parameter.\n");
    return -1;
}

SHELLCMD_ENTRY(dmesg_shellcmd, CMD_TYPE_STD, "dmesg", 1, (CMD_CBK_FUNC)osShellCmdDmesg); /*lint !e19*/

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
#endif
