#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "hi_dcf_wrapper.h"
#include "dcf_wrapper_app.h"

extern HI_DCF_WRAPPER_INDEX g_stDCFIdx;
static HI_DCF_WRAPPER_OBJ_S s_stObjListTail = {0};

HI_S32 GetNextObjList()
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_DCF_WRAPPER_OBJ_LIST_S  stObjList = {0};
    stObjList.apstObj = (HI_DCF_WRAPPER_OBJ_S*)malloc(DCF_SAMPLE_OBJ_LIST_LEN*sizeof(HI_DCF_WRAPPER_OBJ_S));
    if (!stObjList.apstObj)
    {
        printf("errno:not enough memory\n");
        return HI_FAILURE;
    }
    memset(stObjList.apstObj, 0x00, DCF_SAMPLE_OBJ_LIST_LEN*sizeof(HI_DCF_WRAPPER_OBJ_S));

    s32Ret = HI_DCF_WRAPPER_GetNextObjList(g_stDCFIdx, s_stObjListTail.aszFilePath[0],  DCF_SAMPLE_OBJ_LIST_LEN, &stObjList);
    DCF_SAMPLE_CHECK_RETURN(s32Ret);

    memcpy(&s_stObjListTail, &(stObjList.apstObj[stObjList.u32ObjCnt - 1]), sizeof(HI_DCF_WRAPPER_OBJ_S));

    printf("[%s]:[%s][%d][SUCCESS]:Objcnt:%d, Tail Obj of list:%s\n",
        DCF_SAMPLE_NAME,__func__, __LINE__, stObjList.u32ObjCnt, s_stObjListTail.aszFilePath[0]);
    free(stObjList.apstObj);
    stObjList.apstObj = HI_NULL;
    return HI_SUCCESS;
}

HI_S32 GetPrevObjList()
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_DCF_WRAPPER_OBJ_LIST_S  stObjList = {0};
    stObjList.apstObj = (HI_DCF_WRAPPER_OBJ_S*)malloc(DCF_SAMPLE_OBJ_LIST_LEN*sizeof(HI_DCF_WRAPPER_OBJ_S));
    if (!stObjList.apstObj)
    {
        printf("errno:not enough memory\n");
        return HI_FAILURE;
    }
    memset(stObjList.apstObj, 0x00, DCF_SAMPLE_OBJ_LIST_LEN*sizeof(HI_DCF_WRAPPER_OBJ_S));

    s32Ret = HI_DCF_WRAPPER_GetPrevObjList(g_stDCFIdx, s_stObjListTail.aszFilePath[0],  DCF_SAMPLE_OBJ_LIST_LEN, &stObjList);
    DCF_SAMPLE_CHECK_RETURN(s32Ret);

    memcpy(&s_stObjListTail, &(stObjList.apstObj[stObjList.u32ObjCnt - 1]), sizeof(HI_DCF_WRAPPER_OBJ_S));

    printf("[%s]:[%s][%d][SUCCESS]:Objcnt:%d, Tail Obj of list:%s\n",
        DCF_SAMPLE_NAME,__func__, __LINE__, stObjList.u32ObjCnt, s_stObjListTail.aszFilePath[0]);
    free(stObjList.apstObj);
    stObjList.apstObj = HI_NULL;
    return HI_SUCCESS;
}
