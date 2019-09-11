/******************************************************************************

  Copyright (C), 2005-2006, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : hi_argparser.c
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "hisnet_argparser.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define VALUE_REGION_REGION     1       /* Region */
#define VALUE_REGION_SINGLE     0xFE    /* Single Value */
#define VALUE_REGION_DWNLMT     2       /* Exist Minimum Limit */
#define VALUE_REGION_NO_DWNLMT  0xFD    /* No Minimum Limit */
#define VALUE_REGION_UPLMT      4       /* Exist Maximum Limit */
#define VALUE_REGION_NO_UPLMT   0xFB    /* No Maximum Limit */

#define ARG_SET_REGION(u8FRegion)       (u8FRegion |= VALUE_REGION_REGION)      /* Region Bit */
#define ARG_CLR_REGION(u8FRegion)       (u8FRegion &= VALUE_REGION_SINGLE)      /* Single Value */
#define ARG_SET_DWNLMT(u8FRegion)       (u8FRegion |= VALUE_REGION_DWNLMT)      /* Set Minimum Limit Bit */
#define ARG_CLR_DWNLMT(u8FRegion)       (u8FRegion &= VALUE_REGION_NO_DWNLMT)   /* Clear Minimum Limit */
#define ARG_SET_UPLMT(u8FRegion)        (u8FRegion |= VALUE_REGION_UPLMT)       /* Set Maximum Limit Bit */
#define ARG_CLR_UPLMT(u8FRegion)        (u8FRegion &= VALUE_REGION_NO_UPLMT)    /* Clear Maximum Limit */

#define ARG_IS_REGION(u8FRegion)        (u8FRegion & VALUE_REGION_REGION)           /* Region or not */
#define ARG_IS_DWNLMT(u8FRegion)        ((u8FRegion & VALUE_REGION_DWNLMT)  >> 1)   /* Exist Minimum Limit or not */
#define ARG_IS_UPLMT(u8FRegion)         ((u8FRegion & VALUE_REGION_UPLMT)   >> 2)   /* Exist Maximum Limit or not */

#define MAX_ARG_CGI_OUT 2048   /* Parser String Maximum Length */

/*****************************************************************************
\brief 1 get return string, add it after current string
\attention \n
\param[out]   pstrOut:  output string
\param[in]    pszfmt:   ariable string
\retval ::>=0   string length
\retval ::-1    failure

\see \n
:: \n
*****************************************************************************/
HI_S32 Arg_GetRetString(HI_CHAR* pstrOut, HI_CHAR* pszfmt, ...)
{
    int ret = 0;
    int strOutLen = 0;
    va_list stVal;

    if (pstrOut == NULL || pszfmt == NULL)
    {
        printf("pstrOut or pszfmt is NULL\n\n");
        return -1;
    }

    strOutLen = strlen(pstrOut);

    if (strOutLen >= MAX_ARG_CGI_OUT)
    {
        printf("--------strOut len reaches the upper limit----------\n\n");
        return 0;
    }

    va_start(stVal, pszfmt);
    ret = vsprintf(pstrOut + strOutLen, pszfmt, stVal);
    va_end(stVal);

    return ret;
}


HI_S32 Hisnet_ARG_Get_DwnAndUp(HI_CHAR* pszSHead, HI_CHAR* pszSTail, HI_CHAR** ppszDWN, HI_CHAR** ppszUP)
{
    HI_CHAR*    pszVRBegin     = pszSHead;      /* String begin address */
    HI_CHAR*    pszVREnd       = pszVRBegin;    /* String end address */
    HI_U8       u8FRegion       = 0;            /*bit0 : 0, Single value; 1, not single value;
                                                                                bit1 : 0, no up limit ;1, exist up limit;
                                                                                bit3 : 0, no down limit;1,exist down limit*/
    HI_U32  u32DwnLen = 0;
    HI_U32  u32UpLen = 0;
    HI_CHAR*    pszVRSplit = NULL;

    *ppszDWN = *ppszUP = NULL;

    while (pszVREnd != pszSTail)
    {
        if (*pszVREnd != '~')
        {
            pszVREnd++;
        }
        else    /* *pszVREnd== '~' */
        {
            pszVRSplit = pszVREnd;
            break;
        }
    }

    if ((NULL == pszVRSplit) || (pszSHead == pszSTail )) // single value "..." or "~"
    {
        u32DwnLen = pszSTail - pszSHead + 1;
        *ppszDWN = (HI_CHAR*)malloc(u32DwnLen + 1);

        if (*ppszDWN)
        {
            memcpy(*ppszDWN, pszSHead, u32DwnLen);
            (*ppszDWN)[u32DwnLen] = 0;
        }

        ARG_CLR_REGION(u8FRegion);
        return 1;
    }
    else
    {
        ARG_SET_REGION(u8FRegion);

        u32DwnLen = pszVRSplit - pszSHead;

        if (u32DwnLen > 0)     //"..~"
        {
            *ppszDWN = (HI_CHAR*)malloc(u32DwnLen + 1);

            if (*ppszDWN)
            {
                memcpy(*ppszDWN, pszSHead, u32DwnLen);
                (*ppszDWN)[u32DwnLen] = 0;
            }

            ARG_SET_DWNLMT(u8FRegion);
        }
        else
        {
            ARG_CLR_DWNLMT(u8FRegion);
        }

        u32UpLen = pszSTail - pszVRSplit;

        if (u32UpLen > 0)   //"~.."
        {
            *ppszUP = (HI_CHAR*)malloc(u32UpLen + 1);

            if (*ppszUP)
            {
                memcpy(*ppszUP, pszVRSplit + 1, u32UpLen);
                (*ppszUP)[u32UpLen] = 0;
            }

            ARG_SET_UPLMT(u8FRegion);
        }
        else
        {
            ARG_CLR_UPLMT(u8FRegion);
        }

        return 0;
    }
}

HI_S32 Hisnet_ARG_CheckRegion(ARG_OPT_S* pOpt, const char* pszArgv)
{
    HI_CHAR*    pszVRBegin     = pOpt->pszValueRegion;     /* Region String begin address */
    HI_CHAR*    pszVREnd       = pszVRBegin;               /* Region String end address */
    HI_S32      s32CRLT         = 0;                       /* String Compare Return Value */
    HI_U32      u32Count        = 0;
    HI_CHAR*    pszDWN          = NULL;              /* Down String*/
    HI_CHAR*    pszUP           = NULL;              /* Up String */
    HI_U32      u32Rtn          = 0;                        /* CallBack Return Value */

    if (pOpt->pszValueRegion == NULL)
    {
        return HI_SUCCESS;
    }

    switch (pOpt->u32Type & (ARG_TYPE_FLOAT | ARG_TYPE_INT | ARG_TYPE_CHAR | ARG_TYPE_STRING))
    {
        case ARG_TYPE_STRING:
        {
            while (pszVRBegin != NULL)
            {
                u32Count = 0;

                while ((*pszVREnd != '|') && (*pszVREnd != '\0'))
                {
                    pszVREnd++;
                    u32Count++;
                }

                if (strlen(pszArgv) == u32Count)
                {
                    s32CRLT = memcmp(pszVRBegin, pszArgv, u32Count);

                    if (0 == s32CRLT)
                    {
                        break;
                    }
                }

                if (*pszVREnd != '\0')
                {
                    pszVREnd++;
                    pszVRBegin = pszVREnd;
                }
                else
                {
                    pszVRBegin = NULL;
                }
            }

            if (pszVRBegin == NULL)
            {
                return HI_ERR_ARG_NOT_IN_REGION;
            }

            break;
        }

        case ARG_TYPE_CHAR:
        {
            while (pszVRBegin != NULL)
            {
                u32Count = 0;

                pszDWN = pszUP = NULL;

                while ((*pszVREnd != '|') && (*pszVREnd != '\0'))
                {
                    pszVREnd++;
                    u32Count++;
                }

                u32Rtn = Hisnet_ARG_Get_DwnAndUp(pszVRBegin, pszVREnd, &pszDWN, &pszUP);

                if (0 == u32Rtn) /* Region */
                {
                    if ((pszDWN == NULL) || (pszArgv[0] >= pszDWN[0]))   /* Input Param >= Down Limit */
                    {
                        if ((pszUP == NULL) || (pszArgv[0] <= pszUP[0])) /* Input Param >= Up Limit */
                        {
                            break;
                        }
                    }
                }
                else /* Single Value */
                {
                    if ((pszDWN == NULL) || (pszArgv[0] == pszDWN[0]))
                    {
                        break;
                    }
                }

                if (*pszVREnd != '\0')
                {
                    pszVREnd++;
                    pszVRBegin = pszVREnd;
                }
                else
                {
                    pszVRBegin = NULL;
                }

                if (NULL != pszDWN)
                {
                    free(pszDWN);
                    pszDWN = NULL;
                }

                if (NULL != pszUP)
                {
                    free(pszUP);
                    pszUP = NULL;
                }
            }

            if (pszVRBegin == NULL)
            {
                return HI_ERR_ARG_NOT_IN_REGION;
            }

            break;
        }

        case ARG_TYPE_INT:
        {
            HI_S32 s32Vlue = atoi(pszArgv);

            while (pszVRBegin != NULL)
            {
                u32Count = 0;

                pszDWN = pszUP = NULL;

                while ((*pszVREnd != '|') && (*pszVREnd != '\0'))
                {
                    pszVREnd++;
                    u32Count++;
                }

                u32Rtn = Hisnet_ARG_Get_DwnAndUp(pszVRBegin, pszVREnd, &pszDWN, &pszUP);

                if (0 == u32Rtn) /* Region */
                {
                    if ((pszDWN == NULL) || (s32Vlue >= atoi(pszDWN)))   /* Input Param >= Down Limit */
                    {
                        if ((pszUP == NULL) || (s32Vlue <= atoi(pszUP))) /* Input Param >= Up Limit */
                        {
                            break;
                        }
                    }
                }
                else /* Single Value */
                {
                    if ((pszDWN == NULL) || (s32Vlue == atoi(pszDWN)))
                    {
                        break;
                    }
                }

                if (*pszVREnd != '\0')
                {
                    pszVREnd++;
                    pszVRBegin = pszVREnd;
                }
                else
                {
                    pszVRBegin = NULL;
                }

                if (NULL != pszDWN)
                {
                    free(pszDWN);
                    pszDWN = NULL;
                }

                if (NULL != pszUP)
                {
                    free(pszUP);
                    pszUP = NULL;
                }
            }

            if (pszVRBegin == NULL)
            {
                return HI_ERR_ARG_NOT_IN_REGION;
            }

            break;
        }

        case ARG_TYPE_FLOAT:
        {
            HI_FLOAT eVlue = (HI_FLOAT)atof(pszArgv);

            while (pszVRBegin != NULL)
            {
                u32Count = 0;

                pszDWN = pszUP = NULL;

                while ((*pszVREnd != '|') && (*pszVREnd != '\0'))
                {
                    pszVREnd++;
                    u32Count++;
                }

                u32Rtn = Hisnet_ARG_Get_DwnAndUp(pszVRBegin, pszVREnd, &pszDWN, &pszUP);

                if (0 == u32Rtn) /* Region */
                {
                    if ((pszDWN == NULL) || (eVlue >= (HI_FLOAT)atof(pszDWN)))  /* Input Param >= Down Limit */
                    {
                        if ((pszUP == NULL) || (eVlue <= (HI_FLOAT)atof(pszUP))) /* Input Param >= Up Limit */
                        {
                            break;
                        }
                    }
                }
                else /* Single Value */
                {
                    if ((pszDWN == NULL) || (eVlue == (HI_FLOAT)atof(pszDWN)))
                    {
                        break;
                    }

                    //return HI_ERR_ARG_DEFINE;
                }

                if (*pszVREnd != '\0')
                {
                    pszVREnd++;
                    pszVRBegin = pszVREnd;
                }
                else
                {
                    pszVRBegin = NULL;
                }

                if (NULL != pszDWN)
                {
                    free(pszDWN);
                    pszDWN = NULL;
                }

                if (NULL != pszUP)
                {
                    free(pszUP);
                    pszUP = NULL;
                }
            }

            if (pszVRBegin == NULL)
            {
                return HI_ERR_ARG_NOT_IN_REGION;
            }

            break;
        }

        default:
        case ARG_TYPE_MUST:
        case ARG_TYPE_NO_OPT:
        case ARG_TYPE_NO_PARA:
        case ARG_TYPE_SINGLE:
        case ARG_TYPE_HELP:
        case ARG_TYPE_END:
        case ARG_TYPE_BUTT:
        case ARG_TYPE_4BYTE:
            pOpt->s8Isset = HI_FALSE;
            //WRITE_LOG_ERROR("error opt type: %u\n",pOpt->type );
            return HI_ERR_ARG_DEFINE;
    }

    if (NULL != pszDWN)
    {
        free(pszDWN);
    }

    if (NULL != pszUP)
    {
        free(pszUP);
    }

    return HI_SUCCESS;
}

HI_S32 Hisnet_ARG_SetValue(ARG_OPT_S* pOpt, const char* szArgv)
{

    if ((NULL == pOpt->pValue) || (0 == pOpt->u32ValueBuffersize) || (NULL == szArgv))
    {
        return HI_FAILURE;
    }

    switch (pOpt->u32Type & (ARG_TYPE_FLOAT | ARG_TYPE_INT | ARG_TYPE_CHAR | ARG_TYPE_STRING))
    {
        case ARG_TYPE_STRING:
            if (Hisnet_ARG_CheckRegion(pOpt, szArgv) == HI_SUCCESS)
            {
                strncpy((char*)pOpt->pValue, szArgv, pOpt->u32ValueBuffersize);
                pOpt->s8Isset = HI_TRUE;
            }
            else
            {
                return HI_FAILURE;
            }

            break;

        case ARG_TYPE_CHAR:
            if (Hisnet_ARG_CheckRegion(pOpt, szArgv) == HI_SUCCESS)
            {
                *((HI_CHAR*)pOpt->pValue) = szArgv[0];
                pOpt->s8Isset = HI_TRUE;
            }
            else
            {
                return HI_FAILURE;
            }

            break;

        case ARG_TYPE_INT:
            if (Hisnet_ARG_CheckRegion(pOpt, szArgv) == HI_SUCCESS)
            {
                *((HI_S32*)pOpt->pValue) = atoi(szArgv);
                pOpt->s8Isset = HI_TRUE;
            }
            else
            {
                return HI_FAILURE;
            }

            break;

        case ARG_TYPE_FLOAT:
            if (Hisnet_ARG_CheckRegion(pOpt, szArgv) == HI_SUCCESS)
            {
                *((float*)pOpt->pValue) = (float)atof(szArgv);
                pOpt->s8Isset = HI_TRUE;
            }
            else
            {
                return HI_FAILURE;
            }

            break;

        default:
        case ARG_TYPE_MUST:
        case ARG_TYPE_NO_OPT:
        case ARG_TYPE_NO_PARA:
        case ARG_TYPE_SINGLE:
        case ARG_TYPE_HELP:
        case ARG_TYPE_END:
        case ARG_TYPE_BUTT:
        case ARG_TYPE_4BYTE:
            pOpt->s8Isset = HI_FALSE;
            //printf("error opt type: %u\n",pOpt->type );
            return HI_ERR_ARG_DEFINE;
    }

    return HI_SUCCESS;
    /*
    SPACEERR:

        return HI_FAILURE;
    */
}
/*
cmd -i 1003 xyz
             ^-------------
    -i 1003
     ^   ^    1003 arg
     ^
     ^------  i    opt
*/

/*****************************************************************************
 Prototype    : StrToArg
 Description  : Convert string to [argc, argv] pair, the INPUT str had chan-
                ged in this function
 Input        : HI_CHAR* pSrc
                int* argc
                HI_CHAR** argv
 Output       : None
 Return Value :
 Calls        :
 Called By    :

  History        :
  1.Date         : 2005/7/27
    Author       : T41030
    Modification : Created function

*****************************************************************************/
//HI_VOID Hisnet_Server_ARG_StrToArg(HI_CHAR* pSrc, int* argc, HI_CHAR** argv)
//{
//    HI_S32 iNum = 0;
//    HI_BOOL bNewWord = HI_FALSE;
//
//    if (NULL == pSrc || NULL == argc || NULL == argv)
//    {
//        printf("pSrc or argc or argv is NULL\n\n");
//        return;
//    }
//
//    while (*pSrc)
//    {
//        if (*pSrc == ' ')
//        {
//            if (bNewWord == HI_TRUE)
//            {
//
//                *pSrc = 0;
//                bNewWord = HI_FALSE;
//            }
//        }
//        else
//        {
//            if (bNewWord == HI_FALSE)
//            {
//                bNewWord = HI_TRUE;
//                argv[iNum] = pSrc;
//                iNum++;
//            }
//        }
//
//        pSrc++;
//
//    }
//
//    *argc = iNum;
//
//
//}


/*****************************************************************************
 Prototype    : Hisnet_ARGFindOptStr
 Description  : Find option string in option lists, if not find, return
                NULL
 Input        : ARG_OPT_S * opts
                HI_CHAR* strOpt
 Output       : None
 Return Value :
 Calls        :
 Called By    :

  History        :
  1.Date         : 2005/7/27
    Author       : T41030
    Modification : Created function

*****************************************************************************/
static ARG_OPT_S* Hisnet_ARGFindOptStr(ARG_OPT_S* opts, const HI_CHAR* strOpt)
{
    HI_S32 i = 0;

    while (opts[i].u32Type < ARG_TYPE_END)
    {
        if (strcmp(opts[i].pszOpt, strOpt) == 0 )
        {
            return &opts[i];
        }

        i++;
    }

    return NULL;
}


static ARG_OPT_S* Hisnet_ARGFindOpt_NoArg(ARG_OPT_S* opts)
{
    HI_S32 i = 0;

    while (opts[i].u32Type < ARG_TYPE_END)
    {
        if (((opts[i].u32Type & ARG_TYPE_NO_OPT) == ARG_TYPE_NO_OPT) && (opts[i].s8Isset == HI_FALSE))
        {
            return &opts[i];
        }

        i++;
    }

    return NULL;
}

//HI_VOID Hisnet_Server_ARG_ClearOpts(ARG_OPT_S* opts)
//{
//    HI_S32 i = 0;
//
//    while (opts[i].u32Type < ARG_TYPE_END)
//    {
//        opts[i].s8Isset = 0;
//        i++;
//    }
//
//}

HI_S32 Hisnet_Server_ARG_OptIsSet(HI_CHAR* opt_name, ARG_OPT_S* opts)
{
    HI_S32 iOptsIdx = 0;

    if (NULL == opt_name || NULL == opts)
    {
        //WRITE_LOG_ERROR("NO OPT!!!\n");
        return HI_ERR_ARG_NO_OPT;
    }

    while (opts[iOptsIdx].u32Type < ARG_TYPE_END)
    {
        //WRITE_LOG_INFO("nnn: %d, %s  %s \n",iOptsIdx,opts[iOptsIdx].sz_opt, opt_name );
        if (strcmp(opts[iOptsIdx].pszOpt, opt_name) == 0)
        {
            if (opts[iOptsIdx].s8Isset)
            {
                return HI_SUCCESS;
            }

            iOptsIdx++;
        }
        else
        {
            iOptsIdx++;
        }
    }

    return HI_ERR_ARG_OPT_NOTSET;
}


HI_S32 Hisnet_Server_ARG_Parser(HI_S32 argc, const HI_CHAR** argv, ARG_OPT_S* opts)
{
    HI_S32      ret;
    HI_S32      iArgIdx     = 0;
    const HI_CHAR*    pCurrArg    = NULL;
    ARG_OPT_S*   pCurrOpt   = NULL;

    HI_BOOL     bHasParseArg    = HI_FALSE;
    //HI_S32      iNO_OPT_Cnt     = 0;
    HI_S32      s32I            = 0;
    HI_U8       u8ISSetSingle   = HI_FALSE;
    HI_U8       u8ISSetOTHER    = HI_FALSE;


    if (NULL == argv || NULL == opts)
    {
        printf("argv or opts is null\n\n");
        return HI_ERR_ARG_NO_OPT;
    }

    //argv[0] is execute command
    for (iArgIdx = 1; iArgIdx < argc ; iArgIdx++)
    {

        pCurrArg = argv[iArgIdx];

        if ((*pCurrArg == '/' || *pCurrArg == '-') && ((48 > *(pCurrArg + 1)) || (57 < * (pCurrArg + 1))))
        {
            if (*pCurrArg == '-')
            {
                if ((48 <= *(pCurrArg + 1)) && (57 >= *(pCurrArg + 1)))
                {
                    //printf("value is - \n");
                    pCurrOpt = Hisnet_ARGFindOpt_NoArg(opts);

                    if (pCurrOpt == NULL)
                    {
                        MLOGE("Wrong Args z: %s\n", argv[iArgIdx]);
                        return HI_ERR_ARG_NOT_DEFINE_NO_OPT;
                    }

                    ret = Hisnet_ARG_SetValue(pCurrOpt, argv[iArgIdx]);

                    if ( ret != HI_SUCCESS)
                    {
                        MLOGE("Wrong Args z: %s\n", argv[iArgIdx]);
                        return ret;
                    }

                    continue;
                }

            }

            pCurrArg++;

            /*the argv[n] as '-' */
            if (*pCurrArg == 0)
            {
                MLOGE("Wrong Args x: %s\n", argv[iArgIdx]);
                return HI_ERR_ARG_WRONG_FMT;
            }

            pCurrOpt = Hisnet_ARGFindOptStr(opts, pCurrArg);

            if (pCurrOpt == NULL)
            {
                MLOGE("Wrong Args y: %s\n", argv[iArgIdx]);
                return HI_ERR_ARG_WRONG_FMT;
            }

            if ((pCurrOpt->u32Type & ARG_TYPE_NO_PARA) == ARG_TYPE_NO_PARA)
            {
                bHasParseArg = HI_FALSE;
                pCurrOpt->s8Isset = HI_TRUE;
            }
            else
            {
                bHasParseArg = HI_TRUE;
            }

            continue;
        }
        else if (bHasParseArg == HI_FALSE)
        {
            pCurrOpt = Hisnet_ARGFindOpt_NoArg(opts);

            if (pCurrOpt == NULL)
            {
                MLOGE("Wrong Args z: %s\n", argv[iArgIdx]);
                return HI_ERR_ARG_NOT_DEFINE_NO_OPT;
            }

            bHasParseArg = HI_TRUE;
        }

        if (HI_TRUE == bHasParseArg )
        {
            ret = Hisnet_ARG_SetValue(pCurrOpt, argv[iArgIdx]);

            if ( ret != HI_SUCCESS)
            {
                MLOGE("Wrong Args z: %s\n", argv[iArgIdx]);
                return ret;
            }

            bHasParseArg = HI_FALSE;
        }

    }

    /* Check Multi SINGLE option or SINGLE and non-SINGLE option  set at the same time */
    for (s32I = 0; (opts[s32I].u32Type & ARG_TYPE_END) != ARG_TYPE_END; s32I++)
    {
        if (opts[s32I].s8Isset == HI_TRUE)
        {
            if (((opts[s32I].u32Type)&ARG_TYPE_SINGLE) == ARG_TYPE_SINGLE)
            {
                if ((HI_FALSE == u8ISSetSingle) && (HI_FALSE == u8ISSetOTHER))
                {

                    u8ISSetSingle = HI_TRUE;
                }
                else
                {
                    MLOGE("\n");
                    return HI_FAILURE;
                }
            }
            else
            {
                if (HI_FALSE == u8ISSetSingle)
                {

                    u8ISSetOTHER = HI_TRUE;
                }
                else
                {
                    MLOGE("\n");
                    return HI_FAILURE;
                }
            }
        }
    }

    if (HI_FALSE == u8ISSetSingle)
    {
        for (s32I = 0; (opts[s32I].u32Type & ARG_TYPE_END) != ARG_TYPE_END; s32I++)
        {
            if ((opts[s32I].s8Isset == HI_FALSE) && (((opts[s32I].u32Type)&ARG_TYPE_MUST) == ARG_TYPE_MUST))
            {
                MLOGE("\n");
                return HI_ERR_ARG_MUSTOPT_NOTSET;
            }
        }
    }

    return HI_SUCCESS;
}

/*return opts[]*/
#if 0
HI_VOID Hisnet_Server_ARG_PrintHelp(HI_U32 u32Handle, ARG_OPT_S* opts)
{
    HI_S32 i = 0;

    while (opts[i].u32Type < ARG_TYPE_END)
    {
        //HI_Fprintf(stdout, "-%s\n%s", opts[i].pszOpt,opts[i].pszHelpMsg);
        //printf("-%s\n%s", opts[i].sz_opt,opts[i].pHelpMsg);
        //fflush(stdout);
        if (u32Handle != HI_NULL)
        {
            HI_OUT_Printf(u32Handle, "-%s\r\n%s\r\n", opts[i].pszOpt, opts[i].pszHelpMsg);
        }

        i++;
    }

}
#else
HI_VOID Hisnet_Server_ARG_PrintHelp(char* pszOut, ARG_OPT_S* opts)
{
    HI_S32 i = 0;

    while (opts[i].u32Type < ARG_TYPE_END)
    {
        Arg_GetRetString(pszOut, "-%s\r\n%s\r\n", opts[i].pszOpt, opts[i].pszHelpMsg);
        i++;
    }
}
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
