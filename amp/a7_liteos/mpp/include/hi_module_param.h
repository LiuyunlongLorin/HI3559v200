#ifndef __HI_MOD_PARAM__
#define __HI_MOD_PARAM__

#include "hi_type.h"
#include "hi_defines.h"

typedef struct hiBASE_MODULE_PARAMS_S
{
    HI_BOOL bVbForceExit;
}BASE_MODULE_PARAMS_S;


typedef struct hiHIFB_MODULE_PARAMS_S
{
    HI_CHAR video[64];
}HIFB_MODULE_PARAMS_S;

typedef struct hiVGS_MODULE_PARAMS_S
{
    HI_U32 u32MaxVgsJob;
    HI_U32 u32MaxVgsTask;
    HI_U32 u32MaxVgsNode;
    HI_U32 au32VgsEn[VGS_IP_NUM];
    HI_BOOL bVgsHdrSupport;
}VGS_MODULE_PARAMS_S;

typedef struct hiVPSS_MODULE_PARAMS_S
{
    HI_U32 u32VpssEn[VPSS_IP_NUM];
}VPSS_MODULE_PARAMS_S;

typedef struct hiGDC_MODULE_PARAMS_S
{
    HI_U32 u32MaxGdcJob;
    HI_U32 u32MaxGdcTask;
    HI_U32 u32MaxGdcNode;
    HI_U32 au32GdcEn[GDC_IP_NUM];
}GDC_MODULE_PARAMS_S;

typedef struct hiVDEC_MODULE_PARAMS_S
{
    HI_U32 u32VdecMaxChnNum;
}VDEC_MODULE_PARAMS_S;

typedef struct hiIVE_MODULE_PARAMS_S
{
    HI_BOOL bSavePowerEn;
    HI_U16  u16IveNodeNum;
    HI_U16  u16Rsv;
}IVE_MODULE_PARAMS_S;

typedef struct hiSVP_NNIE_MODULE_PARAMS_S
{
    HI_BOOL bSavePowerEn;
    HI_U16 u16NnieTskBufNum;
}SVP_NNIE_MODULE_PARAMS_S;

typedef struct hiSVP_DSP_MODULE_PARAMS_S
{
    HI_U16 u16NodeNum;
    HI_U16 u16DspInitMode;
}SVP_DSP_MODULE_PARAMS_S;

typedef struct hiACODEC_MODULE_PARAMS_S
{
    HI_U32  u32InitDelayTimeMs;
}ACODEC_MODULE_PARAMS_S;

typedef struct hiISP_MODULE_PARAMS_S
{
    HI_U32 u32PwmNum;
    HI_U32 u32ProcParam;
    HI_U32 u32UpdatePos;
    HI_U32 u32IntTimeOut;
    HI_U32 bIntBottomHalf;
    HI_U32 u32StatIntvl;

}ISP_MODULE_PARAMS_S;

typedef struct hiVENC_MODULE_PARAMS_S
{
    HI_U32 u32VencMaxChnNum;
}VENC_MODULE_PARAMS_S;

typedef struct hiVEDU_MODULE_PARAMS_S
{
    HI_U32  vedu_en[VEDU_IP_NUM];
}VEDU_MODULE_PARAMS_S;
typedef struct hiVFMW_MODULE_PARAMS_S
{
    HI_S32 s32VfmwMaxChnNum;
}VFMW_MODULE_PARAMS_S;

typedef struct hiSIL9024_MODULE_PARAMS_S
{
    int norm;
    int i2c_num;
}SIL9024_MODULE_PARAMS_S;

typedef struct hiPM_MODULE_PARAMS_S
{
    HI_BOOL  bAvspOn; /*HI_TRUE: on,HI_FALSE: off*/
    HI_BOOL  bSvpAcceleratorOn; /*HI_TRUE: on,HI_FALSE: off*/
    //	HI_U32 u32RegulatorType; /*0:DC-DC, 1:PMU*/
} PM_MODULE_PARAMS_S;
#endif

