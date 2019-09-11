/******************************************************************************

  Copyright (C), 2016, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : isp_main.h
  Version       : Initial Draft
  Author        : Hisilicon multimedia software group
  Created       : 2013/01/16
  Description   :
  History       :
  1.Date        : 2013/01/16
    Author      :
    Modification: Created file

******************************************************************************/

#ifndef __ISP_MAIN_H__
#define __ISP_MAIN_H__

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/stat.h>

#include <pthread.h>

#include "hi_comm_3a.h"
#include "isp_debug.h"
#include "mkp_isp.h"
#include "isp_dcfinfo.h"
#include "isp_frameinfo.h"
#include "isp_dnginfo.h"
#include "isp_block.h"
#include "vi_ext.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#define ISP_MAX_ALGS_NUM     64

typedef struct hiISP_LIB_NODE_S {
    HI_BOOL bUsed;
    ALG_LIB_S stAlgLib;
    union {
        ISP_AE_REGISTER_S stAeRegsiter;
        ISP_AWB_REGISTER_S stAwbRegsiter;
    };
} ISP_LIB_NODE_S;

typedef struct hiISP_LIB_INFO_S {
    HI_U32         u32ActiveLib; /* the lib used present. */
    ISP_LIB_NODE_S astLibs[MAX_REGISTER_ALG_LIB_NUM];
} ISP_LIB_INFO_S;

typedef struct hiISP_ALG_FUNC_S {
    HI_S32 (*pfn_alg_init)(VI_PIPE ViPipe, HI_VOID *pRegCfg);
    HI_S32 (*pfn_alg_run)(VI_PIPE ViPipe,
                          const HI_VOID *pStatInfo,
                          HI_VOID *pRegCfg,
                          HI_S32 s32Rsv);
    HI_S32 (*pfn_alg_ctrl)(VI_PIPE ViPipe, HI_U32 u32Cmd, HI_VOID *pValue);
    HI_S32 (*pfn_alg_exit)(VI_PIPE ViPipe);
} ISP_ALG_FUNC_S;

typedef struct hiISP_ALG_NODE_S {
    HI_BOOL        bUsed;
    ISP_ALG_MOD_E  enAlgType;
    ISP_ALG_FUNC_S stAlgFunc;
} ISP_ALG_NODE_S;

#define ISP_SYNC_ISO_BUF_MAX 6

typedef struct hiISP_DNG_CCM_S {
    HI_U16 au16HighCcm[9];
    HI_U16 au16LowCcm[9];
    HI_U16 u16HighColorTemp;
    HI_U16 u16LowColorTemp;
} ISP_DNG_CCM_S;

typedef struct hiISP_LINKAGE_S {
    HI_BOOL bDefectPixel;  // each alg need to check this status

    HI_U32  u32IspDgain;
    HI_U32  u32Again;
    HI_U32  u32Dgain;
    HI_U32  u32IspDgainShift;
    HI_U32  u32Iso;
    HI_U32  u32IsoMax;
    HI_U32  u32SensorIso;
    HI_U32  u32ExpRatio;
    HI_U32  au32ExpRatio[3];
    HI_U32  au32WDRGain[4];
    HI_U32  u32Inttime;
    HI_U32  u32PirisGain;
    HI_U8   u8AERunInterval;
    ISP_FSWDR_MODE_E enFSWDRMode;
    ISP_FSWDR_MODE_E enPreFSWDRMode;
    HI_U32  u32ColorTemp;
    HI_U32  au32WhiteBalanceGain[4];
    HI_U16  au16CCM[9];
    HI_U8   u8Saturation;
    HI_BOOL bSnapState;
    HI_BOOL bProTrigger;
    HI_BOOL bLoadCCM;
    SNAP_TYPE_E enSnapType;
    ISP_SNAP_PIPE_MODE_E enSnapPipeMode;
    HI_S32  s32PreviewPipeId;
    HI_S32  s32PicturePipeId;
    ISP_RUNNING_MODE_E enPreviewRunningMode;
    ISP_RUNNING_MODE_E enPictureRunningMode;
    HI_U32  u32UpdatePos;
    HI_U32  au32SyncIsoBuf[ISP_SYNC_ISO_BUF_MAX];
	HI_U32  au32SyncAllExpRatioBuf[ISP_SYNC_ISO_BUF_MAX];
	HI_U32  au32SyncExpRatioBuf[3][ISP_SYNC_ISO_BUF_MAX];
    HI_U32  au8ProIndexBuf[ISP_SYNC_ISO_BUF_MAX];
    HI_U8   u8ProIndex;
    HI_U32  u32IsoDoneFrmcnt;
    HI_U8   u8Cfg2ValidDelayMax;
    HI_BOOL bFlicker;
    HI_U8   u8Freqresult;
    HI_BOOL bStatReady;
    HI_BOOL bRunOnce;
} ISP_LINKAGE_S;

typedef struct hiISP_PARA_REC_S {
    HI_BOOL bWDRCfg;
    HI_BOOL bPubCfg;
    HI_BOOL bInit;
    HI_BOOL bRun;
    HI_BOOL bStitchSync;

    HI_BOOL bRunEn;
} ISP_PARA_REC_S;

typedef struct hiISP_SPECIAL_OPT_S {
    HI_BOOL abBeOnSttUpdate[ISP_MAX_BE_NUM];
    HI_BOOL bFeSttUpdate;
} ISP_SPECIAL_OPT_S;

typedef struct hiISP_SENSOR_IMAGE_MODE_S {
    HI_U16   u16Width;
    HI_U16   u16Height;
    HI_FLOAT f32Fps;
    HI_U8 u8SnsMode;
} ISP_SENSOR_IMAGE_MODE_S;
typedef struct hiISP_BAS_OUT_IMAGE_MODE_S {
    HI_U16 u16OutWidth;
    HI_U16 u16OutHeight;
} ISP_BAS_OUT_IMAGE_MODE_S;

typedef union hiISP_ALG_KEY_U {
    HI_U64 u64Key;
    struct
    {
        HI_U64  bit1Ae          : 1 ;   /* [0] */
        HI_U64  bit1Awb         : 1 ;   /* [1] */
        HI_U64  bit1Af          : 1 ;   /* [2] */
        HI_U64  bit1Dp          : 1 ;   /* [3] */
        HI_U64  bit1Ge          : 1 ;   /* [4] */
        HI_U64  bit1Wdr         : 1 ;   /* [5] */
        HI_U64  bit1Expander    : 1 ;   /* [6] */
        HI_U64  bit1Blc         : 1 ;   /* [7] */
        HI_U64  bit1Bayernr     : 1 ;   /* [8] */
        HI_U64  bit1Split       : 1 ;   /* [9] */
        HI_U64  bit1Flicker     : 1 ;   /* [10] */
        HI_U64  bit1Dg          : 1 ;   /* [11] */
        HI_U64  bit1Hrs         : 1 ;   /* [12] */
        HI_U64  bit1FeLsc       : 1 ;   /* [13] */
        HI_U64  bit1Lsc         : 1 ;   /* [14] */
        HI_U64  bit1RLsc        : 1 ;   /* [15] */
        HI_U64  bit1Rc          : 1 ;   /* [16] */
        HI_U64  bit1Drc         : 1 ;   /* [17] */
        HI_U64  bit1Dehaze      : 1 ;   /* [18] */
        HI_U64  bit1LCac        : 1 ;   /* [19] */
        HI_U64  bit1GCac        : 1 ;   /* [20] */
        HI_U64  bit1Demosaic    : 1 ;   /* [21] */
        HI_U64  bit1Fcr         : 1 ;   /* [22] */
        HI_U64  bit1Gamma       : 1 ;   /* [23] */
        HI_U64  bit1Csc         : 1 ;   /* [24] */
        HI_U64  bit1Ca          : 1 ;   /* [25] */
        HI_U64  bit1Fpn         : 1;    /* [26] */
        HI_U64  bit1Sharpen     : 1;    /* [27] */
        HI_U64  bit1EdgeMark    : 1 ;   /* [28] */
        HI_U64  bit1Mcds        : 1 ;   /* [29] */
        HI_U64  bit1Ldci        : 1 ;   /* [30] */
        HI_U64  bit1PreGamma    : 1 ;   /* [31] */
        HI_U64  bit1FeLogLUT    : 1 ;   /* [32] */
        HI_U64  bit1LogLUT      : 1 ;   /* [33] */
        HI_U64  bit1Clut        : 1 ;   /* [34] */
        HI_U64  bit1Hlc         : 1 ;   /* [35] */
        HI_U64  bit1Detail      : 1 ;   /* [36] */
        HI_U64  bit1Rgbir       : 1 ;   /* [37] */
        HI_U64  bit26Rsv        : 26;   /* [38:63] */
    };
} ISP_ALG_KEY_U;

typedef struct hiISP_CTX_S {
    /* 1. ctrl param */
    HI_BOOL                  bMemInit;
    HI_BOOL                  bSnsReg;
    HI_BOOL                  bISPYuvMode;
    ISP_PARA_REC_S           stIspParaRec;
    RECT_S                   stSysRect;
    ISP_BLOCK_ATTR_S         stBlockAttr;
    pthread_mutex_t          stLock;
    ISP_SPECIAL_OPT_S        stSpecialOpt;

    /* 2. algorithm ctrl param */
    HI_BOOL                  bFreezeFw;
    //HI_BOOL                  bChangeMode;
    HI_BOOL                  bChangeWdrMode;
    HI_BOOL                  bChangeImageMode;
    HI_BOOL                  bChangeIspRes;
    HI_BOOL                  bIsp0P2En;
    HI_U8                    u8Count;
    HI_U8                    u8PreSnsWDRMode;
    HI_U8                    u8SnsWDRMode;
    HI_U32                   u32FrameCnt;    /* frame count for 3a libs which can decide the interval of calculate. */
    ISP_BAYER_FORMAT_E       enBayer;
    ISP_ALG_KEY_U            unAlgKey;

    HI_U8                    u8IspImageModeFlag;
    ISP_SENSOR_IMAGE_MODE_S  stPreSnsImageMode;
    ISP_SENSOR_IMAGE_MODE_S  stSnsImageMode;

    ISP_AE_RESULT_S          stAeResult;
    ISP_AWB_RESULT_S         stAwbResult;

    ISP_BAS_OUT_IMAGE_MODE_S stPreBasImageMode;
    ISP_BAS_OUT_IMAGE_MODE_S stCurBasImageMode;
    SIZE_S                   stPipeSize;
    VI_PIPE_HDR_ATTR_S       stHdrAttr;
    VI_PIPE_WDR_ATTR_S       stWdrAttr;
    VI_FUSION_GROUP_S        stFusionAttr;
    ISP_STITCH_ATTR_S        stStitchAttr;
    COLOR_GAMUT_E            eColorGamut;
    ISP_SMART_INFO_S         stSmartInfo;

    /* 3. 3a register info */
    ISP_BIND_ATTR_S          stBindAttr;
    ISP_LIB_INFO_S           stAeLibInfo;
    ISP_LIB_INFO_S           stAwbLibInfo;
    ISP_LIB_INFO_S           stAfLibInfo;

    /* 4. algorithm ctrl */
    ISP_ALG_NODE_S           astAlgs[ISP_MAX_ALGS_NUM];
    ISP_LINKAGE_S            stLinkage;

    /* 5. debug info */
    ISP_DBG_CTRL_S           stIspDbg;

    /* 6. DCF info*/
    ISP_TRANS_INFO_BUF_S     stIspTransInfo;
    ISP_PRO_INFO_BUF_S       stIspProInfo;

    ISP_UPDATE_INFO_CTRL_S   stUpdateInfoCtrl;
    ISP_FRAME_INFO_CTRL_S    stFrameInfoCtrl;
    ISP_ATTACH_INFO_CTRL_S   stAttachInfoCtrl;
    ISP_GAMMUT_INFO_CTRL_S   stGamutInfoCtrl;
    ISP_PRO_NR_PARAM_CTRL_S  stProNrParamCtrl;
    ISP_PRO_SHP_PARAM_CTRL_S stProShpParamCtrl;
    ISP_DCF_UPDATE_INFO_S    stDcfUpdateInfo;
    ISP_FRAME_INFO_S         stFrameInfo;

    /* DNG info*/
    ISP_DNG_INFO_CTRL_S      stDngInfoCtrl;
    ISP_DNG_COLORPARAM_S     stPreDngColorParam;
    ISP_DNG_CCM_S            stPreDngCcm;

    ISP_PRO_PARAM_S          stProParam;
    HI_U8                    u8ProFrmCnt;
    ISP_SNAP_INFO_S          stSnapIspInfo;

    /* 7. ViProc/BE Addr */
    HI_VOID                 *pIspBeVirtAddr[ISP_STRIPING_MAX_NUM];
    HI_VOID                 *pViProcVirtAddr[ISP_STRIPING_MAX_NUM];
} ISP_CTX_S;

extern HI_S32    g_as32IspFd[ISP_MAX_PIPE_NUM];
extern ISP_CTX_S g_astIspCtx[ISP_MAX_PIPE_NUM];
extern ISP_REGCFG_S *g_pastRegCfgCtx[ISP_MAX_PIPE_NUM];
extern const HI_U32 g_au32IsoLut[ISP_AUTO_ISO_STRENGTH_NUM];

#define ISP_REGCFG_GET_CTX(dev, pstCtx) (pstCtx = g_pastRegCfgCtx[dev])
#define ISP_REGCFG_SET_CTX(dev, pstCtx) (g_pastRegCfgCtx[dev] = pstCtx)
#define ISP_REGCFG_RESET_CTX(dev)       (g_pastRegCfgCtx[dev] = HI_NULL)

#define ISP_MONO_FUSION_MODE(dev) ((HI_TRUE == g_astIspCtx[dev].stFusionAttr.bFusionEn) && \
                                   (dev == g_astIspCtx[dev].stFusionAttr.PipeId[1]))

#define ISP_CHECK_OPEN(dev)                    \
    do {                                       \
        HI_S32 s32Ret = ISP_CheckDevOpen(dev); \
        if (HI_SUCCESS != s32Ret) {            \
            return s32Ret;                     \
        }                                      \
    } while (0)

#define ISP_CHECK_MEM_INIT(dev)                \
    do {                                       \
        HI_S32 s32Ret = ISP_CheckMemInit(dev); \
        if (HI_SUCCESS != s32Ret) {            \
            return s32Ret;                     \
        }                                      \
    } while (0)

#define ISP_CHECK_ISP_WDR_CFG(dev)                                            \
    do {                                                                      \
        if (g_astIspCtx[dev].stIspParaRec.bWDRCfg != HI_TRUE) {               \
            ISP_TRACE(HI_DBG_ERR, "Isp[%d] WDR mode doesn't config!\n", dev); \
            return HI_ERR_ISP_ATTR_NOT_CFG;                                   \
        }                                                                     \
    } while (0)

#define ISP_CHECK_ISP_PUB_ATTR_CFG(dev)                                       \
    do {                                                                      \
        if (g_astIspCtx[dev].stIspParaRec.bPubCfg != HI_TRUE) {               \
            ISP_TRACE(HI_DBG_ERR, "Isp[%d] Pub Attr doesn't config!\n", dev); \
            return HI_ERR_ISP_ATTR_NOT_CFG;                                   \
        }                                                                     \
    } while (0)

#define ISP_CHECK_ISP_INIT(dev)                                           \
    do {                                                                  \
        if (g_astIspCtx[dev].stIspParaRec.bInit != HI_TRUE) {             \
            ISP_TRACE(HI_DBG_ERR, "Isp[%d] doesn't initialized!\n", dev); \
            return HI_ERR_ISP_NOT_INIT;                                   \
        }                                                                 \
    } while (0)

#define ISP_CHECK_OFFLINE_MODE(dev)                                                \
    do {                                                                           \
        if ((IS_ONLINE_MODE(g_astIspCtx[dev].stBlockAttr.enIspRunningMode)) ||  \
            (IS_SIDEBYSIDE_MODE(g_astIspCtx[dev].stBlockAttr.enIspRunningMode))) { \
            return HI_SUCCESS;                                                     \
        }                                                                          \
    } while (0)

#define ISP_GET_CTX(dev, pstIspCtx)    \
    do {                               \
        pstIspCtx = &g_astIspCtx[dev]; \
    } while (0)

/*
 * HIGH LEVEL functions used to set-up and run ISP.
 */
/*
 * This function executes the auto-exposure algorithm
 * and its results are used to drive sensor parameters,
 * drc engine and noise reduction algorithms,
 * Results are based on the collected image histogram data.
 */
HI_S32 ISP_Run(VI_PIPE ViPipe);
HI_S32 ISP_Exit(VI_PIPE ViPipe);
HI_S32 ISP_StitchSyncExit(VI_PIPE ViPipe);

HI_S32 ISP_SwitchWDR(VI_PIPE ViPipe);
HI_S32 ISP_SwitchWDRMode(VI_PIPE ViPipe);
HI_S32 ISP_SwitchImageMode(VI_PIPE ViPipe);
HI_S32 ISP_SwitchMode(VI_PIPE ViPipe);

HI_S32 ISP_WDRCfgSet(VI_PIPE ViPipe);
HI_U32 ISP_FrameCntGet(VI_PIPE ViPipe);

HI_S32 ISP_CheckDevOpen(ISP_DEV dev);
HI_S32 ISP_CheckMemInit(ISP_DEV dev);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* __ISP_MAIN_H__ */
