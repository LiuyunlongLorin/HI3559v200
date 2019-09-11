#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "hi_dcf_wrapper.h"
#include "dcf_wrapper_app.h"

extern HI_DCF_WRAPPER_INDEX g_stDCFIdx;
static HI_DCF_WRAPPER_GRP_S s_stGrp = {0};
static HI_DCF_WRAPPER_OBJ_S s_stObj = {0};

HI_S32 GetLastGrp()
{
    HI_S32 s32Ret = HI_SUCCESS;
    s32Ret = HI_DCF_WRAPPER_GetLastGrp(g_stDCFIdx, &s_stGrp);
    DCF_SAMPLE_CHECK_RETURN(s32Ret);

    printf("[%s]:[%s][%d][SUCCESS]:u8ObjTypIdx:%d, u16Gnum:%d, u16ObjCnt:%d\n",
        DCF_SAMPLE_NAME,__func__, __LINE__, s_stGrp.u8ObjTypIdx, s_stGrp.u16Gnum,  s_stGrp.u16ObjCnt);

    return HI_SUCCESS;
}

HI_S32 GetFirstGrp()
{
    HI_S32 s32Ret = HI_SUCCESS;
    s32Ret = HI_DCF_WRAPPER_GetFirstGrp(g_stDCFIdx, &s_stGrp);
    DCF_SAMPLE_CHECK_RETURN(s32Ret);

    printf("[%s]:[%s][%d][SUCCESS]:u8ObjTypIdx:%d, u16Gnum:%d, u16ObjCnt:%d\n",
        DCF_SAMPLE_NAME,__func__, __LINE__, s_stGrp.u8ObjTypIdx, s_stGrp.u16Gnum,  s_stGrp.u16ObjCnt);

    return HI_SUCCESS;
}

HI_S32 GetPrevGrp()
{
    HI_S32 s32Ret = HI_SUCCESS;
    s32Ret = HI_DCF_WRAPPER_GetPrevGrp(g_stDCFIdx, DCF_SAMPLE_GRP_INTERVAL,&s_stGrp);
    DCF_SAMPLE_CHECK_RETURN(s32Ret);

    printf("[%s]:[%s][%d][SUCCESS]:u8ObjTypIdx:%d, u16Gnum:%d, u16ObjCnt:%d\n",
        DCF_SAMPLE_NAME,__func__, __LINE__, s_stGrp.u8ObjTypIdx, s_stGrp.u16Gnum,  s_stGrp.u16ObjCnt);
    return HI_SUCCESS;
}

HI_S32 GetNextGrp()
{
    HI_S32 s32Ret = HI_SUCCESS;
    s32Ret = HI_DCF_WRAPPER_GetNextGrp(g_stDCFIdx, DCF_SAMPLE_GRP_INTERVAL, &s_stGrp);
    DCF_SAMPLE_CHECK_RETURN(s32Ret);

    printf("[%s]:[%s][%d][SUCCESS]:u8ObjTypIdx:%d, u16Gnum:%d, u16ObjCnt:%d\n",
        DCF_SAMPLE_NAME,__func__, __LINE__, s_stGrp.u8ObjTypIdx, s_stGrp.u16Gnum,  s_stGrp.u16ObjCnt);

    return HI_SUCCESS;
}

HI_S32 GetCurGrpFirstObj()
{
    HI_S32 s32Ret = HI_SUCCESS;
    s32Ret = HI_DCF_WRAPPER_GetCurGrpFirstObj(g_stDCFIdx, &s_stObj);
    DCF_SAMPLE_CHECK_RETURN(s32Ret);

    printf("[%s]:[%s][%d][SUCCESS]:Fcnt:%d, MainFile:%s\n",
        DCF_SAMPLE_NAME,__func__, __LINE__, s_stObj.u8FileCnt, s_stObj.aszFilePath[0]);

    return HI_SUCCESS;
}
HI_S32 GetCurGrpLastObj()
{
    HI_S32 s32Ret = HI_SUCCESS;
    s32Ret = HI_DCF_WRAPPER_GetCurGrpLastObj(g_stDCFIdx, &s_stObj);
    DCF_SAMPLE_CHECK_RETURN(s32Ret);

    printf("[%s]:[%s][%d][SUCCESS]:Fcnt:%d, MainFile:%s\n",
        DCF_SAMPLE_NAME,__func__, __LINE__, s_stObj.u8FileCnt, s_stObj.aszFilePath[0]);

    return HI_SUCCESS;
}

HI_S32 GetCurGrpNextObj()
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HI_DCF_WRAPPER_GetCurGrpNextObj(g_stDCFIdx, &s_stObj);
    DCF_SAMPLE_CHECK_RETURN(s32Ret);

    printf("[%s]:[%s][%d][SUCCESS]:Fcnt:%d, MainFile:%s\n",
        DCF_SAMPLE_NAME,__func__, __LINE__, s_stObj.u8FileCnt, s_stObj.aszFilePath[0]);

    return HI_SUCCESS;
}
HI_S32 GetCurGrpPrevObj()
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HI_DCF_WRAPPER_GetCurGrpPrevObj(g_stDCFIdx, &s_stObj);
    DCF_SAMPLE_CHECK_RETURN(s32Ret);

    printf("[%s]:[%s][%d][SUCCESS]:Fcnt:%d, MainFile:%s\n",
        DCF_SAMPLE_NAME,__func__, __LINE__, s_stObj.u8FileCnt, s_stObj.aszFilePath[0]);

    return HI_SUCCESS;
}

HI_S32 DelFirstObj()
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HI_DCF_WRAPPER_GetFirstGrp(g_stDCFIdx, &s_stGrp);
    DCF_SAMPLE_CHECK_RETURN(s32Ret);

    s32Ret = HI_DCF_WRAPPER_GetCurGrpFirstObj(g_stDCFIdx, &s_stObj);
    DCF_SAMPLE_CHECK_RETURN(s32Ret);

    s32Ret = HI_DCF_WRAPPER_DelObj(g_stDCFIdx, s_stObj.aszFilePath[0]);
    DCF_SAMPLE_CHECK_RETURN(s32Ret);

    return HI_SUCCESS;
}

HI_S32 GetGrpCntByTyp(HI_U8 u8TypIdx)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32Cnt = 0;
    s32Ret = HI_DCF_WRAPPER_GetGrpCnt(g_stDCFIdx, u8TypIdx, &u32Cnt);
    DCF_SAMPLE_CHECK_RETURN(s32Ret);

    printf("[%s]:[%s][%d][SUCCESS]:Grp cnt:[%d,%d]\n", DCF_SAMPLE_NAME,__func__, __LINE__, u8TypIdx, u32Cnt);
    return HI_SUCCESS;
}

HI_S32 GetTotalGrpCnt()
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32Cnt = 0;
    s32Ret = HI_DCF_WRAPPER_GetTotalGrpCnt(g_stDCFIdx, &u32Cnt);
    DCF_SAMPLE_CHECK_RETURN(s32Ret);

    printf("[%s]:[%s][%d][SUCCESS]:Total Grp cnt:%d\n", DCF_SAMPLE_NAME,__func__, __LINE__, u32Cnt);
    return HI_SUCCESS;
}

HI_S32 GetObjCntByTyp(HI_U8 u8TypIdx)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32Cnt = 0;
    s32Ret = HI_DCF_WRAPPER_GetObjCnt(g_stDCFIdx, u8TypIdx, &u32Cnt);
    DCF_SAMPLE_CHECK_RETURN(s32Ret);

    printf("[%s]:[%s][%d][SUCCESS]:Obj cnt:[%d,%d]\n", DCF_SAMPLE_NAME,__func__, __LINE__, u8TypIdx, u32Cnt);
    return HI_SUCCESS;
}

HI_S32 GetTotalObjCnt()
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32Cnt = 0;
    s32Ret = HI_DCF_WRAPPER_GetTotalObjCnt(g_stDCFIdx,  &u32Cnt);
    DCF_SAMPLE_CHECK_RETURN(s32Ret);

    printf("[%s]:[%s][%d][SUCCESS]:Total Obj cnt:%d\n", DCF_SAMPLE_NAME,__func__, __LINE__, u32Cnt);
    return HI_SUCCESS;
}

HI_S32 DelCurGrp()
{
    HI_S32 s32Ret = HI_SUCCESS;
    s32Ret = HI_DCF_WRAPPER_DelCurGrp(g_stDCFIdx);
    DCF_SAMPLE_CHECK_RETURN(s32Ret);

    printf("[%s]:[%s][%d][SUCCESS]:u8ObjTypIdx:%d, u16Gnum:%d, u16ObjCnt:%d\n",
        DCF_SAMPLE_NAME,__func__, __LINE__, s_stGrp.u8ObjTypIdx, s_stGrp.u16Gnum,  s_stGrp.u16ObjCnt);

    return HI_SUCCESS;
}

HI_S32 DelCurObj()
{
    HI_S32 s32Ret = HI_SUCCESS;
    s32Ret = HI_DCF_WRAPPER_DelObj(g_stDCFIdx, s_stObj.aszFilePath[0]);
    DCF_SAMPLE_CHECK_RETURN(s32Ret);

    printf("[%s]:[%s][%d][SUCCESS]: main file:%s\n", DCF_SAMPLE_NAME,__func__, __LINE__, s_stObj.aszFilePath[0]);
    return HI_SUCCESS;
}

