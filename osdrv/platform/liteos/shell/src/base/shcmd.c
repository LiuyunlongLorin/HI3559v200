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


#include "shcmd.h"
#include "los_mux.h"
#include "los_memory.h"
#include "hisoc/uart.h"
#include "stdlib.h"
#include "unistd.h"
#include "dirent.h"
#include "securec.h"
#include "show.h"

#ifdef  __cplusplus
#if  __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#define SHELL_INIT_MAGIC_FLAG       0xABABABAB

extern CMD_ITEM g_shellcmd[];
LOS_HAL_TABLE_BEGIN( g_shellcmd, shellcmd ); /*lint !e19*/

extern CMD_ITEM g_shellcmd_end;
LOS_HAL_TABLE_END( g_shellcmd_end, shellcmd ); /*lint !e19*/


CMD_MOD_INFO_S m_stCmdInfo;

static VOID osFreeCmdPara(CMD_PARSED_S *pstCmdParsed)
{
    UINT32 i = 0;
    for(i = 0; i < pstCmdParsed->uwParCnt; i++)
    {
        if((CHAR*)(pstCmdParsed->stParamArray[i]) != NULL)
        {
            (VOID)LOS_MemFree(m_aucSysMem0, (CHAR*)(pstCmdParsed->stParamArray[i]));
            pstCmdParsed->stParamArray[i] = (AARCHPTR)NULL;
        }
    }
}

static INT32 osStrSeparate (CHAR *pcTabStr, CHAR *pcStrPath, CHAR *pcNameLooking,UINT32 puwLen)
{
    CHAR *pcStrEnd = NULL;
    CHAR *pcCutPos = NULL;
    CHAR *pcTempStr = NULL;
    CHAR *pcShiftStr = NULL;
    CMD_PARSED_S pstParsed = {0};
    char * shell_working_directory = osShellGetWorkingDirtectory();


    if((pcTempStr = (CHAR *)LOS_MemAlloc(m_aucSysMem0, SHOW_MAX_LEN * 2)) == NULL)
    {
        return (INT32)OS_ERROR;
    }

    (VOID)memset_s(pcTempStr, SHOW_MAX_LEN * 2, 0, SHOW_MAX_LEN * 2);
    pcShiftStr = pcTempStr + SHOW_MAX_LEN;

    if(strncpy_s(pcTempStr,SHOW_MAX_LEN - 1,pcTabStr,puwLen))
    {
        (VOID)LOS_MemFree(m_aucSysMem0, pcTempStr);
        return (INT32)OS_ERROR;
    }

    pstParsed.enCmdType = CMD_TYPE_STD;

    /*cut useless or repeat space*/
    if(osCmdKeyShift(pcTempStr,pcShiftStr,SHOW_MAX_LEN - 1))
    {
        (VOID)LOS_MemFree(m_aucSysMem0, pcTempStr);
        return (INT32)OS_ERROR;
    }

    /* get exact position of str to complete */
    /* situation different if end space lost or still exist */
    if(strlen(pcShiftStr) == 0)
    {
        pcTabStr = "";
    }
    else if(pcTempStr[strlen(pcTempStr) - 1] != pcShiftStr[strlen(pcShiftStr) - 1])
    {
        pcTabStr = "";
    }
    else
    {
        if(osCmdTokenSplit(pcShiftStr,' ',&pstParsed))
        {
            (VOID)LOS_MemFree(m_aucSysMem0, pcTempStr);
            return (INT32)OS_ERROR;
        }
        pcTabStr = (CHAR*)pstParsed.stParamArray[pstParsed.uwParCnt - 1];
    }

    (VOID)LOS_MemFree(m_aucSysMem0, pcTempStr);

    /* get fullpath str */
    if(*pcTabStr != '/')
    {
        if(strncpy_s(pcStrPath,CMD_MAX_PATH - 1,shell_working_directory,CMD_MAX_PATH - 1))
        {
            osFreeCmdPara(&pstParsed);
            return (INT32)OS_ERROR;
        }
        if(strcmp(shell_working_directory,"/"))
        {
            if(strncat_s(pcStrPath,CMD_MAX_PATH - 1,"/",CMD_MAX_PATH - strlen(pcStrPath) - 1))
            {
                osFreeCmdPara(&pstParsed);
                return (INT32)OS_ERROR;
            }
        }
    }

    if(strncat_s(pcStrPath,CMD_MAX_PATH - 1,pcTabStr,CMD_MAX_PATH - strlen(pcStrPath) - 1))
    {
        osFreeCmdPara(&pstParsed);
        return (INT32)OS_ERROR;
    }

    /* split str by last '/' */
    if(NULL != (pcStrEnd = strrchr(pcStrPath,'/')))
    {
        if(strncpy_s(pcNameLooking,CMD_MAX_PATH - 1,pcStrEnd+1,CMD_MAX_PATH - 1))//get cmp str
        {
            osFreeCmdPara(&pstParsed);
            return (INT32)OS_ERROR;
        }
    }

    pcCutPos = pcStrPath + strlen(pcStrPath);//get path
    if(NULL != (pcCutPos = strrchr(pcStrPath,'/')))
    {
        *(pcCutPos + 1) = '\0';
    }

    osFreeCmdPara(&pstParsed);
    return LOS_OK;
}

static INT32 osShowPageControl(UINT32 uwTimesPrint, UINT32 uwLineCap, UINT32 uwCount)
{
    CHAR cReadCh = 0;

    if(NEED_NEW_LINE(uwTimesPrint,uwLineCap))
    {
        PRINTK("\n");
        if(SCREEN_IS_FULL(uwTimesPrint,uwLineCap) && (uwTimesPrint < uwCount))
        {
            PRINTK("--More--");
            while(1){
                if(read(0, &cReadCh, 1) != 1)
                {
                    PRINTK("\n");
                    return (INT32)OS_ERROR;
                }
                if(cReadCh == 'q' || cReadCh == 'Q' || cReadCh == 0x03)//ctrl+c
                {
                    PRINTK("\n");
                    return 0;
                }
                else if(cReadCh == '\r')
                {
                    PRINTK("\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b");
                    return 1;
                }
            }
        }
    }

    return 1;
}

static INT32 osSurePrintAll(UINT32 uwCount)
{
    CHAR cReadCh = 0;
    PRINTK("\nDisplay all %d possibilities?(y/n)",uwCount);
    while(1){
        if(read(0, &cReadCh, 1) != 1)
        {
            return (INT32)OS_ERROR;
        }
        if(cReadCh == 'n' || cReadCh == 'N' || cReadCh == 0x03)
        {
            PRINTK("\n");
            return 0;
        }
        else if(cReadCh == 'y' || cReadCh == 'Y' || cReadCh == '\r')
        {
            return 1;
        }
    }
}

static INT32 osPrintMatchList (UINT32 uwCount,CHAR *pcStrPath, CHAR *pcNameLooking, UINT32 uwPrintLen)
{
    UINT32 uwTimesPrint = 0, uwLineCap = 0;
    INT32 swRet = 0;
    DIR *pstOpendir = NULL;
    struct dirent *pstReaddir = NULL;
    CHAR acFormat[10] = {0};

    uwPrintLen = (uwPrintLen > (DEFAULT_SCREEN_WIDTH - 2)) ? (DEFAULT_SCREEN_WIDTH - 2) : uwPrintLen;
    uwLineCap = DEFAULT_SCREEN_WIDTH / (uwPrintLen + 2);
    if(snprintf_s(acFormat,sizeof(acFormat)-1,7/*format-len*/,"%%-%ds  ",uwPrintLen) < 0)
    {
        return (INT32)OS_ERROR;
    }

    if(uwCount > (uwLineCap * DEFAULT_SCREEN_HEIGNT))
    {
        if((swRet = osSurePrintAll(uwCount)) != 1)
        {
            return swRet;
        }
    }

    if(NULL == (pstOpendir = opendir(pcStrPath)))
    {
        return (INT32)OS_ERROR;
    }

    PRINTK("\n");
    while((pstReaddir = readdir(pstOpendir)) != NULL)
    {
        if (strncmp(pcNameLooking, pstReaddir->d_name, strlen(pcNameLooking)) == 0)
        {
            PRINTK(acFormat,pstReaddir->d_name);
            uwTimesPrint++;

            if((swRet = osShowPageControl(uwTimesPrint, uwLineCap, uwCount)) != 1)
            {
                if(closedir(pstOpendir) < 0)
                {
                    return (INT32)OS_ERROR;
                }
                return swRet;
            }
        }
    }

    PRINTK("\n");
    if(closedir(pstOpendir) < 0)
    {
        return (INT32)OS_ERROR;
    }

    return LOS_OK;
}

static void strncmp_cut(const char *s1, char *s2, size_t n)
{
    if (n == 0 || s1 == NULL || s2 == NULL)
        return;
    do {
        if (*s1 && *s2 && *s1 == *s2)
        {
            s1++;
            s2++;
        }
        else
        {
            break;
        }
    } while (--n != 0);
    if (n > 0) {
        /* NUL pad the remaining n-1 bytes */
        while (n-- != 0)
            *s2++ = 0;
    }
    return;
}

static INT32 osExecNameMatch (CHAR *pcStrPath, CHAR *pcNameLooking, CHAR *pcStrObj, UINT32 *puwMaxLen)
{
    INT32  swCount = 0;
    DIR *pstOpendir = NULL;
    struct dirent *pstReaddir = NULL;

    if(NULL == (pstOpendir = opendir(pcStrPath)))
    {
        return (INT32)OS_ERROR;
    }


    while((pstReaddir = readdir(pstOpendir))!=NULL)
    {
        if (strncmp(pcNameLooking, pstReaddir->d_name, strlen(pcNameLooking)) == 0){
            if(swCount == 0){
                if(strncpy_s(pcStrObj,CMD_MAX_PATH - 1,pstReaddir->d_name,CMD_MAX_PATH - 1))
                {
                        (void)closedir(pstOpendir);
                        return (INT32)OS_ERROR;
                }
                *puwMaxLen = strlen(pstReaddir->d_name);
            }
            else
            {
                /* strncmp&cut the same str of name matched */
                strncmp_cut(pstReaddir->d_name,pcStrObj,strlen(pcStrObj));
                if(strlen(pstReaddir->d_name) > *puwMaxLen)
                {
                    *puwMaxLen = strlen(pstReaddir->d_name);
                }
            }
            swCount++;
        }
    }

    if(closedir(pstOpendir) < 0)
    {
        return (INT32)OS_ERROR;
    }

    return swCount;
}

static VOID osCompleteStr(CHAR *pcResult,CHAR *pcTarget,CHAR *pscCmdKey, UINT32 *puwLen)
{
    UINT32 uwSize = 0;
    CHAR *pcDes = NULL;
    CHAR *pcSrc = NULL;
    uwSize = strlen(pcResult) - strlen(pcTarget);
    pcDes = pscCmdKey + *puwLen;
    pcSrc = pcResult + strlen(pcTarget);
    while (uwSize-- > 0)
    {
        PRINTK("%c",*pcSrc);
        if(*puwLen == SHOW_MAX_LEN - 1)
        {
            *pcDes = '\0';
             break;
        }
        *pcDes++ = *pcSrc++;
        (*puwLen)++;
    }
}

static INT32 osTabMatchCmd (CHAR *pscCmdKey, UINT32 *puwLen)
{
    INT32  swCount = 0,swRet = 0;
    CMD_ITEM_S    *pstCmdItemGuard = (CMD_ITEM_S *)NULL;
    CMD_ITEM_S    *pstCurCmdItem = (CMD_ITEM_S *)NULL;
    CHAR *psCmdMajor = pscCmdKey;

    while(*psCmdMajor == 0x20)//cut left space
    {
        psCmdMajor++;
    }

    if (LOS_ListEmpty(&(m_stCmdInfo.stCmdList.list)))
        return (INT32)OS_ERROR;

    LOS_DL_LIST_FOR_EACH_ENTRY(pstCurCmdItem,&(m_stCmdInfo.stCmdList.list), CMD_ITEM_S, list)
    {
        if (strncmp(psCmdMajor, pstCurCmdItem->pstCmd->pscCmdKey, strlen(psCmdMajor)) > 0)
        {
            continue;
        }

        if (strncmp(psCmdMajor, pstCurCmdItem->pstCmd->pscCmdKey, strlen(psCmdMajor)) != 0)
        {
            break;
        }

        if (swCount == 0)
        {
           pstCmdItemGuard = pstCurCmdItem;
        }
        ++swCount;
    }

    if(pstCmdItemGuard == NULL)
    {
        return 0;
    }

    if (swCount == 1)
    {
        osCompleteStr(pstCmdItemGuard->pstCmd->pscCmdKey,psCmdMajor,pscCmdKey,puwLen);
    }

    swRet = swCount;

    if(swCount > 1){
        PRINTK("\n");
        while (swCount--)
        {
            PRINTK("%s  ",pstCmdItemGuard->pstCmd->pscCmdKey); /*lint !e613*/
            pstCmdItemGuard = LOS_DL_LIST_ENTRY(pstCmdItemGuard->list.pstNext,CMD_ITEM_S,list); /*lint !e613*/
        }
        PRINTK("\n");
    }

    return swRet;
}
static INT32 osTabMatchFile (CHAR *pscCmdKey, UINT32 *puwLen)
{
    UINT32 uwMaxLen = 0;
    INT32 swCount = 0;
    CHAR *pcDirOpen = NULL;
    CHAR *pcStrOutput = NULL;
    CHAR *pcStrCmp = NULL;

    if((pcDirOpen = (CHAR *)LOS_MemAlloc(m_aucSysMem0, CMD_MAX_PATH * 3)) == NULL)
    {
        return (INT32)OS_ERROR;
    }

    (VOID)memset_s(pcDirOpen, CMD_MAX_PATH * 3, 0, CMD_MAX_PATH * 3);
    pcStrOutput = pcDirOpen + CMD_MAX_PATH;
    pcStrCmp = pcStrOutput + CMD_MAX_PATH;

    if(osStrSeparate(pscCmdKey, pcDirOpen, pcStrCmp,*puwLen))
    {
        (VOID)LOS_MemFree(m_aucSysMem0, pcDirOpen);
        return (INT32)OS_ERROR;
    }

    swCount = osExecNameMatch (pcDirOpen, pcStrCmp,pcStrOutput,&uwMaxLen);

    /* one or more matched */
    if(swCount >= 1)
    {
        osCompleteStr(pcStrOutput,pcStrCmp,pscCmdKey,puwLen);

        if(swCount == 1)
        {
            (VOID)LOS_MemFree(m_aucSysMem0, pcDirOpen);
            return 1;
        }
        if(-1 == osPrintMatchList(swCount,pcDirOpen, pcStrCmp,uwMaxLen))
        {
            (VOID)LOS_MemFree(m_aucSysMem0, pcDirOpen);
            return (INT32)OS_ERROR;
        }
    }

    (VOID)LOS_MemFree(m_aucSysMem0, pcDirOpen);
    return swCount;
}

/***************************************************************************************
 Function:    osCmdKeyShift
 Description: Pass in the string and clear useless space ,which inlcude:
                  1) The overmatch space which is not be marked by Quote's area
                     Squeeze the overmatch space into one space
                  2) Clear all space before first vaild charatctor
 Input:       pscCmdKey : Pass in the buff string, which is ready to be operated
              pscCmdOut : Pass out the buffer string ,which has already been operated
***************************************************************************************/
LITE_OS_SEC_TEXT_MINOR UINT32 osCmdKeyShift(CHAR *pscCmdKey, CHAR *pscCmdOut, UINT32 uwSize)
{
    CHAR    *pscOutput = (CHAR *)NULL;
    CHAR    *pscOutBak = (CHAR *)NULL;
    UINT32  uwLen = 0;
    BOOL    uwQuotes = FALSE;

    if (NULL == pscCmdKey || NULL == pscCmdOut)
    {
        return (UINT32)OS_ERROR;
    }

    uwLen = strlen(pscCmdKey);
    if (uwLen >= uwSize)
    {
        return (UINT32)OS_ERROR;
    }
    pscOutput = (CHAR*)LOS_MemAlloc(m_aucSysMem0, uwLen + 1);
    if (pscOutput == NULL)
    {
        PRINTK("malloc failure in %s[%d]", __FUNCTION__, __LINE__);
        return (UINT32)OS_ERROR;
    }
    /* Backup the 'pscOutput' start address */
    pscOutBak = pscOutput;
    /* Scan each charactor in 'pscCmdKey',and squeeze the overmuch space and ignore invaild charactor */
    for(;*pscCmdKey!='\0';pscCmdKey++)
    {
        /* Detected a Double Quotes, switch the matching status */
        if( *(pscCmdKey) == '\"')
        {
            SWITCH_QUOTES_STATUS(uwQuotes);
        }
        /* Ignore the current charactor in following situation */
        /* 1) Quotes matching status is FALSE (which said that the space is not been marked by double quotes) */
        /* 2) Current charactor is a space */
        /* 3) Next charactor is a space too, or the string is been seeked to the end already(\0) */
        /* 4) Invaild charactor, such as single quotes */
        if( *(pscCmdKey) == ' ' && ( *(pscCmdKey+1) == ' ' ||*(pscCmdKey+1) == '\0' ) && QUOTES_STATUS_CLOSE(uwQuotes) )
            continue;
        if( *(pscCmdKey) == '\'')
            continue;
        *pscOutput = *pscCmdKey;
        pscOutput++;
    }
    *pscOutput = '\0';
    /* Restore the 'pscOutput' start address */
    pscOutput = pscOutBak;
    uwLen = strlen(pscOutput);
    /* Clear the space which is located at the first charactor in buffer */
    if(*pscOutBak == ' ')
    {
        pscOutput++;
        uwLen--;
    }
    /* Copy out the buffer which is been operated already */

    (VOID)strncpy_s(pscCmdOut, uwSize, pscOutput, uwLen);
    pscCmdOut[uwLen] = '\0';

    (VOID)LOS_MemFree(m_aucSysMem0, pscOutBak);

    return LOS_OK;
}


/*****************************************************************************
 Function    : osCmdKeyCheck
 Description :  checking the input string is legal
 * (1) Keywords only contained 0-9,'a'-'z','A'-'Z','_','-'
 * (4) Keywords field can not be all numbers
 Input       : pscCmdKey
 Output      : None
 Return      : TRUE or FALSE
 *****************************************************************************/
LITE_OS_SEC_TEXT_MINOR BOOL osCmdKeyCheck(CHAR * pscCmdKey)
{
    CHAR *pscTemp = pscCmdKey;
    enum _STAT
    {
        STAT_NONE,
        STAT_DIGIT,
        STAT_OTHER
    }state = STAT_NONE;

    if (strlen(pscCmdKey) >= CMD_KEY_LEN)
    {
        return FALSE;
    }

    while (*pscTemp != '\0')
    {
        if (!(('0' <= *pscTemp) && ('9' >= *pscTemp)) &&
            !(('a' <= *pscTemp) && ('z' >= *pscTemp)) &&
            !(('A' <= *pscTemp) && ('Z' >= *pscTemp)) &&
            (!('_' == *pscTemp)) && (!('-' == *pscTemp)))
        {
            return FALSE;
        }

        if ((*pscTemp) >= '0' && (*pscTemp) <= '9')
        {
            if (STAT_NONE == state)
                state = STAT_DIGIT;
        }
        else
        {
            state = STAT_OTHER;
        }

        pscTemp++;
    }

    if (STAT_DIGIT == state)
    {
        return FALSE;
    }

    return TRUE;
}


LITE_OS_SEC_TEXT_MINOR INT32 osTabCompletion(CHAR *pscCmdKey, UINT32 *puwLen)
{
    INT32 swCount = 0;
    char *pcSpace = NULL;
    char *pcCmdMainStr = pscCmdKey;

    if(pscCmdKey == NULL || puwLen == NULL)
    {
        return (INT32)OS_ERROR;
    }

    /* cut left space */
    while(*pcCmdMainStr == 0x20)
    {
        pcCmdMainStr++;
    }

    /* try to find space in remain */
    pcSpace = strrchr(pcCmdMainStr,0x20);

    if(NULL == pcSpace && *pcCmdMainStr != '\0'){
        swCount = osTabMatchCmd(pscCmdKey,puwLen);
    }

    if (swCount == 0)
    {
        swCount = osTabMatchFile(pscCmdKey,puwLen);
    }

    return swCount;
}

/*****************************************************************************
 Function    : osCmdAscendingInsert
 Description :  Ascending insert command list
 Input       : pstCmd
 Output      : None
 Return      : None
 *****************************************************************************/
LITE_OS_SEC_TEXT_MINOR void osCmdAscendingInsert(CMD_ITEM_S *pstCmd)
{
    CMD_ITEM_S *pstCmdItem = (CMD_ITEM_S *)NULL;
    CMD_ITEM_S *pstCmdNext = (CMD_ITEM_S *)NULL;

    for (pstCmdItem = LOS_DL_LIST_ENTRY((&m_stCmdInfo.stCmdList.list)->pstPrev, CMD_ITEM_S, list);
            &pstCmdItem->list != &(m_stCmdInfo.stCmdList.list); )
    {
        pstCmdNext = LOS_DL_LIST_ENTRY(pstCmdItem->list.pstPrev, CMD_ITEM_S, list);
        if (&pstCmdNext->list != &(m_stCmdInfo.stCmdList.list))
        {
            if ((strncmp((char *)(pstCmdItem->pstCmd->pscCmdKey), (char *)(pstCmd->pstCmd->pscCmdKey), strlen(pstCmd->pstCmd->pscCmdKey)) >= 0) &&
                 (strncmp((char *)(pstCmdNext->pstCmd->pscCmdKey), (char *)(pstCmd->pstCmd->pscCmdKey), strlen(pstCmd->pstCmd->pscCmdKey)) < 0))
            {
                LOS_ListTailInsert(&(pstCmdItem->list), &(pstCmd->list));
                return;
            }

            pstCmdItem = pstCmdNext;
        }
        else
        {
            if (strncmp((char *)(pstCmd->pstCmd->pscCmdKey), (char *)(pstCmdItem->pstCmd->pscCmdKey), strlen(pstCmd->pstCmd->pscCmdKey)) > 0)
            {
                pstCmdItem = pstCmdNext;
            }
            break;
        }
    }

    LOS_ListTailInsert(&(pstCmdItem->list), &(pstCmd->list));
}


LITE_OS_SEC_TEXT_MINOR void shellKeyInit(SHELL_CB * pStShell)
{
    CMD_KEY_S *pStCmdKeyLink;
    pStCmdKeyLink = (CMD_KEY_S *)LOS_MemAlloc(m_aucSysMem0, sizeof(CMD_KEY_S));
    if (pStCmdKeyLink == NULL)
    {
        PRINT_ERR("shell memory alloc error!\n");
        return;
    }
    pStCmdKeyLink->uwCnt = 0;
    LOS_ListInit(&(pStCmdKeyLink->list));
    pStShell->pStCmdKeyLink = (void *)pStCmdKeyLink;
    pStShell->pStCmdMask = (void *)pStCmdKeyLink;
}

LITE_OS_SEC_TEXT_MINOR void shellKeyDeInit(CMD_KEY_S * pStCmdKeyLink)
{
    CMD_KEY_S *pstCmdtmp;

    while (!LOS_ListEmpty(&(pStCmdKeyLink->list)))
    {
        pstCmdtmp = LOS_DL_LIST_ENTRY(pStCmdKeyLink->list.pstNext, CMD_KEY_S, list);
        LOS_ListDelete(&pstCmdtmp->list);
        (VOID)LOS_MemFree(m_aucSysMem0, pstCmdtmp->pcString);/*lint !e424*/
    }

    pStCmdKeyLink->uwCnt = 0;
    (void)LOS_MemFree(m_aucSysMem0, pStCmdKeyLink);
}

LITE_OS_SEC_TEXT_MINOR UINT32 osShellSysCmdRegister(VOID)
{
    UINT32 i = 0;
    UINT8 *pCmdItem;
    UINT32 index = ((AARCHPTR)&g_shellcmd_end - (AARCHPTR)&g_shellcmd[0])/sizeof(CMD_ITEM);
    CMD_ITEM_S *pstCmdItem;

    pCmdItem = (UINT8 *)LOS_MemAlloc(m_aucSysMem0, index * sizeof(CMD_ITEM_S));
    if (NULL == pCmdItem)
    {
        PRINT_ERR("System memory allocation failure!\n");
        return (UINT32)OS_ERROR;
    }

    for (i = 0; i < index; ++i)
    {
        pstCmdItem = (CMD_ITEM_S*)(pCmdItem + i * sizeof(CMD_ITEM_S));
        pstCmdItem->pstCmd = &g_shellcmd[i];
        osCmdAscendingInsert(pstCmdItem);
    }
    m_stCmdInfo.uwListNum += index;
    return LOS_OK; /*lint !e429*/
}


LITE_OS_SEC_TEXT_MINOR VOID osShellCmdPush(CHAR *pcString, CMD_KEY_S *pstCmdNode)
{
    CMD_KEY_S   *pstCmdkey = NULL;
    CMD_KEY_S   *pstCmdNxt;
    UINT32      uwLen;

    if (NULL == pcString || strlen(pcString) == 0)
    {
        return;
    }
    uwLen = strlen(pcString);

    if (pstCmdNode->uwCnt != 0)
    {
        pstCmdNxt = LOS_DL_LIST_ENTRY(pstCmdNode->list.pstPrev, CMD_KEY_S, list);
        if (strcmp(pcString, pstCmdNxt->pcString) == 0)
        {
            return;
        }
    }

    pstCmdkey = (CMD_KEY_S *)LOS_MemAlloc(m_aucSysMem0, sizeof(CMD_KEY_S) + uwLen + 1);
    if (pstCmdkey == NULL)
    {
        return;
    }

    if(strncpy_s(pstCmdkey->pcString, uwLen + 1, pcString, uwLen + 1))
    {
        (VOID)LOS_MemFree(m_aucSysMem0, pstCmdkey);
        return;
    }

    if (pstCmdNode->uwCnt == CMD_HISTORY_LEN)
    {
        pstCmdNxt = LOS_DL_LIST_ENTRY(pstCmdNode->list.pstNext, CMD_KEY_S, list);
        LOS_ListDelete(&(pstCmdNxt->list));
        LOS_ListTailInsert(&(pstCmdNode->list), &(pstCmdkey->list));
        (VOID)LOS_MemFree(m_aucSysMem0, (void *)pstCmdNxt); /*lint !e424*/
        return; /*lint !e429*/
    }

    LOS_ListTailInsert(&(pstCmdNode->list), &(pstCmdkey->list));
    if (pstCmdNode->uwCnt < CMD_HISTORY_LEN)
    {
        pstCmdNode->uwCnt++;
    }

    return; /*lint !e429*/
}

LITE_OS_SEC_TEXT_MINOR VOID shellHistoryShow(UINT32 uwValue, SHELL_CB *pStShell)
{
    CMD_KEY_S *pstCmdtmp = NULL;
    CMD_KEY_S *pstCmdNode = pStShell->pStCmdKeyLink;
    CMD_KEY_S *pstCmdMask = pStShell->pStCmdMask;

    if (uwValue == CMD_KEY_DOWN)
    {
        if (pstCmdMask == pstCmdNode)
        {
            return;
        }

        pstCmdtmp = LOS_DL_LIST_ENTRY(pstCmdMask->list.pstNext, CMD_KEY_S, list);
        if (pstCmdtmp != pstCmdNode)
        {
            pstCmdMask = pstCmdtmp;
        }
        else
        {
            return;
        }
    }
    else if (uwValue == CMD_KEY_UP)
    {
        pstCmdtmp =LOS_DL_LIST_ENTRY(pstCmdMask->list.pstPrev, CMD_KEY_S, list);
        if (pstCmdtmp != pstCmdNode)
        {
            pstCmdMask = pstCmdtmp;
        }
        else
        {
            return;
        }
    }

    while (pStShell->uwShellBufOffset--)
        PRINTK("\b \b");
    PRINTK("%s",pstCmdMask->pcString);
    pStShell->uwShellBufOffset = strlen(pstCmdMask->pcString);
    (VOID)memset_s(pStShell->acShellbuf, SHOW_MAX_LEN, 0, SHOW_MAX_LEN);
    (VOID)memcpy_s(pStShell->acShellbuf, SHOW_MAX_LEN, pstCmdMask->pcString, pStShell->uwShellBufOffset);
    pStShell->pStCmdMask = (void *)pstCmdMask;

}

LITE_OS_SEC_TEXT_MINOR UINT32 osCmdExec(TRANSID_T uwTransId, CMD_PARSED_S *pstCmdParsed, CHAR *pscCmdStr)
{
    UINT32          uwRet = LOS_OK;
    CMD_CBK_FUNC    pfnCmdHook = NULL;
    CMD_ITEM_S      *pstCurCmdItem = (CMD_ITEM_S *)NULL;
    UINT32          i = 0;

    if (NULL == pscCmdStr || strlen(pscCmdStr) == 0)
    {
        return (UINT32)OS_ERROR;
    }


    uwRet = osCmdParse(pscCmdStr, pstCmdParsed);
    if (uwRet != LOS_OK)
    {
        return uwRet;
    }

    LOS_DL_LIST_FOR_EACH_ENTRY(pstCurCmdItem, &(m_stCmdInfo.stCmdList.list), CMD_ITEM_S, list)
    {
        if (pstCmdParsed->enCmdType == pstCurCmdItem->pstCmd->enCmdType &&
            (strlen(pstCurCmdItem->pstCmd->pscCmdKey) == strlen(pstCmdParsed->ascCmdKey)) &&
            (0 == strncmp(pstCurCmdItem->pstCmd->pscCmdKey, (CHAR *)(pstCmdParsed->ascCmdKey), strlen(pstCurCmdItem->pstCmd->pscCmdKey))))
        {
            pfnCmdHook = pstCurCmdItem->pstCmd->pfnCmdHook;
            break;
        }
    }

    uwRet = OS_ERROR;
    if (NULL != pfnCmdHook)
    {
        uwRet = (pfnCmdHook)(pstCmdParsed->uwParCnt, (CHAR **)&pstCmdParsed->stParamArray); /*lint !e545*/
    }

    for(i = 0; i < pstCmdParsed->uwParCnt; i++)
    {
        if((CHAR*)(pstCmdParsed->stParamArray[i]) != NULL)
        {
            (VOID)LOS_MemFree(m_aucSysMem0, (CHAR*)(pstCmdParsed->stParamArray[i]));
            pstCmdParsed->stParamArray[i] = (AARCHPTR)NULL;
        }
    }

    return (UINT32)uwRet;
}

LITE_OS_SEC_TEXT_MINOR UINT32 osCmdInit(VOID)
{
    UINT32 uwRet = 0;
    LOS_ListInit(&(m_stCmdInfo.stCmdList.list));
    m_stCmdInfo.uwListNum = 0;
    m_stCmdInfo.uwInitMagicFlag = SHELL_INIT_MAGIC_FLAG;
    m_stCmdInfo.uwLock  = 0;
    uwRet = LOS_MuxCreate(&m_stCmdInfo.uwLock);
    if(uwRet != LOS_OK)
    {
        PRINT_ERR("creat sem for shell cmd info failed\n");
        return OS_ERROR;
    }
    return LOS_OK;
}

LITE_OS_SEC_TEXT_MINOR UINT32 osCmdReg(CMD_TYPE_E enCmdType, CHAR *pscCmdKey, UINT32 uwParaNum, CMD_CBK_FUNC pfnCmdProc)
{
    CMD_ITEM *pstCmdItem;
    CMD_ITEM_S *pstCmdItem_s;

    (void)LOS_MuxPend(m_stCmdInfo.uwLock,LOS_WAIT_FOREVER);
    if (m_stCmdInfo.uwInitMagicFlag != SHELL_INIT_MAGIC_FLAG)
    {
        (void)LOS_MuxPost(m_stCmdInfo.uwLock);
        PRINT_ERR("[%s] shell is not yet initialized!\n",__FUNCTION__);
        return OS_ERRNO_SHELL_NOT_INIT;
    }
    (void)LOS_MuxPost(m_stCmdInfo.uwLock);

    if ((NULL == pfnCmdProc) || (NULL == pscCmdKey) ||
        (enCmdType >= CMD_TYPE_BUTT) || (strlen(pscCmdKey) >= CMD_KEY_LEN) || !strlen(pscCmdKey))
    {
        return OS_ERRNO_SHELL_CMDREG_PARA_ERROR;
    }

    if (uwParaNum > CMD_MAX_PARAS)
    {
        if (uwParaNum != XARGS)
        {
            return OS_ERRNO_SHELL_CMDREG_PARA_ERROR;
        }
    }

    if (!osCmdKeyCheck(pscCmdKey))
    {
        return OS_ERRNO_SHELL_CMDREG_CMD_ERROR;
    }

    (void)LOS_MuxPend(m_stCmdInfo.uwLock,LOS_WAIT_FOREVER);
    LOS_DL_LIST_FOR_EACH_ENTRY(pstCmdItem_s, &(m_stCmdInfo.stCmdList.list), CMD_ITEM_S, list)
    {
        if ((enCmdType == pstCmdItem_s->pstCmd->enCmdType) && ((strlen(pscCmdKey) == strlen(pstCmdItem_s->pstCmd->pscCmdKey)) &&
            (0 == strncmp((char *)(pstCmdItem_s->pstCmd->pscCmdKey),pscCmdKey,strlen(pscCmdKey)))))
        {
            (void)LOS_MuxPost(m_stCmdInfo.uwLock);
            return OS_ERRNO_SHELL_CMDREG_CMD_EXIST;
        }
    }
    (void)LOS_MuxPost(m_stCmdInfo.uwLock);

    pstCmdItem = (CMD_ITEM *)LOS_MemAlloc(m_aucSysMem0, sizeof(CMD_ITEM));
    if (NULL == pstCmdItem)
    {
        return OS_ERRNO_SHELL_CMDREG_MEMALLOC_ERROR;
    }
    (VOID)memset_s(pstCmdItem, sizeof(CMD_ITEM), '\0', sizeof(CMD_ITEM));

    pstCmdItem_s = (CMD_ITEM_S *)LOS_MemAlloc(m_aucSysMem0, sizeof(CMD_ITEM_S));
    if (NULL == pstCmdItem_s)
    {
        (VOID)LOS_MemFree(m_aucSysMem0, pstCmdItem);
        return OS_ERRNO_SHELL_CMDREG_MEMALLOC_ERROR;
    }

    (VOID)memset_s(pstCmdItem_s, sizeof(CMD_ITEM_S), '\0', sizeof(CMD_ITEM_S));
    pstCmdItem_s->pstCmd = pstCmdItem;
    pstCmdItem_s->pstCmd->pfnCmdHook  = pfnCmdProc;
    pstCmdItem_s->pstCmd->uwParaNum   = uwParaNum;
    pstCmdItem_s->pstCmd->enCmdType   = enCmdType;
    pstCmdItem_s->pstCmd->pscCmdKey   = pscCmdKey;

    (void)LOS_MuxPend(m_stCmdInfo.uwLock,LOS_WAIT_FOREVER);
    osCmdAscendingInsert(pstCmdItem_s);
    m_stCmdInfo.uwListNum++;
    (void)LOS_MuxPost(m_stCmdInfo.uwLock);

    return LOS_OK; /*lint !e429*/
}



#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
