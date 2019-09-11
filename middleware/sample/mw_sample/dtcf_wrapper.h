#ifndef GENFILENAME_WRAPPER_H
#define GENFILENAME_WRAPPER_H
#include "hi_mw_type.h"
#include "hi_recorder_pro.h"

#define MW_CHECK_RETURN(s32Ret) \
    do {                                                 \
        if (HI_SUCCESS != s32Ret)                           \
        {                                               \
            return s32Ret;                          \
        }                                               \
    } while (0)

HI_S32 FileOptInit();
HI_S32 RequestFileNames(HI_MW_PTR pRecord,HI_U32 u32FileCnt, HI_CHAR (*paszFileNames)[HI_REC_FILE_NAME_LEN]);
HI_S32 AddNorm(const HI_CHAR* pszFileName);
HI_S32 MoveEmr(const HI_CHAR* pszFileName);
HI_S32 FileOptDeInit();
HI_S32 RequestDualFileNames(HI_MW_PTR pRecord,HI_U32 u32FileCnt, HI_CHAR (*paszFileNames)[HI_REC_FILE_NAME_LEN]);

#endif
