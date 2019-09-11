/******************************************************************************

  Copyright (C), 2016, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : isp_ext.h
  Version       : Initial Draft
  Author        : Hisilicon multimedia software group
  Created       : 2013/07/17
  Description   :
  History       :
  1.Date        : 2013/07/17
    Author      :
    Modification: Created file

******************************************************************************/
#ifndef __ISP_EXT_H__
#define __ISP_EXT_H__

#include "hi_type.h"
#include "hi_comm_isp.h"
#include "mod_ext.h"
#include "mkp_isp.h"
#include "hi_osal.h"
#include "isp_list.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

typedef enum hiISP_BUS_TYPE_E
{
    ISP_BUS_TYPE_I2C = 0,
    ISP_BUS_TYPE_SSP,

    ISP_BUS_TYPE_BUTT,
} ISP_BUS_TYPE_E;

typedef struct hiISP_PRO_CTRL_S
{
    HI_U8  u8Vcnum;
} ISP_PRO_CTRL_S;

typedef struct hiISP_BUS_CALLBACK_S
{
    HI_S32  (*pfnISPWriteI2CData) (HI_U8 u8I2cDev, HI_U8 u8DevAddr, HI_U32 u32RegAddr,
                                   HI_U32 u32RegAddrByteNum, HI_U32 u32Data, HI_U32 u32DataByteNum);
    HI_S32  (*pfnISPWriteSSPData) (HI_U32 u32SpiDev, HI_U32 u32SpiCsn,
                                   HI_U32 u32DevAddr, HI_U32 u32DevAddrByteNum,
                                   HI_U32 u32RegAddr, HI_U32 u32RegAddrByteNum, HI_U32 u32Data, HI_U32 u32DataByteNum);
} ISP_BUS_CALLBACK_S;

typedef struct hiISP_PIRIS_CALLBACK_S
{
    HI_S32  (*pfn_piris_gpio_update)(HI_S32 ViPipe, const HI_S32 *s32Pos);
} ISP_PIRIS_CALLBACK_S;

typedef struct hiISP_VIBUS_CALLBACK_S
{
    HI_S32  (*pfnIspBeStaRead)(HI_S32 ViPipe, ISP_BE_STAT_S *pstBeStat);
    HI_S32  (*pfnIspBeCfgRead)(HI_S32 ViPipe, ISP_REG_CFG_S *pstRegCfgInfo);
} ISP_VIBUS_CALLBACK_S;

//-- AF --------------------------------------------------------------------------------------
#define DRV_AF_ZONE_ROW                 (15)
#define DRV_AF_ZONE_COLUMN             (17)
#define DRV_WDR_CHN_MAX                 (4)

typedef struct hiISP_DRV_FOCUS_ZONE_S
{
    HI_U16  u16v1;
    HI_U16  u16h1;
    HI_U16  u16v2;
    HI_U16  u16h2;
    HI_U16  u16y;
    HI_U16  u16HlCnt;
} ISP_DRV_FOCUS_ZONE_S;

typedef struct hiISP_DRV_FE_FOCUS_STATISTICS_S
{
    ISP_DRV_FOCUS_ZONE_S stZoneMetrics[DRV_WDR_CHN_MAX][DRV_AF_ZONE_ROW][DRV_AF_ZONE_COLUMN]; /*RO, The zoned measure of contrast*/
} ISP_DRV_FE_FOCUS_STATISTICS_S;

typedef struct hiISP_DRV_BE_FOCUS_STATISTICS_S
{
    ISP_DRV_FOCUS_ZONE_S stZoneMetrics[DRV_AF_ZONE_ROW][DRV_AF_ZONE_COLUMN]; /*RO, The zoned measure of contrast*/
} ISP_DRV_BE_FOCUS_STATISTICS_S;

typedef struct hiISP_DRV_AF_STATISTICS_S
{
    ISP_DRV_FE_FOCUS_STATISTICS_S      stFEAFStat;
    ISP_DRV_BE_FOCUS_STATISTICS_S      stBEAFStat;
} ISP_DRV_AF_STATISTICS_S;

//----------------------------------------------------------------------------------------
// isp sync task
typedef enum hiISP_SYNC_TSK_METHOD_E
{
    ISP_SYNC_TSK_METHOD_HW_IRQ = 0,
    ISP_SYNC_TSK_METHOD_WORKQUE,

    ISP_SYNC_TSK_METHOD_BUTT

} ISP_SYNC_TSK_METHOD_E;

typedef struct hiISP_SYNC_TASK_NODE_S
{
    ISP_SYNC_TSK_METHOD_E enMethod;
    HI_S32 ( *pfnIspSyncTskCallBack ) (HI_U64 u64Data);
    HI_U64 u64Data;
    const char *pszId;
    struct osal_list_head list;
    ISP_DRV_AF_STATISTICS_S     *pstFocusStat;
} ISP_SYNC_TASK_NODE_S;

typedef struct hiISP_EXPORT_FUNC_S
{
    HI_S32  (*pfnISPRegisterBusCallBack) (HI_S32 ViPipe, ISP_BUS_TYPE_E enType, ISP_BUS_CALLBACK_S *pstBusCb);
    HI_S32  (*pfnISPRegisterPirisCallBack) (HI_S32 ViPipe, ISP_PIRIS_CALLBACK_S *pstPirisCb);
    HI_S32  (*pfnISPRegisterViBusCallBack) (HI_S32 ViPipe, ISP_VIBUS_CALLBACK_S *pstViBusCb);
    HI_S32  (*pfnISP_GetDCFInfo)(VI_PIPE ViPipe, ISP_DCF_INFO_S *pstIspDCF);
    HI_S32  (*pfnISP_SetDCFInfo)(VI_PIPE ViPipe, ISP_DCF_INFO_S *pstIspDCF);
    HI_S32  (*pfnISP_GetIspUpdateInfo)(VI_PIPE ViPipe, ISP_DCF_UPDATE_INFO_S *pstIspUpdateInfo);
    HI_S32  (*pfnISP_SetIspUpdateInfo)(VI_PIPE ViPipe, ISP_DCF_UPDATE_INFO_S *pstIspUpdateInfo);
    HI_S32  (*pfnISP_GetFrameInfo)(VI_PIPE ViPipe, ISP_FRAME_INFO_S *pstIspFrame);
    HI_S32  (*pfnISP_SetFrameInfo)(VI_PIPE ViPipe, ISP_FRAME_INFO_S *pstIspFrame);
    HI_S32  (*pfnISP_GetAttachInfo)(VI_PIPE ViPipe, ISP_ATTACH_INFO_S *pstIspAttachInfo);
    HI_S32  (*pfnISP_GetColorGamutInfo)(VI_PIPE ViPipe, ISP_COLORGAMMUT_INFO_S *pstIspColorGamutInfo);
    HI_S32  (*pfnISP_GetDngImageDynamicInfo)(VI_PIPE ViPipe, DNG_IMAGE_DYNAMIC_INFO_S *pstDngImageDynamicInfo);
    HI_S32  (*pfnISP_SaveSnapConfig)(VI_PIPE ViPipe, ISP_CONFIG_INFO_S *pstSnapInfo);
    HI_S32  (*pfnISP_SetProEnable)(VI_PIPE ViPipe);
    HI_S32  (*pfnISP_DRV_GetReadyBeBuf)(VI_PIPE ViPipe, ISP_BE_WO_CFG_BUF_S *pstBeCfgBuf);
    HI_S32  (*pfnISP_DRV_PutFreeBeBuf)(VI_PIPE ViPipe, ISP_BE_WO_CFG_BUF_S *pstBeCfgBuf);
    HI_S32  (*pfnISP_DRV_HoldBusyBeBuf)(VI_PIPE ViPipe, ISP_BE_WO_CFG_BUF_S *pstBeCfgBuf);
    HI_S32  (*pfnISP_DRV_GetBeSyncPara)(VI_PIPE ViPipe, ISP_BE_SYNC_PARA_S *pstBeSyncPara);
    HI_BOOL (*pfnISP_GetProCtrl)(VI_PIPE ViPipe, ISP_PRO_CTRL_S *pstProCtrl);
    HI_S32  (*pfnISP_SetSnapAttr)(VI_PIPE ViPipe, ISP_SNAP_ATTR_S *pstSnapAttr);
    HI_S32  (*pfnISP_SetProNrParam )(VI_PIPE ViPipe, const ISP_PRO_BNR_PARAM_S *pstProNrParam);
    HI_S32  (*pfnISP_SetProShpParam )(VI_PIPE ViPipe, const ISP_PRO_SHARPEN_PARAM_S *pstProShpParam);
    HI_S32  (*pfnISP_GetProNrParam )(VI_PIPE ViPipe, ISP_PRO_BNR_PARAM_S *pstProNrParam);
    HI_S32  (*pfnISP_GetProShpParam )(VI_PIPE ViPipe, ISP_PRO_SHARPEN_PARAM_S *pstProShpParam);
    HI_S32  (*pfnISP_DRV_BeEndIntProc)(VI_PIPE ViPipe);
    HI_S32  (*pfnISPRegisterSyncTask)(VI_PIPE ViPipe, ISP_SYNC_TASK_NODE_S *pstNewNode);
    HI_S32  (*pfnISPUnRegisterSyncTask)(VI_PIPE ViPipe, ISP_SYNC_TASK_NODE_S *pstDelNode);
    int     (*pfnISP_IntBottomHalf)(int irq, void *id);
    int     (*pfnISP_ISR)(int irq, void *id);
    HI_S32  (*pfnISP_GetPubAttr)(VI_PIPE ViPipe, ISP_PUB_ATTR_S *pstPubAttr);
    HI_S32  (*pfnISP_DRV_GetRgbirFormat)(VI_PIPE ViPipe, ISP_IRBAYER_FORMAT_E *pRgbirFormat);
} ISP_EXPORT_FUNC_S;

#define CKFN_ISP() \
    (NULL != FUNC_ENTRY(ISP_EXPORT_FUNC_S, HI_ID_ISP))

#define CKFN_ISP_RegisterBusCallBack()\
    (NULL != FUNC_ENTRY(ISP_EXPORT_FUNC_S, HI_ID_ISP)->pfnISPRegisterBusCallBack)
#define CALL_ISP_RegisterBusCallBack(ViPipe,enType,pstBusCb)\
    FUNC_ENTRY(ISP_EXPORT_FUNC_S, HI_ID_ISP)->pfnISPRegisterBusCallBack(ViPipe,enType,pstBusCb)

#define CKFN_ISP_RegisterPirisCallBack()\
    (NULL != FUNC_ENTRY(ISP_EXPORT_FUNC_S, HI_ID_ISP)->pfnISPRegisterPirisCallBack)
#define CALL_ISP_RegisterPirisCallBack(ViPipe,pstPirisCb)\
    FUNC_ENTRY(ISP_EXPORT_FUNC_S, HI_ID_ISP)->pfnISPRegisterPirisCallBack(ViPipe,pstPirisCb)

#define CKFN_ISP_RegisterSnapCallBack()\
    (NULL != FUNC_ENTRY(ISP_EXPORT_FUNC_S, HI_ID_ISP)->pfnISPRegisterSnapCallBack)
#define CALL_ISP_RegisterSnapCallBack(ViPipe,pstSnapCb)\
    FUNC_ENTRY(ISP_EXPORT_FUNC_S, HI_ID_ISP)->pfnISPRegisterSnapCallBack(ViPipe,pstSnapCb)

#define CKFN_ISP_RegisterViBusCallBack()\
    (NULL != FUNC_ENTRY(ISP_EXPORT_FUNC_S, HI_ID_ISP)->pfnISPRegisterViBusCallBack)
#define CALL_ISP_RegisterViBusCallBack(ViPipe,pstViBusCb)\
    FUNC_ENTRY(ISP_EXPORT_FUNC_S, HI_ID_ISP)->pfnISPRegisterViBusCallBack(ViPipe,pstViBusCb)

#define CKFN_ISP_GetDCFInfo()\
    (NULL != FUNC_ENTRY(ISP_EXPORT_FUNC_S, HI_ID_ISP)->pfnISP_GetDCFInfo)
#define CALL_ISP_GetDCFInfo(ViPipe,pstIspDCF)\
    FUNC_ENTRY(ISP_EXPORT_FUNC_S, HI_ID_ISP)->pfnISP_GetDCFInfo(ViPipe,pstIspDCF)

#define CKFN_ISP_GetIspUpdateInfo()\
    (NULL != FUNC_ENTRY(ISP_EXPORT_FUNC_S, HI_ID_ISP)->pfnISP_GetIspUpdateInfo)
#define CALL_ISP_GetIspUpdateInfo(ViPipe,pstIspUpdateInfo)\
    FUNC_ENTRY(ISP_EXPORT_FUNC_S, HI_ID_ISP)->pfnISP_GetIspUpdateInfo(ViPipe,pstIspUpdateInfo)

#define CKFN_ISP_SetIspUpdateInfo()\
    (NULL != FUNC_ENTRY(ISP_EXPORT_FUNC_S, HI_ID_ISP)->pfnISP_SetIspUpdateInfo)
#define CALL_ISP_SetIspUpdateInfo(ViPipe,pstIspUpdateInfo)\
    FUNC_ENTRY(ISP_EXPORT_FUNC_S, HI_ID_ISP)->pfnISP_SetIspUpdateInfo(ViPipe,pstIspUpdateInfo)

#define CKFN_ISP_GetFrameInfo()\
    (NULL != FUNC_ENTRY(ISP_EXPORT_FUNC_S, HI_ID_ISP)->pfnISP_GetFrameInfo)
#define CALL_ISP_GetFrameInfo(ViPipe,pstIspFrame)\
    FUNC_ENTRY(ISP_EXPORT_FUNC_S, HI_ID_ISP)->pfnISP_GetFrameInfo(ViPipe,pstIspFrame)

#define CKFN_ISP_SetFrameInfo()\
    (NULL != FUNC_ENTRY(ISP_EXPORT_FUNC_S, HI_ID_ISP)->pfnISP_SetFrameInfo)
#define CALL_ISP_SetFrameInfo(ViPipe,pstIspFrame)\
    FUNC_ENTRY(ISP_EXPORT_FUNC_S, HI_ID_ISP)->pfnISP_SetFrameInfo(ViPipe,pstIspFrame)

#define CKFN_ISP_GetAttachInfo()\
    (NULL != FUNC_ENTRY(ISP_EXPORT_FUNC_S, HI_ID_ISP)->pfnISP_GetAttachInfo)
#define CALL_ISP_GetAttachInfo(ViPipe,pstIspAttachInfo)\
    FUNC_ENTRY(ISP_EXPORT_FUNC_S, HI_ID_ISP)->pfnISP_GetAttachInfo(ViPipe,pstIspAttachInfo)

#define CKFN_ISP_GetColorGamutInfo()\
    (NULL != FUNC_ENTRY(ISP_EXPORT_FUNC_S,HI_ID_ISP)->pfnISP_GetColorGamutInfo)
#define CALL_ISP_GetColorGamutInfo(ViPipe,pstIspColorGamutInfo)\
    FUNC_ENTRY(ISP_EXPORT_FUNC_S,HI_ID_ISP)->pfnISP_GetColorGamutInfo(ViPipe, pstIspColorGamutInfo)

#define CKFN_ISP_SetDCFInfo()\
    (NULL != FUNC_ENTRY(ISP_EXPORT_FUNC_S, HI_ID_ISP)->pfnISP_SetDCFInfo)
#define CALL_ISP_SetDCFInfo(ViPipe,pstIspDCF)\
    FUNC_ENTRY(ISP_EXPORT_FUNC_S, HI_ID_ISP)->pfnISP_SetDCFInfo(ViPipe,pstIspDCF)

#define CKFN_ISP_GetDngImageDynamicInfo()\
    (NULL != FUNC_ENTRY(ISP_EXPORT_FUNC_S, HI_ID_ISP)->pfnISP_GetDngImageDynamicInfo)
#define CALL_ISP_GetDngImageDynamicInfo(ViPipe, pstDngImageDynamicInfo)\
    FUNC_ENTRY(ISP_EXPORT_FUNC_S, HI_ID_ISP)->pfnISP_GetDngImageDynamicInfo(ViPipe, pstDngImageDynamicInfo)

#define CKFN_ISP_SaveSnapConfig()\
    (NULL != FUNC_ENTRY(ISP_EXPORT_FUNC_S, HI_ID_ISP)->pfnISP_SaveSnapConfig)
#define CALL_ISP_SaveSnapConfig(ViPipe,pstSnapInfo)\
    FUNC_ENTRY(ISP_EXPORT_FUNC_S, HI_ID_ISP)->pfnISP_SaveSnapConfig(ViPipe,pstSnapInfo)

#define CKFN_ISP_SetProEnable()\
    (NULL != FUNC_ENTRY(ISP_EXPORT_FUNC_S, HI_ID_ISP)->pfnISP_SetProEnable)
#define CALL_ISP_SetProEnable(ViPipe)\
    FUNC_ENTRY(ISP_EXPORT_FUNC_S, HI_ID_ISP)->pfnISP_SetProEnable(ViPipe)

#define CKFN_ISP_DRV_GetReadyBeBuf()\
    (NULL != FUNC_ENTRY(ISP_EXPORT_FUNC_S, HI_ID_ISP)->pfnISP_DRV_GetReadyBeBuf)
#define CALL_ISP_DRV_GetReadyBeBuf(ViPipe, pstBeCfgBuf)\
    FUNC_ENTRY(ISP_EXPORT_FUNC_S, HI_ID_ISP)->pfnISP_DRV_GetReadyBeBuf(ViPipe, pstBeCfgBuf)

#define CKFN_ISP_DRV_PutFreeBeBuf()\
    (NULL != FUNC_ENTRY(ISP_EXPORT_FUNC_S, HI_ID_ISP)->pfnISP_DRV_PutFreeBeBuf)
#define CALL_ISP_DRV_PutFreeBeBuf(ViPipe, pstBeCfgBuf)\
    FUNC_ENTRY(ISP_EXPORT_FUNC_S, HI_ID_ISP)->pfnISP_DRV_PutFreeBeBuf(ViPipe, pstBeCfgBuf)

#define CKFN_ISP_DRV_HoldBusyBeBuf()\
    (NULL != FUNC_ENTRY(ISP_EXPORT_FUNC_S, HI_ID_ISP)->pfnISP_DRV_HoldBusyBeBuf)
#define CALL_ISP_DRV_HoldBusyBeBuf(ViPipe, pstBeCfgBuf)\
    FUNC_ENTRY(ISP_EXPORT_FUNC_S, HI_ID_ISP)->pfnISP_DRV_HoldBusyBeBuf(ViPipe, pstBeCfgBuf)

#define CKFN_ISP_DRV_GetBeSyncPara()\
    (NULL != FUNC_ENTRY(ISP_EXPORT_FUNC_S, HI_ID_ISP)->pfnISP_DRV_GetBeSyncPara)
#define CALL_ISP_DRV_GetBeSyncPara(ViPipe, pstBeSyncPara)\
    FUNC_ENTRY(ISP_EXPORT_FUNC_S, HI_ID_ISP)->pfnISP_DRV_GetBeSyncPara(ViPipe, pstBeSyncPara)

#define CKFN_ISP_GetProCtrl()\
    (NULL != FUNC_ENTRY(ISP_EXPORT_FUNC_S, HI_ID_ISP)->pfnISP_GetProCtrl)
#define CALL_ISP_GetProCtrl(ViPipe,pstProCtrl)\
    FUNC_ENTRY(ISP_EXPORT_FUNC_S, HI_ID_ISP)->pfnISP_GetProCtrl(ViPipe,pstProCtrl)

#define CKFN_ISP_SetSnapAttr()\
    (NULL != FUNC_ENTRY(ISP_EXPORT_FUNC_S, HI_ID_ISP)->pfnISP_SetSnapAttr)
#define CALL_ISP_SetSnapAttr(ViPipe, pstSnapAttr)\
    FUNC_ENTRY(ISP_EXPORT_FUNC_S, HI_ID_ISP)->pfnISP_SetSnapAttr(ViPipe, pstSnapAttr)

#define CKFN_ISP_SetProNrParam()\
    (NULL != FUNC_ENTRY(ISP_EXPORT_FUNC_S, HI_ID_ISP)->pfnISP_SetProNrParam)
#define CALL_ISP_SetProNrParam(ViPipe, pstProNrParam)\
    FUNC_ENTRY(ISP_EXPORT_FUNC_S, HI_ID_ISP)->pfnISP_SetProNrParam(ViPipe, pstProNrParam)
#define CKFN_ISP_SetProShpParam()\
    (NULL != FUNC_ENTRY(ISP_EXPORT_FUNC_S, HI_ID_ISP)->pfnISP_SetProShpParam)
#define CALL_ISP_SetProShpParam(ViPipe, pstProShpParam)\
    FUNC_ENTRY(ISP_EXPORT_FUNC_S, HI_ID_ISP)->pfnISP_SetProShpParam(ViPipe, pstProShpParam)
#define CKFN_ISP_GetProNrParam()\
    (NULL != FUNC_ENTRY(ISP_EXPORT_FUNC_S, HI_ID_ISP)->pfnISP_GetProNrParam)
#define CALL_ISP_GetProNrParam(ViPipe, pstProNrParam)\
    FUNC_ENTRY(ISP_EXPORT_FUNC_S, HI_ID_ISP)->pfnISP_GetProNrParam(ViPipe, pstProNrParam)
#define CKFN_ISP_GetProShpParam()\
    (NULL != FUNC_ENTRY(ISP_EXPORT_FUNC_S, HI_ID_ISP)->pfnISP_GetProShpParam)
#define CALL_ISP_GetProShpParam(ViPipe, pstProShpParam)\
    FUNC_ENTRY(ISP_EXPORT_FUNC_S, HI_ID_ISP)->pfnISP_GetProShpParam(ViPipe, pstProShpParam)
#define CKFN_ISP_DRV_BeEndIntProc()\
    (NULL != FUNC_ENTRY(ISP_EXPORT_FUNC_S, HI_ID_ISP)->pfnISP_DRV_BeEndIntProc)
#define CALL_ISP_DRV_BeEndIntProc(ViPipe)\
    FUNC_ENTRY(ISP_EXPORT_FUNC_S, HI_ID_ISP)->pfnISP_DRV_BeEndIntProc(ViPipe)

#define CKFN_ISP_GetPubAttr()\
    (NULL != FUNC_ENTRY(ISP_EXPORT_FUNC_S, HI_ID_ISP)->pfnISP_GetPubAttr)
#define CALL_ISP_GetPubAttr(ViPipe, pstPubAttr)\
    FUNC_ENTRY(ISP_EXPORT_FUNC_S, HI_ID_ISP)->pfnISP_GetPubAttr(ViPipe, pstPubAttr)

#define CKFN_ISP_RegisterSyncTask()\
    (NULL != FUNC_ENTRY(ISP_EXPORT_FUNC_S, HI_ID_ISP)->pfnISPRegisterSyncTask)
#define CALL_ISP_RegisterSyncTask(ViPipe, pstNode)\
    FUNC_ENTRY(ISP_EXPORT_FUNC_S, HI_ID_ISP)->pfnISPRegisterSyncTask(ViPipe, pstNode)

#define CKFN_ISP_UnRegisterSyncTask()\
    (NULL != FUNC_ENTRY(ISP_EXPORT_FUNC_S, HI_ID_ISP)->pfnISPUnRegisterSyncTask)
#define CALL_ISP_UnRegisterSyncTask(ViPipe, pstNode)\
    FUNC_ENTRY(ISP_EXPORT_FUNC_S, HI_ID_ISP)->pfnISPUnRegisterSyncTask(ViPipe, pstNode)

#define CKFN_ISP_IntBottomHalf()\
    (NULL != FUNC_ENTRY(ISP_EXPORT_FUNC_S, HI_ID_ISP)->pfnISP_IntBottomHalf)
#define CALL_ISP_IntBottomHalf(irq, id)\
    FUNC_ENTRY(ISP_EXPORT_FUNC_S, HI_ID_ISP)->pfnISP_IntBottomHalf(irq, id)

#define CKFN_ISP_ISR()\
    (NULL != FUNC_ENTRY(ISP_EXPORT_FUNC_S, HI_ID_ISP)->pfnISP_ISR)
#define CALL_ISP_ISR(irq, id)\
    FUNC_ENTRY(ISP_EXPORT_FUNC_S, HI_ID_ISP)->pfnISP_ISR(irq, id)

#define CKFN_ISP_GetRgbirFormat()\
    (NULL != FUNC_ENTRY(ISP_EXPORT_FUNC_S, HI_ID_ISP)->pfnISP_DRV_GetRgbirFormat)
#define CALL_ISP_GetRgbirFormat(ViPipe ,pRgbirFormat)                                \
    FUNC_ENTRY(ISP_EXPORT_FUNC_S, HI_ID_ISP)->pfnISP_DRV_GetRgbirFormat(ViPipe,pRgbirFormat)


#define HI_ISP_SYNC_TASK_ID_MAX_LENGTH 64
extern HI_S32 hi_isp_sync_task_register(VI_PIPE ViPipe, ISP_SYNC_TASK_NODE_S *pstNewNode);
extern HI_S32 hi_isp_sync_task_unregister(VI_PIPE ViPipe, ISP_SYNC_TASK_NODE_S *pstDelNode);

HI_S32 ISP_SetSmartInfo(VI_PIPE ViPipe, ISP_SMART_INFO_S* pstSmartInfo);
HI_S32 ISP_GetSmartInfo(VI_PIPE ViPipe, ISP_SMART_INFO_S* pstSmartInfo);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif
