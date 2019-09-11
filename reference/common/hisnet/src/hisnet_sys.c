#include <string.h>
#include "hisnet_cmd_sys.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

static HISNET_SYS_SVR_CB_S s_stSYSSvrCB;

HI_S32 Hisnet_SYS_GetCallBackFunction(HISNET_SYS_SVR_CB_S* pstSvrCB)
{
    if (HI_NULL != pstSvrCB)
    {
        memcpy(pstSvrCB, &s_stSYSSvrCB, sizeof(HISNET_SYS_SVR_CB_S));
    }
    return HI_SUCCESS;
}

HI_S32 Hisnet_SYS_RegCallBackFunction(const HISNET_SYS_SVR_CB_S* pstSvrCB)
{
    if (HI_NULL == pstSvrCB)
    {
        memset(&s_stSYSSvrCB,0, sizeof(HISNET_SYS_SVR_CB_S));
    }
    else
    {
        memcpy(&s_stSYSSvrCB,pstSvrCB, sizeof(HISNET_SYS_SVR_CB_S));
    }
    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
