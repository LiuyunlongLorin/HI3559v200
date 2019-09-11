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
#include "los_memory.h"

/*
**Filter out double quote or single-quoted strings at both ends
 */
 LITE_OS_SEC_TEXT_MINOR CHAR *osCmdParseStrdup(CHAR *pscStr)
 {
     CHAR *pscTemp;
     CHAR * pscNew;

     pscNew = (CHAR*)LOS_MemAlloc(m_aucSysMem0, strlen(pscStr) + 1);
     if(pscNew == NULL)
         return NULL;

     pscTemp = pscNew;
     for(; *pscStr != '\0'; pscStr++)
     {
         if( *pscStr == '\"' || *pscStr == '\'')
             continue;
         *pscNew = *pscStr;
         pscNew++;
     }
     *pscNew = '\0';
     return pscTemp;
 }

LITE_OS_SEC_TEXT_MINOR UINT32 osCmdParseParaGet(AARCHPTR *uwvalue, CHAR *pscParaTokenStr)
{
    if (NULL == pscParaTokenStr || NULL == uwvalue )
    {
        return (UINT32)OS_ERROR;
    }

    *uwvalue = (AARCHPTR)osCmdParseStrdup(pscParaTokenStr);

    return LOS_OK;
}

LITE_OS_SEC_TEXT_MINOR UINT32 osCmdParseOneToken(CMD_PARSED_S *pstCmdParsed, UINT32 uwIndex, CHAR *pscToken)
{
    UINT32 uwRet = LOS_OK;
    UINT32 uwlen = 0;

    if (NULL == pstCmdParsed)
    {
        return (UINT32)OS_ERROR;
    }

    if (0 == uwIndex)
    {
        if(pstCmdParsed->enCmdType != CMD_TYPE_STD)
        {
            return uwRet;
        }
    }

    if (pscToken != NULL && pstCmdParsed->uwParCnt < CMD_MAX_PARAS)
    {
        uwlen = pstCmdParsed->uwParCnt;
        uwRet = osCmdParseParaGet(&(pstCmdParsed->stParamArray[uwlen]), pscToken);
        if(uwRet != LOS_OK)
        {
            return uwRet;
        }
        pstCmdParsed->uwParCnt++;
    }

    return uwRet;
}

LITE_OS_SEC_TEXT_MINOR UINT32 osCmdTokenSplit(CHAR *pscCmdStr, CHAR scSplit, CMD_PARSED_S *pstCmdParsed)
{
    enum {
        STAT_INIT,
        STAT_TOKEN_IN,
        STAT_TOKEN_OUT
    } state = STAT_INIT;
    UINT32 uwCount = 0;
    CHAR  *psctoken, *p;
    UINT32 uwRet = LOS_OK;
    BOOL uwQuotes = FALSE;

    if (NULL == pscCmdStr)
    {
        return (UINT32)OS_ERROR;
    }

    p = pscCmdStr;
    psctoken = pscCmdStr;

    for (p = pscCmdStr; (*p) != '\0' && LOS_OK == uwRet; p++)
    {
        if((*p) == '\"')
        {
            SWITCH_QUOTES_STATUS(uwQuotes);
        }
        switch (state)
        {
            case STAT_INIT:
            case STAT_TOKEN_IN:
            {
                if ((*p) == scSplit  && QUOTES_STATUS_CLOSE(uwQuotes))
                {
                    *p = '\0';
                    uwRet = osCmdParseOneToken(pstCmdParsed, uwCount++, psctoken);
                    state = STAT_TOKEN_OUT;
                }
            }
            break;
            case STAT_TOKEN_OUT:
            {
                if ((*p) != scSplit)
                {
                    psctoken = p;
                    state = STAT_TOKEN_IN;
                }
            }
            break;
            default:
            break;
        }
    }

    if ((LOS_OK == uwRet && STAT_TOKEN_IN == state) || STAT_INIT == state)
    {
        uwRet = osCmdParseOneToken(pstCmdParsed, uwCount++, psctoken);
    }

    return uwRet;
}

LITE_OS_SEC_TEXT_MINOR UINT32 osCmdParse(CHAR *pscCmdStr, CMD_PARSED_S *pstCmdParsed)
{
    if (NULL == pscCmdStr || NULL == pstCmdParsed ||
        0 == strlen(pscCmdStr))
    {
        return (UINT32)OS_ERROR;
    }

    (VOID)osCmdTokenSplit(pscCmdStr, ' ', pstCmdParsed);

    return pstCmdParsed->uwRetCode;
}
