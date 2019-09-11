#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "hi_dcf_wrapper.h"
#include "dcf_wrapper_app.h"

extern HI_DCF_WRAPPER_INDEX g_stDCFIdx;
static HI_BOOL s_bNewGrp = HI_TRUE;

static HI_S32 NewFile(HI_CHAR* pszFileName)
{
    FILE *fHandle = HI_NULL;
    fHandle = fopen(pszFileName, "wb+");
    if(fHandle == NULL)
    {
        printf("fopen %s error.\n", pszFileName);
        return HI_FAILURE;
    }

    if(fclose(fHandle))
    {
        perror("close file\n");
    }
    return HI_SUCCESS;
}

HI_S32 NewObj(HI_U8 u8TypIdx)
{
    HI_U8 i =0;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_DCF_WRAPPER_OBJ_S stObj = {0};

    s32Ret = HI_DCF_WRAPPER_GetNewFilePaths(g_stDCFIdx, s_bNewGrp, u8TypIdx, &stObj);
    DCF_SAMPLE_CHECK_RETURN(s32Ret);

    for(i = 0; i < stObj.u8FileCnt; i++)
    {
        s32Ret = NewFile(stObj.aszFilePath[i]);
        DCF_SAMPLE_CHECK_RETURN(s32Ret);
    }

    s32Ret = HI_DCF_WRAPPER_AddObj(g_stDCFIdx, stObj.aszFilePath[0]);
    DCF_SAMPLE_CHECK_RETURN(s32Ret);
    s_bNewGrp = HI_FALSE;
    printf("[%s]:[%s][%d][SUCCESS]:main file:%s\n", DCF_SAMPLE_NAME,__func__, __LINE__, stObj.aszFilePath[0]);
    return HI_SUCCESS;
}
