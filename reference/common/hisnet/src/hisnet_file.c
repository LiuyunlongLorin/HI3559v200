#include <string.h>
#include "hisnet_cmd_file.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

static HISNET_FILE_SVR_CB_S s_stFILESvrCB;

HI_S32 Hisnet_FILE_GetCallBackFunction(HISNET_FILE_SVR_CB_S* pstSvrCB)
{
    if (HI_NULL != pstSvrCB)
    {
        memcpy(pstSvrCB, &s_stFILESvrCB, sizeof(HISNET_FILE_SVR_CB_S));
    }
    return HI_SUCCESS;
}

HI_S32 Hisnet_FILE_RegCallBackFunction(const HISNET_FILE_SVR_CB_S* pstSvrCB)
{
    if (HI_NULL == pstSvrCB)
    {
        memset(&s_stFILESvrCB,0, sizeof(HISNET_FILE_SVR_CB_S));
    }
    else
    {
        memcpy(&s_stFILESvrCB,pstSvrCB, sizeof(HISNET_FILE_SVR_CB_S));
    }
    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
