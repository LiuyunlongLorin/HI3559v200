#ifndef __DCF_WRAPPER_APP__
#define __DCF_WRAPPER_APP__
#include "hi_type.h"

#define DCF_SAMPLE_NAME                 "DCF_SAMPLE"
#define DCF_NAMERULE_CFG_MIN_TYP_IDX    (0)
#define DCF_NAMERULE_CFG_MAX_TYP_IDX    (8)
#define DCF_SAMPLE_OBJ_LIST_LEN         (100)
#define DCF_SAMPLE_GRP_INTERVAL         (1)

#define DCF_SAMPLE_CHECK_RETURN(s32Ret) \
    do {\
        if (HI_SUCCESS != s32Ret)\
        {\
            printf("[%s]:[%s][%d]-----FAILURE:%d\n", DCF_SAMPLE_NAME,__func__, __LINE__, s32Ret);\
            return s32Ret;\
        }\
    } while (0)
/**func to start and destroy DCF module*/
HI_S32 FileOptInit();
HI_S32 FileOptDeInit();

/**func to rec*/
HI_S32 NewObj(HI_U8 u8TypIdx);

/**func to palybak on board*/
HI_S32 GetLastGrp();
HI_S32 GetFirstGrp();
HI_S32 GetPrevGrp();
HI_S32 GetNextGrp();
HI_S32 GetCurGrpFirstObj();
HI_S32 GetCurGrpLastObj();
HI_S32 GetCurGrpNextObj();
HI_S32 GetCurGrpPrevObj();
HI_S32 DelFirstObj();
HI_S32 GetGrpCntByTyp(HI_U8 u8TypIdx);
HI_S32 GetObjCntByTyp(HI_U8 u8TypIdx);
HI_S32 GetTotalGrpCnt();
HI_S32 GetTotalObjCnt();
HI_S32 DelCurGrp();
HI_S32 DelCurObj();

/**func to palybak on client*/
HI_S32 GetNextObjList();
HI_S32 GetPrevObjList();
#endif
