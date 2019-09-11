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


#ifndef _HWLITEOS_SHELL_SHCMD_H__
#define _HWLITEOS_SHELL_SHCMD_H__



#include "string.h"
#include "stdlib.h"
#include "los_base.h"
#include "los_list.h"
#include "shcmdparse.h"
#include "show.h"

#include "los_tables.h"
#include "console.h"

#ifdef  __cplusplus
#if  __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

typedef BOOL(*CMD_VERIFY_TRANSID)(UINT32 uwTransID);

typedef struct tagCmdItem
{
    CMD_TYPE_E  enCmdType;
    CHAR        *pscCmdKey;
    UINT32      uwParaNum;
    CMD_CBK_FUNC    pfnCmdHook;
} CMD_ITEM;

typedef struct tagCmdItemSet
{
    LOS_DL_LIST list;
    CMD_ITEM *pstCmd;
} CMD_ITEM_S;


/* global info for shell module */
typedef struct tagCmdModInfo
{
    CMD_ITEM_S  stCmdList;
    UINT32      uwListNum;
    UINT32      uwInitMagicFlag;
    UINT32      uwLock;
    CMD_VERIFY_TRANSID   pfnTransIdHook;
} CMD_MOD_INFO_S;

typedef struct tagCmdKeySave
{
    UINT32 uwCnt;
    LOS_DL_LIST list;
    CHAR pcString[0]; /*lint !e43*/
} CMD_KEY_S;

#define SHELLCMD_ENTRY(_l, _cmdtype, _cmdkey, _paranum, _cmdhook)   \
    CMD_ITEM _l LOS_HAL_TABLE_ENTRY(shellcmd) =  \
{                                                           \
    _cmdtype,                                              \
    _cmdkey,                                               \
    _paranum,                                               \
    _cmdhook                                                \
};

#define NEED_NEW_LINE(times_print,line_cap) ((times_print) % (line_cap) == 0)
#define SCREEN_IS_FULL(times_print,line_cap) ((times_print) >= ((line_cap) * DEFAULT_SCREEN_HEIGNT))

extern CMD_ITEM g_shellcmd[];
extern CMD_ITEM g_shellcmd_end;

extern UINT32 osCmdInit(VOID);
extern UINT32 osCmdExec(TRANSID_T uwTransId, CMD_PARSED_S *pstCmdParsed, CHAR *pcCmdStr);
extern UINT32 osCmdKeyShift(CHAR *pscCmdKey, CHAR *pscCmdOut, UINT32 uwSize);
extern BOOL   osCmdKeyCheck(CHAR *pscCmdKey);
extern INT32 osTabCompletion(CHAR *pscBuf, UINT32 *uwLen);
extern void osShellCmdPush(CHAR *pcString, CMD_KEY_S *pstCmdNode);
extern void osCmdAscendingInsert(CMD_ITEM_S *pstCmd);
extern void shellHistoryShow(UINT32 uwValue, SHELL_CB * pStShell);
extern void shellKeyInit(SHELL_CB * pStShell);
extern void shellKeyDeInit(CMD_KEY_S * pStCmdKeyLink);
extern UINT32 osShellSysCmdRegister(VOID);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif    /*_HWLITEOS_SHELL_SHCMD_H__*/
