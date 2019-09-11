
#ifndef __HAL_MEDIA_FREQ_H__
#define __HAL_MEDIA_FREQ_H__

#include "hi_type.h"
#include "hal_pm_reg.h"
#include "pm_ext.h"
#include "hi_comm_pm.h"

#define MEDIA_PM_LEVEL_NUM 14

typedef enum {
    VI_F214M = 0,
    VI_F300M,
    VI_F340M,
    VI_F396M,
    VI_F500M,
    VI_F600M,

} HI_VI_FREQ_E;

typedef enum {
    PIPEBE_F214M = 0,
    PIPEBE_F300M,
    PIPEBE_F340M,
    PIPEBE_FVI,

} HI_PIPEBE_FREQ_E;

typedef enum {
    VPSS_F214M = 0,
    VPSS_F300M,
    VPSS_F396M,

} HI_VPSS_FREQ_E;

typedef enum {
    VEDU_F396M = 0,
    VEDU_F500M,
    VEDU_F594M,

} HI_VEDU_FREQ_E;

typedef enum {
    VDH_F396M = 0,
    VDH_F500M,
    VDH_F594M,
} HI_VDH_FREQ_E;


typedef enum {
    JPGE_F400M = 0,

} HI_JPGE_FREQ_E;

typedef enum {
    VDP_F198M = 0,
    VDP_F300M,

} HI_VDP_FREQ_E;

typedef enum {
    VGS_F300M = 0,
    VGS_F396M,
    VGS_F500M,

} HI_VGS_FREQ_E;

typedef enum {
    GDC_F340M = 0,
    GDC_F400M,
    GDC_F475M,

} HI_GDC_FREQ_E;

typedef enum {
    AVSP_F300M = 0,

} HI_AVSP_FREQ_E;

typedef enum {
    TDE_F150M = 0,

} HI_TDE_FREQ_E;

typedef enum {
    AIAO_F1188M = 0,
    AIAO_F1500M,

} HI_AIAO_FREQ_E;


typedef struct {
    HI_VI_FREQ_E          enVI[VI_MAX_DEV_NUM];
    HI_PIPEBE_FREQ_E   enPipeBE;
    HI_VPSS_FREQ_E      enVPSS;
    HI_VEDU_FREQ_E      enVEDU;
    HI_VDH_FREQ_E        enVDH;
    HI_JPGE_FREQ_E       enJPGE;
    HI_VDP_FREQ_E        enVDP;
    HI_VGS_FREQ_E        enVGS;
    HI_GDC_FREQ_E       enGDC;
    HI_TDE_FREQ_E        enTDE;
    HI_AVSP_FREQ_E      enAVSP;
    HI_AIAO_FREQ_E      enAIAO;
} HI_MEDIA_FREQ_S;


typedef enum {
    VI,
    BE,
    VPSS,
    VEDU,
    VDH,
    JPGE,
    VDP,
    GDC,
    VGS,
    TDE,
    AVSP,
    AIAO,
} HI_MEDIA_MODULE_E;

typedef enum hiEN_PM_ERR_CODE_E {
    MPP_EN_PM_ILLEGAL_PARAM = 0X40,
    MPP_EN_PM_NULL_PTR = 0X41,
    MPP_EN_PM_OVERRANGE = 0x42,
    MPP_EN_PM_SET_DVFS = 0x43,
} EN_PM_ERR_CODE_E;


HI_S32 SetMediaFreqReg(const HI_MEDIA_FREQ_S* pstMediaFreq);

HI_S32 GetMediaFreqReg(HI_MEDIA_FREQ_S*  pstMediaFreq);

HI_S32 PM_HAL_GetMediaFreqByUsrCfg(const HI_MPI_PM_MEDIA_CFG_S* pstPmParam, HI_MEDIA_FREQ_S* pstMediaFreq);

HI_S32 PM_HAL_GetMiscPolicy(void);

void PM_HAL_GetMediaProfile(HI_MEDIA_FREQ_S* pstMediaFreq, HI_U32*  pu32Profile);

HI_S32 PM_HAL_GetMediaProfileByUsrCfg(HI_MPI_PM_MEDIA_CFG_S* pstUsrParam, HI_U32*  pu32Profile);

HI_S32 PM_HAL_SetMediaFreqByUsrCfg(const HI_MPI_PM_MEDIA_CFG_S* pstUsrParam);


#endif /*#ifndef __HAL_MEDIA_FREQ_H__*/


