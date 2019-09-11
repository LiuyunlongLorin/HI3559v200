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

#ifndef _HWLITEOS_SHELL_SHCMDPARSE_H__
#define _HWLITEOS_SHELL_SHCMDPARSE_H__


#include <string.h>
#include "show.h"
#include "los_base.h"

#ifdef  __cplusplus
#if  __cplusplus
extern "C" {
#endif
#endif


#define CMD_PARSED_RETCODE_BASE            (LOS_OK)
#define CMD_PARSED_RETCODE_TYPE_INVALID    (CMD_PARSED_RETCODE_BASE + 1)
#define CMD_PARSED_RETCODE_PARAM_OVERTOP   (CMD_PARSED_RETCODE_BASE + 3)
#define CMD_PARSED_RETCODE_CMDKEY_NOTFOUND (CMD_PARSED_RETCODE_BASE + 4)

typedef UINT32 (*FUNC_ONE_TOKEN)(VOID *ctx, UINT32 uwIndex, CHAR *pscToken);


/**
* CMD_PARSED_S
*
* Descrebe: the info struct after cmd parser
*
*/
typedef struct tagCmdParsed
{
    BOOL    bCmdType;                          /* is there a cmd keyword */
    UINT32  uwRetCode;
    UINT32  uwParCnt;                       /* count of para */
    CMD_TYPE_E  enCmdType;                          /* cmd type, judge cmd keyword */
    CHAR        ascCmdKey[CMD_KEY_LEN];               /* cmd keyword str */
    AARCHPTR      stParamArray[CMD_MAX_PARAS];
} CMD_PARSED_S;


extern UINT32 osCmdParse(CHAR *pscCmdStr, CMD_PARSED_S *pstCmdParsed);
extern CHAR * osCmdParseStrdup(CHAR * pscStr);
extern UINT32 osCmdParseOneToken( CMD_PARSED_S *pstCmdParsed, UINT32 uiIndex, CHAR *pscToken );
extern UINT32 osCmdTokenSplit( CHAR *pscCmdStr,
                               CHAR  scSplit,
                               CMD_PARSED_S *pstCmdParsed );



#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif    /*_HWLITEOS_SHELL_SHCMDPARSE_H__*/
