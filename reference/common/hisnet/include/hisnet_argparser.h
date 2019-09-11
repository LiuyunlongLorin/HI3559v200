/******************************************************************************

  Copyright (C), 2005-2006, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : hisnet_server_argparser.h
  Version       : Initial Draft
  Author        : Hisilicon multimedia software group
  Created       : 2005/7/1
  Last Modified :
  Description   : Argument parser
  Function List :
  History       :
  1.Date        : 2005/7/27
    Author      : T41030
    Modification: Created file

  2.Date        : 2006/4/7
    Author      : qiubin
    Modification: modify for HI3510 Demo
******************************************************************************/


#ifndef __HISNET_ARGPARSER_H__
#define __HISNET_ARGPARSER_H__
#include "hi_appcomm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define HI_ERR_ARG_BASE                 0xA102C000
#define HI_ERR_ARG_WRONG_FMT            (HI_ERR_ARG_BASE + 0x01)
#define HI_ERR_ARG_NOT_DEFINE_NO_OPT    (HI_ERR_ARG_BASE + 0x02)
#define HI_ERR_ARG_NO_OPT               (HI_ERR_ARG_BASE + 0x03)
#define HI_ERR_ARG_OPT_NOTSET           (HI_ERR_ARG_BASE + 0x04)
#define HI_ERR_ARG_DEFINE               (HI_ERR_ARG_BASE + 0x05)
#define HI_ERR_ARG_MUSTOPT_NOTSET       (HI_ERR_ARG_BASE + 0x06)
#define HI_ERR_ARG_NOT_IN_REGION        (HI_ERR_ARG_BASE + 0x07)

#define ARG_S4 "    "     /*4 space*/

typedef enum hiARG_TYPE_E
{
    /** ARG_TYPE_NO_PARA / ARG_TYPE_NO_OPT only canbe select one of them */
    /** ARG_TYPE_STRING /ARG_TYPE_CHAR /ARG_TYPE_INT / ARG_TYPE_FLOAT only canbe select one of them */
    ARG_TYPE_MUST       = 1,   /* Required Arg */
    ARG_TYPE_NO_OPT     = 2,   /* No option is allowed(-XXX), only one parameter */
    ARG_TYPE_NO_PARA    = 4,   /* Single, no parameter */
    ARG_TYPE_STRING     = 8,   /* String */
    ARG_TYPE_CHAR       = 16,  /* Char */
    ARG_TYPE_INT        = 32,  /* Interger */
    ARG_TYPE_FLOAT      = 64,  /* Float */
    ARG_TYPE_HELP       = 128, /**/
    ARG_TYPE_SINGLE     = 256, /* Solo Option, or return parse error */
    ARG_TYPE_END        = 512,
    ARG_TYPE_BUTT       = 1024,
    ARG_TYPE_4BYTE      = 0xFFFFFFFF
} ARG_TYPE_E;

typedef struct hiARGOPT_STRUCT_S
{
    HI_CHAR*        pszOpt;         /** Option Name, eg. :"XXX" in "cmd -XXX"  */
    HI_U32          u32Type;        /** Use demo_arg_type, eg.:ARG_TYPE_MUST|ARG_TYPE_NO_OPT|ARG_TYPE_STRING */
    HI_CHAR*        pszValueRegion; /** Optional Value or Region, Application confirm
                                                                1) Option Value : "1|2|3|all",  can be used to string,char,int
                                                                2) Option Region : "0~10","1~","~12","a~z","a~";,  can be used to char,int,float
                                                                2) Option Region :  "~-12|0~10|20","~a|c~e|x~y|X~Y";,  can be used to int,char,float */
    HI_CHAR         s8Isset;        /** 1: be set, 0 not set */
    HI_U8           au8resv[3];
    HI_CHAR*        pszHelpMsg;
    HI_VOID*        pValue;
    HI_U32          u32ValueBuffersize;/*pValue BufferSize*/
} ARG_OPT_S;

HI_S32  Hisnet_Server_ARG_Parser(HI_S32 s32Argc, const HI_CHAR** ppszArgv, ARG_OPT_S* pstArgopts);
HI_VOID Hisnet_Server_ARG_PrintHelp(HI_CHAR* pszOut, ARG_OPT_S* pstArgopts);
HI_S32  Hisnet_Server_ARG_OptIsSet(HI_CHAR* pszOptName, ARG_OPT_S* pstArgopts);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __HISNET_ARGPARSER_H__ */
