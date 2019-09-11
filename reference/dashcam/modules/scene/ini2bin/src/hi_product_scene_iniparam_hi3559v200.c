/**
 * @file    hi_product_scene.c
 * @brief   photo picture.
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/2/2
 * @version   1.0

 */
#include <stdio.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "hi_product_scene_setparam.h"
#include "hi_product_scene_iniparam.h"
#include "hi_confaccess.h"
#include "hi_appcomm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/** \addtogroup     SCENE_CHIP */
/** @{ */  /** <!-- [SCENE_CHIP] */

#define LINE_NUM 200
static HI_S64* s_pas64LineNum = HI_NULL;

static HI_S32  PDT_SCENE_GetNumbers(HI_CHAR* pszInputLine)
{
    HI_CHAR*    pszVRBegin     = pszInputLine;
    HI_CHAR*    pszVREnd       = pszVRBegin;
    HI_U32      u32PartCount = 0;
    HI_CHAR     szPart[20] = {0};
    HI_U32      u32WholeCount = 0;
    HI_S32      s32Length = strlen(pszInputLine);
    HI_U64      u64Hex;

    HI_S32      i = 0;
    HI_BOOL     bHex = HI_FALSE;
    memset(s_pas64LineNum, 0 , sizeof(HI_S64) * LINE_NUM);
    while ((pszVREnd != NULL))
    {
        if ((u32WholeCount > s32Length) || (u32WholeCount == s32Length))
        {
            break;
        }

        while ((*pszVREnd != '|') && (*pszVREnd != '\0') && (*pszVREnd != ','))
        {
            if (*pszVREnd == 'x')
            {
                bHex= HI_TRUE;
            }
            pszVREnd++;
            u32PartCount++;
            u32WholeCount++;
        }

        memcpy(szPart, pszVRBegin, u32PartCount);

        if (bHex == HI_TRUE)
        {
            HI_CHAR* pszstr;
            u64Hex = (HI_U64)strtoll(szPart + 2, &pszstr, 16);
            s_pas64LineNum[i] = u64Hex;
        }
        else
        {
            s_pas64LineNum[i] = atoll(szPart);
        }

        memset(szPart, 0, 20);
        u32PartCount = 0;
        pszVREnd++;
        pszVRBegin = pszVREnd;
        u32WholeCount++;
        i++;
    }

    return i;
}

HI_S32 PDT_SCENE_LoadStatic3DNR(const HI_CHAR *pszIniModule, HI_PDT_SCENE_STATIC_3DNR_S* pstStatic3Dnr)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR *pszString = NULL;
    HI_U32 u32IdxM = 0;
    HI_S32 s32Value = 0;
    HI_CHAR aszIniNodeName[PDT_SCENE_INIPARAM_NODE_NAME_LEN] = {0,};

    s_pas64LineNum = (HI_S64*)malloc(sizeof(HI_S64) * LINE_NUM);

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_threednr:ThreeDNRCount");/*ThreeDNRCount*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_threednr:ThreeDNRCount", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_threednr:ThreeDNRCount failed\n");
            return HI_FAILURE;
        }
        pstStatic3Dnr->u323DNRCount = (HI_U32)s32Value;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_threednr:IsoThresh");/*IsoThresh*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbers(pszString);
        for (u32IdxM = 0; u32IdxM < pstStatic3Dnr->u323DNRCount; u32IdxM++)
        {
            pstStatic3Dnr->au323DNRIso[u32IdxM] = s_pas64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_threednr:3DnrParam_0");/*3DnrParam_0*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        for (u32IdxM = 0; u32IdxM < pstStatic3Dnr->u323DNRCount; u32IdxM++)
        {
            snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_threednr:3DnrParam_%d", u32IdxM);
            s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
            if (HI_SUCCESS != s32Ret)
            {
                MLOGE("load %s failed\n", aszIniNodeName);
                return HI_FAILURE;
            }

            if (NULL != pszString)
            {
                HI_SCENE_3DNR_VI_S *pVIX      = &(pstStatic3Dnr->ast3DNRVIValue[u32IdxM]);
                HI_SCENE_3DNR_VPSS_S *pVPX    = &(pstStatic3Dnr->ast3DNRVPSSValue[u32IdxM]);
                HI_SCENE_3DNR_VI_IEy *pViI    = &(pVIX->IEy);
                HI_SCENE_3DNR_VI_SFy *pViS    = &(pVIX->SFy);
                HI_SCENE_3DNR_VPSS_IEy  *pVpI = pVPX->IEy;
                HI_SCENE_3DNR_VPSS_SFy  *pVpS = pVPX->SFy;
                HI_SCENE_3DNR_VPSS_MDy  *pVpM = pVPX->MDy;
                HI_SCENE_3DNR_VPSS_TFy  *pVpT = pVPX->TFy;
                HI_SCENE_3DNR_VPSS_RFs  *pVpR = &pVPX->RFs;
                HI_SCENE_3DNR_VPSS_NRc  *pVpC = &pVPX->NRc;
                HI_S32 g_enGmc = 0;

                sscanf(pszString,
                       " -en                    |             %3d |             %3d |             %3d |             %3d "
                       " -nXsf1     %3d:%3d:%3d |     %3d:%3d:%3d |     %3d:%3d:%3d |     %3d:%3d:%3d |     %3d:%3d:%3d "
                       " -nXsf2     %3d:%3d:%3d |     %3d:%3d:%3d |     %3d:%3d:%3d |     %3d:%3d:%3d |     %3d:%3d:%3d "
                       " -nXsf4     %3d:%3d:%3d |     %3d:%3d:%3d |     %3d:%3d:%3d |     %3d:%3d:%3d |     %3d:%3d:%3d "
                       " -bwsf4             %3d |             %3d |                 |                 |             %3d "
                       " -kmsf4                 |                 |             %3d |             %3d |                 "
                       " -nXsf5 %3d:%3d:%3d:%3d | %3d:%3d:%3d:%3d | %3d:%3d:%3d:%3d | %3d:%3d:%3d:%3d | %3d:%3d:%3d:%3d "
                       " -dzsf5             %3d |             %3d |             %3d |             %4d |             %3d "
                       " -nXsf6 %3d:%3d:%3d:%3d | %3d:%3d:%3d:%3d | %3d:%3d:%3d:%3d | %3d:%3d:%3d:%3d | %3d:%3d:%3d:%3d "
                       " -nXsfr6    %3d:%3d:%3d |     %3d:%3d:%3d |     %3d:%3d:%3d |     %3d:%3d:%3d |     %3d:%3d:%3d "
                       "                        |                 |                 |                 |                 "
                       " -SelRt         %3d:%3d |                 |                 |                 |         %3d:%3d "
                       " -DeRt          %3d:%3d |                 |                 |                 |         %3d:%3d "
                       "                        |                 |                 |                 |                 "
                       " -TriTh             %3d |             %3d |             %3d |             %3d |             %3d "
                       " -nXsfn     %3d:%3d:%3d |     %3d:%3d:%3d |     %3d:%3d:%3d |     %3d:%3d:%3d |     %3d:%3d:%3d "
                       " -nXsth         %3d:%3d |         %3d:%3d |         %3d:%3d |         %3d:%3d |         %3d:%3d "
                       " -sfr    (0)        %3d |             %3d |             %3d |             %3d |         %3d     "
                       "                        |                 |                 |                 |                 "
                       " -tedge                 |             %3d |             %3d |                 |                 "
                       "                        |                 |                 |                 |                 "
                       " -ref                   |         %3d:%3d |                 |                 |                 "
                       " -refUpt                |             %3d |                 |                 |                 "
                       " -rftIdx                |             %3d |                 |                 |                 "
                       " -refCtl                |         %3d:%3d |                 |                 |                 "
                       "                        |                 |                 |                 |                 "
                       " -biPath                |             %3d |             %3d |                 |                 "
                       " -nXstr   (1)           |         %3d:%3d |         %3d:%3d |                 |                 "
                       " -nXsdz                 |         %3d:%3d |         %3d:%3d |                 |                 "
                       "                        |                 |                 |                 |                 "
                       " -nXtss                 |         %3d:%3d |         %3d:%3d |                 |                 "
                       " -nXtsi                 |         %3d:%3d |         %3d:%3d |                 |                 "
                       " -nXtfs                 |         %3d:%3d |         %3d:%3d |                 |                 "
                       " -nXdzm                 |         %3d:%3d |         %3d:%3d |                 |                 "
                       " -nXtdz   (3)           |         %3d:%3d |         %3d:%3d |                 |                 "
                       " -nXtdx                 |         %3d:%3d |         %3d:%3d |                 |                 "
                       "                        |                 |                 |                 |                 "
                       " -nXtfr0  (2)           |     %3d:%3d:%3d |     %3d:%3d:%3d |                 |                 "
                       "                        |     %3d:%3d:%3d |     %3d:%3d:%3d |                 |                 "
                       " -nXtfr1  (2)           |     %3d:%3d:%3d |     %3d:%3d:%3d |                 |                 "
                       "                        |     %3d:%3d:%3d |     %3d:%3d:%3d |                 |                 "
                       "                        |                 |                 |                 |                 "
                       " -mXid0                 |     %3d:%3d:%3d |     %3d:%3d:%3d |                 |                 "
                       " -mXid1                 |     %3d:%3d:%3d |     %3d:%3d:%3d |                 |                 "
                       " -mXmadz                |         %3d:%3d |         %3d:%3d |                 |                 "
                       " -mXmabr                |         %3d:%3d |         %3d:%3d |                 |                 "
                       "                        |                 |                 +------pNRc------+|                 "
                       " -AdvMath               |         %3d     |                 |  -sfc      %3d  |                 "
                       " -mXmath                |         %3d:%3d |         %3d:%3d |                 |                 "
                       "                        |                 |                 |  -ctfs     %3d  |                 "
                       "                        |                 |                 |  -tfc      %3d  |                 "
                       " -mXmate                |         %3d:%3d |         %3d:%3d |                 |                 "
                       " -mXmabw                |         %3d:%3d |         %3d:%3d |                 |                 "
                       " -mXmatw                |         %3d     |         %3d     |                 |                 "
                       " -mXmasw                |         %3d     |         %3d     |                 |                 ",
                       &pVpS[0].NRyEn, &pVpS[1].NRyEn, &pVpS[2].NRyEn, &pVpC->NRcEn,
                       &pViS->SFS1, &pViS->SFT1, &pViS->SBR1, &pVpS[0].SFS1, &pVpS[0].SFT1, &pVpS[0].SBR1, &pVpS[1].SFS1, &pVpS[1].SFT1, &pVpS[1].SBR1, &pVpS[2].SFS1, &pVpS[2].SFT1, &pVpS[2].SBR1, &pVpC->SFy.SFS1, &pVpC->SFy.SFT1, &pVpC->SFy.SBR1,
                       &pViS->SFS2, &pViS->SFT2, &pViS->SBR2, &pVpS[0].SFS2, &pVpS[0].SFT2, &pVpS[0].SBR2, &pVpS[1].SFS2, &pVpS[1].SFT2, &pVpS[1].SBR2, &pVpS[2].SFS2, &pVpS[2].SFT2, &pVpS[2].SBR2, &pVpC->SFy.SFS2, &pVpC->SFy.SFT2, &pVpC->SFy.SBR2,
                       &pViS->SFS4, &pViS->SFT4, &pViS->SBR4, &pVpS[0].SFS4, &pVpS[0].SFT4, &pVpS[0].SBR4, &pVpS[1].SFS4, &pVpS[1].SFT4, &pVpS[1].SBR4, &pVpS[2].SFS4, &pVpS[2].SFT4, &pVpS[2].SBR4, &pVpC->SFy.SFS4, &pVpC->SFy.SFT4, &pVpC->SFy.SBR4,
                       &pViS->BWSF4, &pVpS[0].BWSF4, &pVpC->SFy.BWSF4,
                       &pVpS[1].kMode, &pVpS[2].kMode,
                       &pViI->IES0, &pViI->IES1, &pViI->IES2, &pViI->IES3, &pVpI[0].IES0, &pVpI[0].IES1, &pVpI[0].IES2, &pVpI[0].IES3, &pVpI[1].IES0, &pVpI[1].IES1, &pVpI[1].IES2, &pVpI[1].IES3, &pVpI[2].IES0, &pVpI[2].IES1, &pVpI[2].IES2, &pVpI[2].IES3, &pVpC->IEy.IES0, &pVpC->IEy.IES1, &pVpC->IEy.IES2, &pVpC->IEy.IES3,
                       &pViI->IEDZ, &pVpI[0].IEDZ, &pVpI[1].IEDZ, &pVpI[2].IEDZ, &pVpC->IEy.IEDZ,
                       &pViS->SPN6, &pViS->SBN6, &pViS->PBR6, &pViS->JMODE, &pVpS[0].SPN6, &pVpS[0].SBN6, &pVpS[0].PBR6, &pVpS[0].JMODE, &pVpS[1].SPN6, &pVpS[1].SBN6, &pVpS[1].PBR6, &pVpS[1].JMODE, &pVpS[2].SPN6, &pVpS[2].SBN6, &pVpS[2].PBR6, &pVpS[2].JMODE, &pVpC->SFy.SPN6, &pVpC->SFy.SBN6, &pVpC->SFy.PBR6, &pVpC->SFy.JMODE,
                       &pViS->SFR6[0], &pViS->SFR6[1], &pViS->SFR6[2], &pVpS[0].SFR6[0], &pVpS[0].SFR6[1], &pVpS[0].SFR6[2], &pVpS[1].SFR6[0], &pVpS[1].SFR6[1], &pVpS[1].SFR6[2], &pVpS[2].SFR6[0], &pVpS[2].SFR6[1], &pVpS[2].SFR6[2], &pVpC->SFy.SFR6[0], &pVpC->SFy.SFR6[1], &pVpC->SFy.SFR6[2],
                       &pViS->SRT0, &pViS->SRT1, &pVpC->SFy.SRT0, &pVpC->SFy.SRT1,
                       &pViS->DeRate, &pViS->DeIdx, &pVpC->SFy.DeRate, &pVpC->SFy.DeIdx,
                       &pViS->TriTh, &pVpS[0].TriTh, &pVpS[1].TriTh, &pVpS[2].TriTh, &pVpC->SFy.TriTh,
                       &pViS->SFN0, &pViS->SFN1, &pViS->SFN3, &pVpS[0].SFN0, &pVpS[0].SFN1, &pVpS[0].SFN3, &pVpS[1].SFN0, &pVpS[1].SFN1, &pVpS[1].SFN3, &pVpS[2].SFN0, &pVpS[2].SFN1, &pVpS[2].SFN3, &pVpC->SFy.SFN0, &pVpC->SFy.SFN1, &pVpC->SFy.SFN3,
                       &pViS->STH1, &pViS->STH3, &pVpS[0].STH1, &pVpS[0].STH3, &pVpS[1].STH1, &pVpS[1].STH3, &pVpS[2].STH1, &pVpS[2].STH3, &pVpC->SFy.STH1, &pVpC->SFy.STH3,
                       &pViS->SFR, &pVpS[0].SFR, &pVpS[1].SFR, &pVpS[2].SFR, &pVpC->SFy.SFR,
                       &pVpT[0].tEdge, &pVpT[1].tEdge,
                       &pVpT[0].bRef, &g_enGmc,
                       &pVpR[0].RFUI,
                       &pVpT[0].RFI,
                       &pVpR[0].RFDZ, &pVpR[0].RFSLP,
                       &pVpM[0].biPath, &pVpM[1].biPath,
                       &pVpT[0].STR0, &pVpT[0].STR1, &pVpT[1].STR0, &pVpT[1].STR1,
                       &pVpT[0].SDZ0, &pVpT[0].SDZ1, &pVpT[1].SDZ0, &pVpT[1].SDZ1,
                       &pVpT[0].TSS0, &pVpT[0].TSS1, &pVpT[1].TSS0, &pVpT[1].TSS1,
                       &pVpT[0].TSI0, &pVpT[0].TSI1, &pVpT[1].TSI0, &pVpT[1].TSI1,
                       &pVpT[0].TFS0, &pVpT[0].TFS1, &pVpT[1].TFS0, &pVpT[1].TFS1,
                       &pVpT[0].DZMode0, &pVpT[0].DZMode1, &pVpT[1].DZMode0, &pVpT[1].DZMode1,
                       &pVpT[0].TDZ0, &pVpT[0].TDZ1, &pVpT[1].TDZ0, &pVpT[1].TDZ1,
                       &pVpT[0].TDX0, &pVpT[0].TDX1, &pVpT[1].TDX0, &pVpT[1].TDX1,
                       &pVpT[0].TFR0[0], &pVpT[0].TFR0[1], &pVpT[0].TFR0[2], &pVpT[1].TFR0[0], &pVpT[1].TFR0[1], &pVpT[1].TFR0[2],
                       &pVpT[0].TFR0[3], &pVpT[0].TFR0[4], &pVpT[0].TFR0[5], &pVpT[1].TFR0[3], &pVpT[1].TFR0[4], &pVpT[1].TFR0[5],
                       &pVpT[0].TFR1[0], &pVpT[0].TFR1[1], &pVpT[0].TFR1[2], &pVpT[1].TFR1[0], &pVpT[1].TFR1[1], &pVpT[1].TFR1[2],
                       &pVpT[0].TFR1[3], &pVpT[0].TFR1[4], &pVpT[0].TFR1[5], &pVpT[1].TFR1[3], &pVpT[1].TFR1[4], &pVpT[1].TFR1[5],
                       &pVpM[0].MAI00, &pVpM[0].MAI01, &pVpM[0].MAI02, &pVpM[1].MAI00, &pVpM[1].MAI01, &pVpM[1].MAI02,
                       &pVpM[0].MAI10, &pVpM[0].MAI11, &pVpM[0].MAI12, &pVpM[1].MAI10, &pVpM[1].MAI11, &pVpM[1].MAI12,
                       &pVpM[0].MADZ0, &pVpM[0].MADZ1, &pVpM[1].MADZ0, &pVpM[1].MADZ1,
                       &pVpM[0].MABR0, &pVpM[0].MABR1, &pVpM[1].MABR0, &pVpM[1].MABR1,
                       &pVpR[0].advMATH, &pVPX->pNRc.SFC,
                       &pVpM[0].MATH0, &pVpM[0].MATH1, &pVpM[1].MATH0, &pVpM[1].MATH1,
                       &pVPX->pNRc.CTFS,
                       &pVPX->pNRc.TFC,
                       &pVpM[0].MATE0, &pVpM[0].MATE1, &pVpM[1].MATE0, &pVpM[1].MATE1,
                       &pVpM[0].MABW0, &pVpM[0].MABW1, &pVpM[1].MABW0, &pVpM[1].MABW1,
                       &pVpM[0].MATW, &pVpM[1].MATW,
                       &pVpM[0].MASW, &pVpM[1].MASW
                      );
                free(pszString);
                pszString = HI_NULL;
            }
        }
    }

    free(s_pas64LineNum);
    s_pas64LineNum = HI_NULL;
    return HI_SUCCESS;
}

HI_VOID PDT_PARAM_DebugStatic3DNR(const HI_PDT_SCENE_STATIC_3DNR_S* pstStatic3Dnr)
{
    if (HI_NULL == pstStatic3Dnr)
    {
        printf("Null Pointer!.\n");
        return ;
    }


    HI_S32 i = 0;
    printf(RED"StaticThreeDnr Configure:\n"NONE);

    printf("ThreeDNRCount is %d.\n",pstStatic3Dnr->u323DNRCount);

    printf("IsoThresh is: ");
    for (i = 0; i < pstStatic3Dnr->u323DNRCount; i++)
    {
        printf("%d ",pstStatic3Dnr->au323DNRIso[i]);
    }
    printf("\n");

    for (i = 0; i < pstStatic3Dnr->u323DNRCount; i++)
    {
        printf("3DnrParam_%d is: \n",i);

        HI_SCENE_3DNR_VI_S sVIX;
        HI_SCENE_3DNR_VPSS_S sVPX;
        memcpy(&sVIX, &(pstStatic3Dnr->ast3DNRVIValue[i]), sizeof(HI_SCENE_3DNR_VI_S));
        memcpy(&sVPX, &(pstStatic3Dnr->ast3DNRVPSSValue[i]), sizeof(HI_SCENE_3DNR_VPSS_S));

        HI_SCENE_3DNR_VI_IEy *pViI    = &(sVIX.IEy);
        HI_SCENE_3DNR_VI_SFy *pViS    = &(sVIX.SFy);
        HI_SCENE_3DNR_VPSS_IEy  *pVpI = sVPX.IEy;
        HI_SCENE_3DNR_VPSS_SFy  *pVpS = sVPX.SFy;
        HI_SCENE_3DNR_VPSS_MDy  *pVpM = sVPX.MDy;
        HI_SCENE_3DNR_VPSS_TFy  *pVpT = sVPX.TFy;
        HI_SCENE_3DNR_VPSS_RFs  *pVpR = &(sVPX.RFs);
        HI_S32 g_enGmc = 0;

        printf("  -en                    |             %3d |             %3d |             %3d  \n",
                pVpS[0].NRyEn, pVpS[1].NRyEn, pVpS[2].NRyEn);
        printf("  -nXsf1     %3d:%3d:%3d |     %3d:%3d:%3d |     %3d:%3d:%3d |     %3d:%3d:%3d \n",
                pViS->SFS1, pViS->SFT1, pViS->SBR1, pVpS[0].SFS1, pVpS[0].SFT1, pVpS[0].SBR1,
                pVpS[1].SFS1, pVpS[1].SFT1, pVpS[1].SBR1, pVpS[2].SFS1, pVpS[2].SFT1, pVpS[2].SBR1);
        printf("  -nXsf2     %3d:%3d:%3d |     %3d:%3d:%3d |     %3d:%3d:%3d |     %3d:%3d:%3d \n",
                pViS->SFS2, pViS->SFT2, pViS->SBR2, pVpS[0].SFS2, pVpS[0].SFT2, pVpS[0].SBR2,
                pVpS[1].SFS2, pVpS[1].SFT2, pVpS[1].SBR2, pVpS[2].SFS2, pVpS[2].SFT2, pVpS[2].SBR2);
        printf("  -nXsf4     %3d:%3d:%3d |     %3d:%3d:%3d |     %3d:%3d:%3d |     %3d:%3d:%3d \n",
                pViS->SFS4, pViS->SFT4, pViS->SBR4, pVpS[0].SFS4, pVpS[0].SFT4, pVpS[0].SBR4,
                pVpS[1].SFS4, pVpS[1].SFT4, pVpS[1].SBR4, pVpS[2].SFS4, pVpS[2].SFT4, pVpS[2].SBR4);
        printf("  -bwsf4             %3d |             %3d |                 |                 \n",
                pViS->BWSF4, pVpS[0].BWSF4);
        printf("  -kmsf4                 |                 |             %3d |             %3d \n",
                pVpS[1].kMode, pVpS[2].kMode);
        printf("  -nXsf5 %3d:%3d:%3d:%3d | %3d:%3d:%3d:%3d | %3d:%3d:%3d:%3d | %3d:%3d:%3d:%3d \n",
                pViI->IES0, pViI->IES1, pViI->IES2, pViI->IES3, pVpI[0].IES0, pVpI[0].IES1,
                pVpI[0].IES2, pVpI[0].IES3, pVpI[1].IES0, pVpI[1].IES1, pVpI[1].IES2, pVpI[1].IES3,
                pVpI[2].IES0, pVpI[2].IES1, pVpI[2].IES2, pVpI[2].IES3);
        printf("  -dzsf5             %3d |             %3d |             %3d |             %4d \n",
                pViI->IEDZ, pVpI[0].IEDZ, pVpI[1].IEDZ, pVpI[2].IEDZ);
        printf("  -nXsf6 %3d:%3d:%3d:%3d | %3d:%3d:%3d:%3d | %3d:%3d:%3d:%3d | %3d:%3d:%3d:%3d \n",
                pViS->SPN6, pViS->SBN6, pViS->PBR6, pViS->JMODE, pVpS[0].SPN6, pVpS[0].SBN6,
                pVpS[0].PBR6, pVpS[0].JMODE, pVpS[1].SPN6, pVpS[1].SBN6, pVpS[1].PBR6, pVpS[1].JMODE,
                pVpS[2].SPN6, pVpS[2].SBN6, pVpS[2].PBR6, pVpS[2].JMODE);
        printf("  -nXsfr6    %3d:%3d:%3d |     %3d:%3d:%3d |     %3d:%3d:%3d |     %3d:%3d:%3d \n",
                pViS->SFR6[0], pViS->SFR6[1], pViS->SFR6[2], pVpS[0].SFR6[0], pVpS[0].SFR6[1],
                pVpS[0].SFR6[2], pVpS[1].SFR6[0], pVpS[1].SFR6[1], pVpS[1].SFR6[2], pVpS[2].SFR6[0],
                pVpS[2].SFR6[1], pVpS[2].SFR6[2]);
        printf("  -SelRt         %3d:%3d |                 |                 |                 \n",
                pViS->SRT0, pViS->SRT1);
        printf("  -DeRt          %3d:%3d |                 |                 |                 \n",
                pViS->DeRate, pViS->DeIdx);
        printf("  -TriTh             %3d |             %3d |             %3d |             %3d \n",
                pViS->TriTh, pVpS[0].TriTh, pVpS[1].TriTh, pVpS[2].TriTh);
        printf("  -nXsfn     %3d:%3d:%3d |     %3d:%3d:%3d |     %3d:%3d:%3d |     %3d:%3d:%3d \n",
                pViS->SFN0, pViS->SFN1, pViS->SFN3, pVpS[0].SFN0, pVpS[0].SFN1, pVpS[0].SFN3,
                pVpS[1].SFN0, pVpS[1].SFN1, pVpS[1].SFN3, pVpS[2].SFN0, pVpS[2].SFN1, pVpS[2].SFN3);
        printf("  -nXsth         %3d:%3d |         %3d:%3d |         %3d:%3d |         %3d:%3d \n",
                pViS->STH1, pViS->STH3, pVpS[0].STH1, pVpS[0].STH3,
                pVpS[1].STH1, pVpS[1].STH3, pVpS[2].STH1, pVpS[2].STH3);
        printf("  -sfr    (0)        %3d |             %3d |             %3d |             %3d \n",
                pViS->SFR, pVpS[0].SFR, pVpS[1].SFR, pVpS[2].SFR);
        printf("  -tedge                 |             %3d |             %3d |                 \n",
                pVpT[0].tEdge, pVpT[1].tEdge);
        printf("  -ref                   |         %3d:%3d |                 |                 \n",
                pVpT[0].bRef, g_enGmc);
        printf("  -rftIdx                |             %3d |                 |                 \n",
                pVpT[0].RFI);
        printf("  -refUpt                |         %3d:%3d |                 |                 \n",
                pVpR[0].bRFU, pVpR[0].RFUI);
        printf("  -refCtl                |         %3d:%3d |                 |                 \n",
                pVpR[0].RFDZ, pVpR[0].RFSLP);
        printf("  -biPath                |             %3d |             %3d |                 \n",
                pVpM[0].biPath, pVpM[1].biPath);
        printf("  -nXstr   (1)           |         %3d:%3d |         %3d:%3d |                 \n",
                pVpT[0].STR0, pVpT[0].STR1, pVpT[1].STR0, pVpT[1].STR1);
        printf("  -nXsdz                 |         %3d:%3d |         %3d:%3d |                 \n",
                pVpT[0].SDZ0, pVpT[0].SDZ1, pVpT[1].SDZ0, pVpT[1].SDZ1);
        printf("  -nXtss                 |         %3d:%3d |         %3d:%3d |                 \n",
                pVpT[0].TSS0, pVpT[0].TSS1, pVpT[1].TSS0, pVpT[1].TSS1);
        printf("  -nXtsi                 |         %3d:%3d |         %3d:%3d |                 \n",
                pVpT[0].TSI0, pVpT[0].TSI1, pVpT[1].TSI0, pVpT[1].TSI1);
        printf("  -nXtfs                 |         %3d:%3d |         %3d:%3d |                 \n",
                pVpT[0].TFS0, pVpT[0].TFS1, pVpT[1].TFS0, pVpT[1].TFS1);
        printf("  -nXtdz   (3)           |         %3d:%3d |         %3d:%3d |                 \n",
                pVpT[0].TDZ0, pVpT[0].TDZ1, pVpT[1].TDZ0, pVpT[1].TDZ1);
        printf("  -nXtdx                 |         %3d:%3d |         %3d:%3d |                 \n",
                pVpT[0].TDX0, pVpT[0].TDX1, pVpT[1].TDX0, pVpT[1].TDX1);
        printf("  -nXtfr0  (2)           |     %3d:%3d:%3d |     %3d:%3d:%3d |                 \n",
                pVpT[0].TFR0[0], pVpT[0].TFR0[1], pVpT[0].TFR0[2],
                pVpT[1].TFR0[0], pVpT[1].TFR0[1], pVpT[1].TFR0[2]);
        printf("                         |     %3d:%3d:%3d |     %3d:%3d:%3d |                 \n",
                pVpT[0].TFR0[3], pVpT[0].TFR0[4], pVpT[0].TFR0[5],
                pVpT[1].TFR0[3], pVpT[1].TFR0[4], pVpT[1].TFR0[5]);
        printf("  -nXtfr1  (2)           |     %3d:%3d:%3d |     %3d:%3d:%3d |                 \n",
                pVpT[0].TFR1[0], pVpT[0].TFR1[1], pVpT[0].TFR1[2],
                pVpT[1].TFR1[0], pVpT[1].TFR1[1], pVpT[1].TFR1[2]);
        printf("                         |     %3d:%3d:%3d |     %3d:%3d:%3d |                 \n",
                pVpT[0].TFR1[3], pVpT[0].TFR1[4], pVpT[0].TFR1[5],
                pVpT[1].TFR1[3], pVpT[1].TFR1[4], pVpT[1].TFR1[5]);
        printf("  -mXid0                 |     %3d:%3d:%3d |     %3d:%3d:%3d |                 \n",
                pVpM[0].MAI00, pVpM[0].MAI01, pVpM[0].MAI02,
                pVpM[1].MAI00, pVpM[1].MAI01, pVpM[1].MAI02);
        printf("  -mXid1                 |     %3d:%3d:%3d |     %3d:%3d:%3d |                 \n",
                pVpM[0].MAI10, pVpM[0].MAI11, pVpM[0].MAI12,
                pVpM[1].MAI10, pVpM[1].MAI11, pVpM[1].MAI12);
        printf("  -mXmadz                |         %3d:%3d |         %3d:%3d |                 \n",
                pVpM[0].MADZ0, pVpM[0].MADZ1, pVpM[1].MADZ0, pVpM[1].MADZ1);
        printf("                         |                 |                 +------pNRc------+n\n");
        printf("  -AdvMath               |         %3d     |                 |  -sfc      %3d  \n",
                pVpR[0].advMATH, sVPX.pNRc.SFC);
        printf("  -mXmath                |         %3d:%3d |         %3d:%3d |                 \n",
                pVpM[0].MATH0, pVpM[0].MATH1, pVpM[1].MATH0, pVpM[1].MATH1);
        printf("                         |                 |                 |  -ctfs     %3d  \n",
                sVPX.pNRc.CTFS);
        printf("                         |                 |                 |  -tfc      %3d  \n",
                sVPX.pNRc.TFC);
        printf("  -mXmate                |         %3d:%3d |         %3d:%3d |                 \n",
                pVpM[0].MATE0, pVpM[0].MATE1, pVpM[1].MATE0, pVpM[1].MATE1);
        printf("  -mXmabw                |         %3d:%3d |         %3d:%3d |                 \n",
                pVpM[0].MABW0, pVpM[0].MABW1, pVpM[1].MABW0, pVpM[1].MABW1);
        printf("  -mXmatw                |         %3d     |         %3d     |                 \n",
                pVpM[0].MATW,pVpM[1].MATW);
    }
}

/** @}*/  /** <!-- ==== SCENE_CHIP End ====*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
