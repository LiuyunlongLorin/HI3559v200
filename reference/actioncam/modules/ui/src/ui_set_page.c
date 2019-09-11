/**
 * @file      ui_set_page.c
 * @brief     ui set page. show all settings.
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/2/24
 * @version   1.0

 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "ui_common.h"
#include "hi_gv_listbox.h"
#include "hi_powercontrol.h"
#include "hi_product_netctrl.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /*  __cplusplus  */


static HI_RESID s_resHaveSubMenu;
static HI_RESID s_resNoSubMenu;
static HIGV_HANDLE s_HaveSubMenuHdl;
static HIGV_HANDLE s_NoSubMenuHdl;
static HIGV_HANDLE s_DataBufferHdl = INVALID_HANDLE;

static HI_U32 s_u32CurLevel = 0;
static PDT_UI_SET_LEVEL_S s_astLevels[MAX_SET_LEVELS] = {{PDT_UI_ACTION_SHOW_FIRST_VIEW, 0, 0} };
extern HIGV_HANDLE s_u32WinCallDialog;

#ifdef CONFIG_RAWCAP_ON
static HI_S32 s_s32DebugOn = 0;
#endif

static const PDT_UI_SET_ACTION_AND_PARAM_TYPE_S s_astActionAndParamType[] =
{
    {PDT_UI_ACTION_SET_MEDIAMODE,        HI_PDT_PARAM_TYPE_MEDIAMODE },
    {PDT_UI_ACTION_SET_PHOTO_SCENE,      HI_PDT_PARAM_TYPE_PHOTO_SCENE },
    {PDT_UI_ACTION_SET_PHOTO_OUTPUT_FMT ,HI_PDT_PARAM_TYPE_PHOTO_OUTPUT_FMT },
    {PDT_UI_ACTION_SET_DELAY_TIME,       HI_PDT_PARAM_TYPE_DELAY_TIME },
    {PDT_UI_ACTION_SET_LAPSE_INTERVAL,   HI_PDT_PARAM_TYPE_LAPSE_INTERVAL },
    {PDT_UI_ACTION_SET_BURST_TYPE,       HI_PDT_PARAM_TYPE_BURST_TYPE },
    {PDT_UI_ACTION_SET_LOOP_TIME,        HI_PDT_PARAM_TYPE_LOOP_TIME },
    {PDT_UI_ACTION_SET_ENC_PAYLOAD_TYPE, HI_PDT_PARAM_TYPE_ENC_PAYLOAD_TYPE },
    {PDT_UI_ACTION_SET_PROTUNE_EXP_EV,   HI_PDT_PARAM_TYPE_PROTUNE_EXP_EV },
    {PDT_UI_ACTION_SET_PROTUNE_EXP_TIME, HI_PDT_PARAM_TYPE_PROTUNE_EXP_TIME },
    {PDT_UI_ACTION_SET_PROTUNE_ISO,      HI_PDT_PARAM_TYPE_PROTUNE_ISO },
    {PDT_UI_ACTION_SET_PROTUNE_WB,       HI_PDT_PARAM_TYPE_PROTUNE_WB },
    {PDT_UI_ACTION_SET_PROTUNE_METRY,    HI_PDT_PARAM_TYPE_PROTUNE_METRY },

    {PDT_UI_ACTION_SET_OSD,    HI_PDT_PARAM_TYPE_OSD },
    {PDT_UI_ACTION_SET_AUDIO,  HI_PDT_PARAM_TYPE_AUDIO },
    {PDT_UI_ACTION_SET_DIS,    HI_PDT_PARAM_TYPE_DIS },
    {PDT_UI_ACTION_SET_LDC,    HI_PDT_PARAM_TYPE_LDC },
    {PDT_UI_ACTION_SET_KEYTONE,HI_PDT_PARAM_TYPE_KEYTONE },

    {PDT_UI_ACTION_SET_POWERON_ACTION,     HI_PDT_PARAM_TYPE_POWERON_ACTION },
    {PDT_UI_ACTION_SET_SCREEN_BRIGHTNESS,  HI_PDT_PARAM_TYPE_SCREEN_BRIGHTNESS },
    {PDT_UI_ACTION_SET_SCREEN_DORMANT,     HI_PDT_PARAM_TYPE_SCREEN_DORMANT_MODE },
    {PDT_UI_ACTION_SET_SYS_DORMANT,        HI_PDT_PARAM_TYPE_SYS_DORMANT_MODE },
    {PDT_UI_ACTION_SET_USB_MODE,           HI_PDT_PARAM_TYPE_USB_MODE},
    {PDT_UI_ACTION_SET_LANGUAGE,           HI_PDT_PARAM_TYPE_LANGUAGE},
};

/* constant data in first level view */
static const PDT_UI_SET_DATA_S s_astFirstLevelItems[] = {
    {PDT_UI_ACTION_SHOW_MODE_SETTINGS,  HI_PDT_PARAM_TYPE_BUTT,                STRING_WORK_MODE,         HI_TRUE},
    {PDT_UI_ACTION_SHOW_MEDIA_SETTINGS, HI_PDT_PARAM_TYPE_BUTT,                STRING_MEDIA,             HI_TRUE},
    {PDT_UI_ACTION_SHOW_POWERON_ACTION, HI_PDT_PARAM_TYPE_POWERON_ACTION,      STRING_POWERON_ACTIONS,   HI_TRUE},
    {PDT_UI_ACTION_SHOW_SYS_DORMANT,    HI_PDT_PARAM_TYPE_SYS_DORMANT_MODE,    STRING_SYSTEM_DORMANT,    HI_TRUE},
    {PDT_UI_ACTION_SHOW_SCREEN_DORMANT, HI_PDT_PARAM_TYPE_SCREEN_DORMANT_MODE, STRING_SCREEN_DORMANT,    HI_TRUE},
    {PDT_UI_ACTION_SHOW_USB_MODE,       HI_PDT_PARAM_TYPE_USB_MODE,            STRING_USB_MODE,          HI_TRUE},
    {PDT_UI_ACTION_SHOW_DATE_PAGE,      HI_PDT_PARAM_TYPE_BUTT,                STRING_SET_DATE,          HI_TRUE},
    {PDT_UI_ACTION_SHOW_TIME_PAGE,      HI_PDT_PARAM_TYPE_BUTT,                STRING_SET_TIME,          HI_TRUE},
    {PDT_UI_ACTION_SHOW_FORMAT,         HI_PDT_PARAM_TYPE_BUTT,                STRING_FORMAT_SD,         HI_TRUE},
    {PDT_UI_ACTION_SHOW_RESET,          HI_PDT_PARAM_TYPE_BUTT,                STRING_RESET_SYSTEM,      HI_TRUE},
    {PDT_UI_ACTION_SHOW_LANGUAGE,       HI_PDT_PARAM_TYPE_LANGUAGE,            STRING_LANGUAGE,          HI_TRUE},
    {PDT_UI_ACTION_SET_DEBUG,           HI_PDT_PARAM_TYPE_BUTT,                STRING_DEBUG,             HI_FALSE},
    {PDT_UI_ACTION_SHOW_ABOUT,          HI_PDT_PARAM_TYPE_BUTT,                STRING_ABOUT,             HI_TRUE},
};

static const PDT_UI_SET_DATA_S s_astCommonVideoItems[] = {
    {PDT_UI_ACTION_SHOW_MEDIAMODE,        HI_PDT_PARAM_TYPE_MEDIAMODE,        STRING_RESOLUTION,   HI_TRUE},
    {PDT_UI_ACTION_SET_AUDIO,             HI_PDT_PARAM_TYPE_AUDIO,            STRING_AUDIO,        HI_FALSE},
    {PDT_UI_ACTION_SET_ENC_PAYLOAD_TYPE,  HI_PDT_PARAM_TYPE_ENC_PAYLOAD_TYPE, STRING_VIDEO_ENCODE, HI_FALSE},
    {PDT_UI_ACTION_SET_DIS,               HI_PDT_PARAM_TYPE_DIS,              STRING_DIS,          HI_FALSE},
    {PDT_UI_ACTION_SHOW_PROTUNE_EXP_EV,   HI_PDT_PARAM_TYPE_PROTUNE_EXP_EV,   STRING_EXPOSURE_EV,  HI_TRUE},
    {PDT_UI_ACTION_SHOW_PROTUNE_ISO,      HI_PDT_PARAM_TYPE_PROTUNE_ISO,      STRING_ISO,          HI_TRUE},
    {PDT_UI_ACTION_SHOW_PROTUNE_WB,       HI_PDT_PARAM_TYPE_PROTUNE_WB,       STRING_WB,           HI_TRUE},
    {PDT_UI_ACTION_SHOW_PROTUNE_METRY,    HI_PDT_PARAM_TYPE_PROTUNE_METRY,    STRING_METER,        HI_TRUE},
};
static const PDT_UI_SET_DATA_S s_astCommonPhotoItems[] = {
    {PDT_UI_ACTION_SHOW_MEDIAMODE,        HI_PDT_PARAM_TYPE_MEDIAMODE,        STRING_RESOLUTION,    HI_TRUE},
    {PDT_UI_ACTION_SHOW_PHOTO_OUTPUT_FMT, HI_PDT_PARAM_TYPE_PHOTO_OUTPUT_FMT, STRING_OUTPUT_FORMAT, HI_TRUE},
};

static const PDT_UI_SET_DATA_S s_astProtunePhotoItems[] = {
    {PDT_UI_ACTION_SHOW_PHOTO_SCENE,      HI_PDT_PARAM_TYPE_PHOTO_SCENE,      STRING_SCENE,        HI_TRUE},
    {PDT_UI_ACTION_SHOW_PROTUNE_EXP_EV,   HI_PDT_PARAM_TYPE_PROTUNE_EXP_EV,   STRING_EXPOSURE_EV,  HI_TRUE},
    {PDT_UI_ACTION_SHOW_PROTUNE_EXP_TIME, HI_PDT_PARAM_TYPE_PROTUNE_EXP_TIME, STRING_EXPOSURE_TIME,HI_TRUE},
    {PDT_UI_ACTION_SHOW_PROTUNE_ISO,      HI_PDT_PARAM_TYPE_PROTUNE_ISO,      STRING_ISO,          HI_TRUE},
    {PDT_UI_ACTION_SHOW_PROTUNE_WB,       HI_PDT_PARAM_TYPE_PROTUNE_WB,       STRING_WB,           HI_TRUE},
    {PDT_UI_ACTION_SHOW_PROTUNE_METRY,    HI_PDT_PARAM_TYPE_PROTUNE_METRY,    STRING_METER,        HI_TRUE},
};

/*
 * WorkMode unique setting items.
 * every workmode have at 0 or 1 unique items,  get its item by using WORKMODE as index !!!
 */
static const PDT_UI_SET_DATA_S s_astPrivateModeItems[] = {
    {PDT_UI_ACTION_BUTT,                HI_PDT_PARAM_TYPE_BUTT,           0,                     HI_FALSE},/*Record Normal*/
    {PDT_UI_ACTION_SHOW_LOOP_TIME,      HI_PDT_PARAM_TYPE_LOOP_TIME,      STRING_LOOP_TIME,      HI_TRUE}, /*Record Loop */
    {PDT_UI_ACTION_SHOW_LAPSE_INTERVAL, HI_PDT_PARAM_TYPE_LAPSE_INTERVAL, STRING_LAPSE_INTERVAL, HI_TRUE}, /*Record Lapse*/
    {PDT_UI_ACTION_BUTT,                HI_PDT_PARAM_TYPE_BUTT,           0,                     HI_FALSE},/*Record Slow*/

    {PDT_UI_ACTION_BUTT,                HI_PDT_PARAM_TYPE_BUTT,           0,                     HI_FALSE},/*Photo Single */
    {PDT_UI_ACTION_SHOW_DELAY_TIME,     HI_PDT_PARAM_TYPE_DELAY_TIME,     STRING_DELAY_TIME,     HI_TRUE}, /*Photo Delay*/
    {PDT_UI_ACTION_SHOW_LAPSE_INTERVAL, HI_PDT_PARAM_TYPE_LAPSE_INTERVAL, STRING_LAPSE_INTERVAL, HI_TRUE}, /*Multi Lapse*/
    {PDT_UI_ACTION_SHOW_BURST_TYPE,     HI_PDT_PARAM_TYPE_BURST_TYPE,     STRING_BUTST_TYPE,     HI_TRUE}, /*Multi Burst*/

    {PDT_UI_ACTION_SHOW_LAPSE_INTERVAL, HI_PDT_PARAM_TYPE_LAPSE_INTERVAL, STRING_LAPSE_INTERVAL, HI_TRUE}, /*Record &Snap*/
};

static const PDT_UI_SET_DATA_S s_astMediaItems[] = {
    {PDT_UI_ACTION_SET_OSD,     HI_PDT_PARAM_TYPE_OSD,    STRING_OSD,     HI_FALSE},
    {PDT_UI_ACTION_SET_LDC,     HI_PDT_PARAM_TYPE_LDC,    STRING_LDC,     HI_FALSE},
    {PDT_UI_ACTION_SHOW_VOLUME, HI_PDT_PARAM_TYPE_VOLUME, STRING_VOLUME,  HI_TRUE},
    {PDT_UI_ACTION_SET_KEYTONE, HI_PDT_PARAM_TYPE_KEYTONE,STRING_KEYTONE, HI_FALSE},
};


static HI_BOOL UI_IsSupportDIS(HI_PDT_WORKMODE_E enWorkMode)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i = 0, j = 0, k = 0;

    HI_PDT_MEDIAMODE_E enMediaMode = HI_PDT_MEDIAMODE_BUTT;
    HI_PDT_PARAM_MEDIA_CAPABILITY_S stCapability;
    HI_PDT_MEDIA_CFG_S stMediaCfg;
    HI_PDT_SCENE_MODE_S stSceneMode;

    s32Ret = HI_PDT_PARAM_GetWorkModeParam(enWorkMode, HI_PDT_PARAM_TYPE_MEDIAMODE, (HI_VOID *)&enMediaMode);

    memset(&stCapability, 0, sizeof(HI_PDT_PARAM_MEDIA_CAPABILITY_S));
    s32Ret |= HI_PDT_PARAM_GetMediaCapability(enMediaMode, &stCapability);

    s32Ret |= HI_PDT_PARAM_GetMediaCfg(enWorkMode, enMediaMode, &stMediaCfg, &stSceneMode);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FALSE);

    for(i = 0; i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; i++)
    {
        if(HI_TRUE != stMediaCfg.stVideoCfg.stVcapCfg.astVcapDevAttr[i].bEnable)
        {
            continue;
        }

        for(j = 0; j < HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT; j++)
        {
            if(HI_TRUE != stMediaCfg.stVideoCfg.stVcapCfg.astVcapDevAttr[i].astVcapPipeAttr[j].bEnable)
            {
                continue;
            }

            for(k = 0; k < HI_PDT_MEDIA_VCAPPIPE_CHN_MAX_CNT; k++)
            {
                if(HI_TRUE != stMediaCfg.stVideoCfg.stVcapCfg.astVcapDevAttr[i].astVcapPipeAttr[j].astPipeChnAttr[k].bEnable)
                {
                    continue;
                }

                if(HI_FALSE == stCapability.astVcapDev[i].astVcapPipe[j].astPipeChn[k].bSupportDIS)
                {
                    return HI_FALSE;
                }
            }
        }
    }
    return HI_TRUE;
}

static HI_BOOL UI_IsSupportLDC(HI_VOID)
{
    HI_S32 s32Ret;
    HI_S32 i = 0;
    HI_S32 j = 0;
    HI_S32 k = 0;

    HI_PDT_MEDIAMODE_E enMediaMode = HI_PDT_MEDIAMODE_BUTT;
    HI_PDT_PARAM_MEDIA_CAPABILITY_S stCapability = {};
    HI_PDT_MEDIA_CFG_S stMediaCfg = {};
    HI_PDT_SCENE_MODE_S stSceneMode = {};
    HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkModeState = {};

    s32Ret = HI_PDT_STATEMNG_GetState(&stWorkModeState);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FALSE);

    s32Ret = HI_PDT_PARAM_GetWorkModeParam(stWorkModeState.enWorkMode, HI_PDT_PARAM_TYPE_MEDIAMODE, (HI_VOID *)&enMediaMode);

    memset(&stCapability, 0, sizeof(HI_PDT_PARAM_MEDIA_CAPABILITY_S));
    s32Ret |= HI_PDT_PARAM_GetMediaCapability(enMediaMode, &stCapability);

    s32Ret |= HI_PDT_PARAM_GetMediaCfg(stWorkModeState.enWorkMode, enMediaMode, &stMediaCfg, &stSceneMode);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FALSE);

    for(i = 0; i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; i++)
    {
        if(HI_TRUE != stMediaCfg.stVideoCfg.stVcapCfg.astVcapDevAttr[i].bEnable)
        {
            continue;
        }

        for(j = 0; j < HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT; j++)
        {
            if(HI_TRUE != stMediaCfg.stVideoCfg.stVcapCfg.astVcapDevAttr[i].astVcapPipeAttr[j].bEnable)
            {
                continue;
            }

            for(k = 0; k < HI_PDT_MEDIA_VCAPPIPE_CHN_MAX_CNT; k++)
            {
                if(HI_TRUE != stMediaCfg.stVideoCfg.stVcapCfg.astVcapDevAttr[i].astVcapPipeAttr[j].astPipeChnAttr[k].bEnable)
                {
                    continue;
                }

                if(HI_FALSE == stCapability.astVcapDev[i].astVcapPipe[j].astPipeChn[k].bSupportLDC)
                {
                    return HI_FALSE;
                }
            }
        }
    }
    return HI_TRUE;
}

static HI_S32 UI_GetParamTypeByAction(PDT_UI_ACTION_E enAction, HI_PDT_PARAM_TYPE_E * pType)
{
    HI_U32 i = 0;
    for(i=0; i<ARRAY_SIZE(s_astActionAndParamType); i++)
    {
        if(enAction == s_astActionAndParamType[i].enAction)
        {
            *pType = s_astActionAndParamType[i].enParamType;
            return HI_SUCCESS;
        }
    }
    MLOGE("action %u have no param type\n", enAction);
    return HI_FAILURE;
}
static HI_VOID UI_PushLevelInfo(PDT_UI_ACTION_E enAction, HI_U32 u32StartRow, HI_U32 u32ActiveRow)
{
    if(s_u32CurLevel >= 2)
    {
        MLOGE("CurLevel=%u, can not forward\n", s_u32CurLevel);
        return;
    }
    s_u32CurLevel++;
    s_astLevels[s_u32CurLevel].enAction = enAction;
    s_astLevels[s_u32CurLevel].u32StartRowIndex = u32StartRow;
    s_astLevels[s_u32CurLevel].u32ActiveRowIndex= u32ActiveRow;

}

static HI_VOID UI_UpdateCurLevelInfo(HI_U32 u32StartRow, HI_U32 u32ActiveRow)
{
    if(s_u32CurLevel > 2)
    {
        MLOGE("WARNING: CurLevel=%u, set to 2\n", s_u32CurLevel);
        s_u32CurLevel = 2;
    }

    s_astLevels[s_u32CurLevel].u32StartRowIndex = u32StartRow;
    s_astLevels[s_u32CurLevel].u32ActiveRowIndex= u32ActiveRow;
}

static HI_VOID UI_ParseSetData2SetRow(const PDT_UI_SET_DATA_S * pstSetData, PDT_UI_SET_ROW_S* pstSetRow)
{
    HI_S32 s32Ret = 0;

    pstSetRow->u32Action   = pstSetData->u32Action;
    pstSetRow->u32ParamType = pstSetData->u32ParamType;
    pstSetRow->resIcon = pstSetData->bHaveSubMenu ? s_HaveSubMenuHdl: s_NoSubMenuHdl;
    s32Ret = PDT_UI_COMM_GetStringByID(pstSetData->u32Title, pstSetRow->szTitle, sizeof(pstSetRow->szTitle));
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("GetStringByID failed. %#x\n", s32Ret);
        snprintf(pstSetRow->szTitle, sizeof(pstSetRow->szTitle), "%s", "????");
    }
}


static HI_VOID UI_LoadFirstLevelList()
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32Index = 0;
    HIGV_DBROW_S stDbRow = {};
    PDT_UI_SET_ROW_S stSetRow = {};
    HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkModeState = {};
    PDT_UI_DATETIME_S stUIDateTime = {};

    s32Ret = HI_PDT_STATEMNG_GetState(&stWorkModeState);
    PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret,"GetState");

    s32Ret = PDT_UI_COMM_GetDateTime(&stUIDateTime);
    PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret,"GetDateTime");


    for(u32Index = 0; u32Index < ARRAY_SIZE(s_astFirstLevelItems); u32Index++)
    {
        memset(&stDbRow, 0, sizeof(stDbRow));
        memset(&stSetRow, 0, sizeof(stSetRow));

        UI_ParseSetData2SetRow(&s_astFirstLevelItems[u32Index], &stSetRow);

        switch(stSetRow.u32Action)
        {
            case PDT_UI_ACTION_SHOW_MODE_SETTINGS:
                s32Ret = PDT_UI_COMM_GetStringOfMode(stWorkModeState.enWorkMode,
                        stSetRow.szContent, sizeof(stSetRow.szContent));
                break;

            case PDT_UI_ACTION_SHOW_POWERON_ACTION:
            case PDT_UI_ACTION_SHOW_SYS_DORMANT:
            case PDT_UI_ACTION_SHOW_SCREEN_DORMANT:
            case PDT_UI_ACTION_SHOW_USB_MODE:
            case PDT_UI_ACTION_SHOW_LANGUAGE:
                if (stSetRow.u32Action == PDT_UI_ACTION_SHOW_USB_MODE
                    && HI_TRUE == HI_PDT_NETCTRL_CheckClientConnect(HI_NULL))
                {
                    continue;
                }

                s32Ret = PDT_UI_COMM_GetParamValueDesc(HI_PDT_WORKMODE_BUTT, stSetRow.u32ParamType,
                        stSetRow.szContent, sizeof(stSetRow.szContent));
                break;

            case PDT_UI_ACTION_SHOW_DATE_PAGE:
                s32Ret = PDT_UI_COMM_GetDateString(&stUIDateTime, stSetRow.szContent, sizeof(stSetRow.szContent));
                break;

            case PDT_UI_ACTION_SHOW_TIME_PAGE:
                s32Ret = PDT_UI_COMM_GetTimeString(&stUIDateTime, stSetRow.szContent, sizeof(stSetRow.szContent));
                break;

            case PDT_UI_ACTION_SET_DEBUG:
#ifdef CONFIG_RAWCAP_ON
                if(HI_PDT_WORKMODE_NORM_REC == stWorkModeState.enWorkMode
                    || HI_PDT_WORKMODE_SING_PHOTO == stWorkModeState.enWorkMode) /** support debug */
                {
                    s32Ret = PDT_UI_COMM_GetStringByID(0 == s_s32DebugOn ? STRING_OFF : STRING_ON,
                        stSetRow.szContent, sizeof(stSetRow.szContent));
                }
                else /** unsupport debug, hide */
                {
                    continue;
                }
#else
                continue;
#endif
                break;

            default:
                break;
        }
        PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret, "LoadFirstLevelList");

        stDbRow.pData = &stSetRow;
        stDbRow.Size = sizeof(PDT_UI_SET_ROW_S);

        s32Ret = HI_GV_DDB_Append(s_DataBufferHdl, &stDbRow);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("DDB_Append failed!\n");
        }
    }

}

static HI_VOID UI_LoadVideoModeSettings(HI_U32 u32WorkMode)
{
    HI_S32 s32Ret = 0;
    HI_U32 u32Index = 0;
    HI_S32 s32Value = 0;
    HIGV_DBROW_S stDbRow = {};
    PDT_UI_SET_ROW_S stSetRow = {};

    /*work mode private item,  one mode have at most 1 item*/
    if( u32WorkMode < ARRAY_SIZE(s_astPrivateModeItems)
        && s_astPrivateModeItems[u32WorkMode].u32Action != PDT_UI_ACTION_BUTT)
    {
        memset(&stDbRow, 0, sizeof(stDbRow));
        memset(&stSetRow, 0, sizeof(stSetRow));

        /* static data */
        UI_ParseSetData2SetRow(&s_astPrivateModeItems[u32WorkMode], &stSetRow);

        /* dynamic data */
        stSetRow.u32WorkMode = u32WorkMode;
        s32Ret = PDT_UI_COMM_GetParamValueDesc(u32WorkMode, stSetRow.u32ParamType,
                stSetRow.szContent, sizeof(stSetRow.szContent));
        if(s32Ret)
        {
            MLOGE("GetCurParamString %x\n", s32Ret);
        }
        stDbRow.pData = &stSetRow;
        stDbRow.Size = sizeof(PDT_UI_SET_ROW_S);

        s32Ret = HI_GV_DDB_Append(s_DataBufferHdl, &stDbRow);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("DDB_Append failed!\n");
        }
    }

    /* work mode common items */
    for(u32Index = 0; u32Index < ARRAY_SIZE(s_astCommonVideoItems); u32Index++)
    {
        memset(&stDbRow, 0, sizeof(stDbRow));
        memset(&stSetRow, 0, sizeof(stSetRow));

        /* static data */
        UI_ParseSetData2SetRow(&s_astCommonVideoItems[u32Index], &stSetRow);

        /* dynamic data */
        stSetRow.u32WorkMode = u32WorkMode;
        s32Ret = PDT_UI_COMM_GetParamValueDesc(u32WorkMode, stSetRow.u32ParamType, stSetRow.szContent, sizeof(stSetRow.szContent));
        if(s32Ret)
        {
            MLOGE("GetCurParamString %x\n", s32Ret);
        }

        /* LapseRec and SlowRec have no AUDIO config*/
        if(PDT_UI_ACTION_SET_AUDIO == stSetRow.u32Action
            && (HI_PDT_WORKMODE_SLOW_REC == u32WorkMode || HI_PDT_WORKMODE_LPSE_REC == u32WorkMode))
        {
            continue;
        }

        switch((PDT_UI_ACTION_E)stSetRow.u32Action)
        {
            case PDT_UI_ACTION_SET_DIS:
                if(HI_FALSE == UI_IsSupportDIS(u32WorkMode))
                {
                    continue;
                }
                /** no break */
            case PDT_UI_ACTION_SET_AUDIO:
                s32Ret = HI_PDT_PARAM_GetWorkModeParam(u32WorkMode, stSetRow.u32ParamType, &s32Value);
                if(s32Ret)
                {
                    MLOGE("GetWorkModeParam failed %#x\n", s32Ret);
                }
                stSetRow.s32Value = (HI_BOOL)s32Value ? HI_FALSE : HI_TRUE;
                break;

            case PDT_UI_ACTION_SET_ENC_PAYLOAD_TYPE:
                s32Ret = HI_PDT_PARAM_GetWorkModeParam(u32WorkMode, stSetRow.u32ParamType, &s32Value);
                if(s32Ret)
                {
                    MLOGE("GetWorkModeParam failed %#x\n", s32Ret);
                }
                if( HI_MAPI_PAYLOAD_TYPE_H264 == (HI_MAPI_PAYLOAD_TYPE_E)(s32Value) )
                {
                    stSetRow.s32Value = HI_MAPI_PAYLOAD_TYPE_H265;
                }
                else
                {
                    stSetRow.s32Value = HI_MAPI_PAYLOAD_TYPE_H264;
                }
                break;

            default:
                break;
        }


        stDbRow.pData = &stSetRow;
        stDbRow.Size = sizeof(PDT_UI_SET_ROW_S);

        s32Ret = HI_GV_DDB_Append(s_DataBufferHdl, &stDbRow);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("DDB_Append failed!\n");
        }
    }
}

static HI_S32 UI_LoadPhotoModeSettings(HI_U32 u32WorkMode)
{
    HI_S32 s32Ret = 0;
    HI_U32 u32Index = 0;
    HI_PDT_PHOTO_SCENE_E enScene = HI_PDT_PHOTO_SCENE_BUTT;
    HIGV_DBROW_S stDbRow = {};
    PDT_UI_SET_ROW_S stSetRow = {};

    /*photo mode private item,  one mode have at most 1 item*/
    if( u32WorkMode < ARRAY_SIZE(s_astPrivateModeItems)
        && s_astPrivateModeItems[u32WorkMode].u32Action != PDT_UI_ACTION_BUTT)
    {
        memset(&stDbRow, 0, sizeof(stDbRow));
        memset(&stSetRow, 0, sizeof(stSetRow));

        /* static data */
        UI_ParseSetData2SetRow(&s_astPrivateModeItems[u32WorkMode], &stSetRow);

        /* dynamic data */
        stSetRow.u32WorkMode = u32WorkMode;
        s32Ret = PDT_UI_COMM_GetParamValueDesc(u32WorkMode, stSetRow.u32ParamType, stSetRow.szContent, sizeof(stSetRow.szContent));
        if(s32Ret)
        {
            MLOGE("GetCurParamString %x\n", s32Ret);
        }

        stDbRow.pData = &stSetRow;
        stDbRow.Size = sizeof(PDT_UI_SET_ROW_S);

        s32Ret = HI_GV_DDB_Append(s_DataBufferHdl, &stDbRow);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("DDB_Append failed!\n");
        }
    }

    /* photo mode common items */
    for(u32Index = 0; u32Index < ARRAY_SIZE(s_astCommonPhotoItems); u32Index++)
    {
        memset(&stDbRow, 0, sizeof(stDbRow));
        memset(&stSetRow, 0, sizeof(stSetRow));

        /* static data */
        UI_ParseSetData2SetRow(&s_astCommonPhotoItems[u32Index], &stSetRow);

        /*only SING PHOTO and DELAY PHOTO  have output format*/
        if(PDT_UI_ACTION_SHOW_PHOTO_OUTPUT_FMT == stSetRow.u32Action
            && HI_PDT_WORKMODE_DLAY_PHOTO != u32WorkMode
            && HI_PDT_WORKMODE_SING_PHOTO != u32WorkMode)
        {
            continue;
        }

        /* dynamic data */
        stSetRow.u32WorkMode = u32WorkMode;
        s32Ret = PDT_UI_COMM_GetParamValueDesc(u32WorkMode, stSetRow.u32ParamType, stSetRow.szContent, sizeof(stSetRow.szContent));
        if(s32Ret)
        {
            MLOGE("GetCurParamString %x\n", s32Ret);
        }

        stDbRow.pData = &stSetRow;
        stDbRow.Size = sizeof(PDT_UI_SET_ROW_S);

        s32Ret = HI_GV_DDB_Append(s_DataBufferHdl, &stDbRow);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("DDB_Append failed!\n");
        }
    }

    /* photo mode protune items */
    for(u32Index = 0; u32Index < ARRAY_SIZE(s_astProtunePhotoItems); u32Index++)
    {
        memset(&stDbRow, 0, sizeof(stDbRow));
        memset(&stSetRow, 0, sizeof(stSetRow));

        /* static data */
        UI_ParseSetData2SetRow(&s_astProtunePhotoItems[u32Index], &stSetRow);

        if((HI_PDT_WORKMODE_LPSE_PHOTO == u32WorkMode && HI_PDT_PARAM_TYPE_PHOTO_SCENE == stSetRow.u32ParamType)
            || (HI_PDT_WORKMODE_BURST == u32WorkMode
            && (HI_PDT_PARAM_TYPE_PHOTO_SCENE == stSetRow.u32ParamType || HI_PDT_PARAM_TYPE_PROTUNE_EXP_TIME == stSetRow.u32ParamType)))
        {
            continue;
        }

        /* dynamic data */
        stSetRow.u32WorkMode = u32WorkMode;
        s32Ret = PDT_UI_COMM_GetParamValueDesc(u32WorkMode, stSetRow.u32ParamType, stSetRow.szContent, sizeof(stSetRow.szContent));
        if(s32Ret)
        {
            MLOGE("GetCurParamString %x\n", s32Ret);
        }

        stDbRow.pData = &stSetRow;
        stDbRow.Size = sizeof(PDT_UI_SET_ROW_S);

        s32Ret = HI_GV_DDB_Append(s_DataBufferHdl, &stDbRow);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("DDB_Append failed!\n");
        }

        if(PDT_UI_ACTION_SHOW_PHOTO_SCENE == stSetRow.u32Action)
        {
            s32Ret = HI_PDT_PARAM_GetWorkModeParam(u32WorkMode, stSetRow.u32ParamType, &enScene);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

            if(HI_PDT_PHOTO_SCENE_HDR == enScene)
            {
                /*SCENE_HDR don't support protune settings*/
                break;
            }
        }
    }

    return HI_SUCCESS;
}

static HI_VOID UI_LoadWorkModeSettings(HI_VOID)
{
    HI_S32 s32Ret;
    HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkModeState = {};

    s32Ret = HI_PDT_STATEMNG_GetState(&stWorkModeState);
    PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret, "STATEMNG_GetState");

    switch(stWorkModeState.enWorkMode)
    {
        case HI_PDT_WORKMODE_NORM_REC:
        case HI_PDT_WORKMODE_LOOP_REC:
        case HI_PDT_WORKMODE_LPSE_REC:
        case HI_PDT_WORKMODE_SLOW_REC:
        case HI_PDT_WORKMODE_RECSNAP:
            UI_LoadVideoModeSettings(stWorkModeState.enWorkMode);
            break;

        case HI_PDT_WORKMODE_SING_PHOTO:
        case HI_PDT_WORKMODE_DLAY_PHOTO:
        case HI_PDT_WORKMODE_LPSE_PHOTO:
        case HI_PDT_WORKMODE_BURST:
            UI_LoadPhotoModeSettings(stWorkModeState.enWorkMode);
            break;

        default:
            MLOGE("WorkMode:%x\n", stWorkModeState.enWorkMode);
            break;
    }
}

/* OSD,FLIP, LDC,VOLUME */
static HI_VOID UI_LoadMediaSettings(HI_VOID)
{
    MLOGI("\n");
    HI_S32 s32Ret = 0;
    HI_U32 u32Index = 0;
    HI_BOOL bValue = HI_FALSE;
    HIGV_DBROW_S stDbRow = {};
    PDT_UI_SET_ROW_S stSetRow = {};

    for(u32Index = 0; u32Index < ARRAY_SIZE(s_astMediaItems); u32Index++)
    {
        memset(&stDbRow, 0, sizeof(stDbRow));
        memset(&stSetRow, 0, sizeof(stSetRow));

        UI_ParseSetData2SetRow(&s_astMediaItems[u32Index], &stSetRow);

        s32Ret = PDT_UI_COMM_GetParamValueDesc(HI_PDT_WORKMODE_BUTT, stSetRow.u32ParamType, stSetRow.szContent, sizeof(stSetRow.szContent));
        if(s32Ret)
        {
            MLOGE("GetCurParamString %x\n", s32Ret);
        }

        switch((PDT_UI_ACTION_E)stSetRow.u32Action)
        {
            case PDT_UI_ACTION_SET_LDC:
            if(HI_FALSE == UI_IsSupportLDC())
            {
                continue;
            }
            // no break;
            case PDT_UI_ACTION_SET_OSD:
                s32Ret = HI_PDT_PARAM_GetWorkModeParam(HI_PDT_WORKMODE_BUTT, stSetRow.u32ParamType, &bValue);
                if(s32Ret)
                {
                    MLOGE("GetWorkModeParam failed %#x\n", s32Ret);
                }
                stSetRow.s32Value = !bValue;
                break;

            case PDT_UI_ACTION_SET_KEYTONE:
                s32Ret = HI_PDT_PARAM_GetCommParam(stSetRow.u32ParamType, &bValue);
                if(s32Ret)
                {
                    MLOGE("GetCommParam failed %#x\n", s32Ret);
                }
                stSetRow.s32Value = !bValue;
                break;

            default:
                break;
        }

        stDbRow.pData = &stSetRow;
        stDbRow.Size = sizeof(PDT_UI_SET_ROW_S);

        s32Ret = HI_GV_DDB_Append(s_DataBufferHdl, &stDbRow);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("DDB_Append failed!\n");
        }
    }

}

/*
 * get current value and value set
 * add to DDB
 */
static HI_S32 UI_LoadSettingValueSet(PDT_UI_ACTION_E enAction)
{
    HI_S32 s32Ret;
    HI_S32 s32Value;
    HI_U32 u32Index;
    HI_PDT_WORKMODE_E enWorkMode = HI_PDT_WORKMODE_NORM_REC;
    HI_PDT_PARAM_TYPE_E enParamType = HI_PDT_PARAM_TYPE_BUTT;
    HI_PDT_ITEM_VALUESET_S stValueSet = {};
    HI_TIMEDTASK_ATTR_S stTimedTaskAttr = {};
    HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkModeState = {};

    HIGV_DBROW_S stDbRow = {};
    PDT_UI_SET_ROW_S stSetRow = {};

    /*Get current work mode*/
    s32Ret = HI_PDT_STATEMNG_GetState(&stWorkModeState);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    enWorkMode = stWorkModeState.enWorkMode;

    s32Ret = UI_GetParamTypeByAction(enAction, &enParamType);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    switch(enParamType)
    {
        case HI_PDT_PARAM_TYPE_SYS_DORMANT_MODE:
        case HI_PDT_PARAM_TYPE_SCREEN_DORMANT_MODE:
            s32Ret = HI_PDT_PARAM_GetCommParam(enParamType, &stTimedTaskAttr);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            s32Value = (HI_TRUE == stTimedTaskAttr.bEnable ? stTimedTaskAttr.u32Time_sec : 0);

            s32Ret = HI_PDT_PARAM_GetCommItemValues(enParamType, &stValueSet);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            break;

        case HI_PDT_PARAM_TYPE_POWERON_ACTION:
        case HI_PDT_PARAM_TYPE_USB_MODE:
        case HI_PDT_PARAM_TYPE_LANGUAGE:
            s32Ret = HI_PDT_PARAM_GetCommParam(enParamType, &s32Value);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

            s32Ret = HI_PDT_PARAM_GetCommItemValues(enParamType, &stValueSet);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            break;

        case HI_PDT_PARAM_TYPE_DELAY_TIME:
        case HI_PDT_PARAM_TYPE_BURST_TYPE:
        case HI_PDT_PARAM_TYPE_LAPSE_INTERVAL:
        case HI_PDT_PARAM_TYPE_LOOP_TIME:
        case HI_PDT_PARAM_TYPE_MEDIAMODE:
        case HI_PDT_PARAM_TYPE_PHOTO_OUTPUT_FMT:
        case HI_PDT_PARAM_TYPE_PHOTO_SCENE:
        case HI_PDT_PARAM_TYPE_PROTUNE_EXP_EV:
        case HI_PDT_PARAM_TYPE_PROTUNE_EXP_TIME:
        case HI_PDT_PARAM_TYPE_PROTUNE_ISO:
        case HI_PDT_PARAM_TYPE_PROTUNE_WB:
        case HI_PDT_PARAM_TYPE_PROTUNE_METRY:
            s32Ret = HI_PDT_PARAM_GetWorkModeParam(enWorkMode, enParamType, &s32Value);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

            s32Ret = HI_PDT_PARAM_GetWorkModeItemValues(enWorkMode, enParamType, &stValueSet);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            break;

        default:
            MLOGE("param type %u not processed\n", enParamType);
            return HI_FAILURE;
    }

    for(u32Index = 0; u32Index < stValueSet.s32Cnt && u32Index < HI_PDT_ITEM_VALUESET_MAX_NUM; ++u32Index)
    {
#ifndef CFG_POST_PROCESS
        if(HI_PDT_PARAM_TYPE_PHOTO_SCENE == enParamType
            && HI_PDT_PHOTO_SCENE_HDR == stValueSet.astValues[u32Index].s32Value)
        {
            continue;
        }
#endif

        if(HI_PDT_WORKMODE_LPSE_PHOTO == enWorkMode && enParamType == HI_PDT_PARAM_TYPE_PROTUNE_EXP_TIME)
        {
            HI_S32 s32LapseInterval;
            s32Ret = HI_PDT_PARAM_GetWorkModeParam(HI_PDT_WORKMODE_LPSE_PHOTO, HI_PDT_PARAM_TYPE_LAPSE_INTERVAL, &s32LapseInterval);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

            if(stValueSet.astValues[u32Index].s32Value / 1000 >= s32LapseInterval)
            {
                continue;
            }
        }

        memset(&stDbRow,  0, sizeof(stDbRow));
        memset(&stSetRow, 0, sizeof(stSetRow));

        s32Ret = PDT_UI_COMM_Translate(stValueSet.astValues[u32Index].szDesc, stSetRow.szTitle, sizeof(stSetRow.szTitle));
        if(HI_SUCCESS != s32Ret)
        {
            snprintf(stSetRow.szTitle, sizeof(stSetRow.szTitle), "%s", stValueSet.astValues[u32Index].szDesc);
        }

        if(s32Value == stValueSet.astValues[u32Index].s32Value)
        {
            snprintf(stSetRow.szContent, sizeof(stSetRow.szContent), "*");
        }

        stSetRow.resIcon = INVALID_HANDLE;
        stSetRow.u32Action = enAction;
        stSetRow.u32ParamType = enParamType;
        stSetRow.u32WorkMode = enWorkMode;
        stSetRow.s32Value = stValueSet.astValues[u32Index].s32Value;

        stDbRow.pData = &stSetRow;
        stDbRow.Size = sizeof(PDT_UI_SET_ROW_S);
        MLOGD("Load Values [action:%d, mode:%d, type:%d, value:%d]\n", enAction, enWorkMode, enParamType, stSetRow.s32Value);

        s32Ret = HI_GV_DDB_Append(s_DataBufferHdl, &stDbRow);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("DDB_Append failed!\n");
        }
    }

    return  HI_SUCCESS;
}

static HI_VOID UI_SET_ShowList(HI_BOOL bSetIndex)
{
    HI_S32 s32Ret;

    PDT_UI_ACTION_E enAction = s_astLevels[s_u32CurLevel].enAction;
    HI_U32 u32StartIndex = s_astLevels[s_u32CurLevel].u32StartRowIndex;
    HI_U32 u32SelectIndex = s_astLevels[s_u32CurLevel].u32ActiveRowIndex;
    MLOGD("u32StartIndex = %u, u32SelectIndex = %u\n", u32StartIndex, u32SelectIndex);

    HI_GV_DDB_EnableDataChange(s_DataBufferHdl, HI_FALSE);
    HI_GV_DDB_Clear(s_DataBufferHdl);

    switch(enAction)
    {
        case PDT_UI_ACTION_SHOW_FIRST_VIEW:
            UI_LoadFirstLevelList();
            break;
        case PDT_UI_ACTION_SHOW_MODE_SETTINGS:
            UI_LoadWorkModeSettings();
            break;
        case PDT_UI_ACTION_SHOW_MEDIA_SETTINGS:
            UI_LoadMediaSettings();
            break;

        case PDT_UI_ACTION_SHOW_POWERON_ACTION:
        case PDT_UI_ACTION_SHOW_SYS_DORMANT:
        case PDT_UI_ACTION_SHOW_SCREEN_DORMANT:
        case PDT_UI_ACTION_SHOW_USB_MODE:
        case PDT_UI_ACTION_SHOW_LANGUAGE:

        case PDT_UI_ACTION_SHOW_DELAY_TIME:
        case PDT_UI_ACTION_SHOW_BURST_TYPE:
        case PDT_UI_ACTION_SHOW_LAPSE_INTERVAL:
        case PDT_UI_ACTION_SHOW_LOOP_TIME:
        case PDT_UI_ACTION_SHOW_MEDIAMODE:
        case PDT_UI_ACTION_SHOW_PHOTO_OUTPUT_FMT:
        case PDT_UI_ACTION_SHOW_PHOTO_SCENE:
        case PDT_UI_ACTION_SHOW_PROTUNE_EXP_EV:
        case PDT_UI_ACTION_SHOW_PROTUNE_EXP_TIME:
        case PDT_UI_ACTION_SHOW_PROTUNE_ISO:
        case PDT_UI_ACTION_SHOW_PROTUNE_WB:
        case PDT_UI_ACTION_SHOW_PROTUNE_METRY:
            UI_LoadSettingValueSet((PDT_UI_ACTION_E)(enAction + PDT_UI_SET_AND_SHOW_OFFSET));  /* PDT_UI_ACTION_SHOW_XXX --> PDT_UI_ACTION_SET_XXX */
            break;

        default:
            MLOGE("Action %d not processed!\n", enAction);
            break;
    }

    HI_GV_DDB_EnableDataChange(s_DataBufferHdl, HI_TRUE);

    s32Ret = HI_GV_Widget_SyncDB(SET_LISTBOX);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("SyncDB: %#x\n", s32Ret);
    }

    if(HI_TRUE == bSetIndex)
    {
        s32Ret = HI_GV_List_SetStartItem(SET_LISTBOX, u32StartIndex);
        if(HI_SUCCESS != s32Ret)
        {
            MLOGE("SetStartItem: %#x\n", s32Ret);
        }

        s32Ret = HI_GV_List_SetSelItem(SET_LISTBOX, u32SelectIndex);
        if(HI_SUCCESS != s32Ret)
        {
            MLOGE("SetSelItem: %#x\n", s32Ret);
        }
    }

    s32Ret = HI_GV_Widget_Paint(SET_LISTBOX, NULL);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("Widget_Paint: %#x\n", s32Ret);
    }
}

static HI_VOID UI_SET_Back(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    if( 0 == s_u32CurLevel  ||  s_u32CurLevel >= MAX_SET_LEVELS)
    {
        s32Ret = PDT_UI_WINMNG_FinishWindow(SET_PAGE);
        if(HI_SUCCESS != s32Ret)
        {
            MLOGE("PDT_UI_WINMNG_FinishWindow: %#x\n", s32Ret);
        }
        UI_UpdateCurLevelInfo(0, 0);
    }
    else
    {
        s_u32CurLevel--;
        s32Ret = HI_GV_Widget_Active(SET_LISTBOX);
        if(HI_SUCCESS != s32Ret)
        {
            MLOGE("Widget_Active: %#x\n", s32Ret);
        }
        UI_SET_ShowList(HI_TRUE);
    }
}

HI_S32 PDT_UI_SET_OnShow(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGI("\n");
    HI_S32 s32Ret = 0;

    s32Ret = HI_GV_List_SetFlingParam(SET_LISTBOX, 1);
    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("SetFlingParam failed, s32Ret:%d\n",s32Ret);
    }

    s32Ret = HI_GV_ADM_GetDDBHandle(DATAMODEL_SETTINGS, &s_DataBufferHdl);
    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("GetDDBHandle failed, s32Ret:%d\n",s32Ret);
        return s32Ret;
    }

    s32Ret = HI_GV_Res_CreateID(ICON_HAVE_SUB_MENU_PATH, HIGV_RESTYPE_IMG, &s_resHaveSubMenu);
    s32Ret |= HI_GV_Res_GetResInfo(s_resHaveSubMenu, &s_HaveSubMenuHdl);
    s32Ret |= HI_GV_Res_CreateID(ICON_NO_SUB_MENU_PATH, HIGV_RESTYPE_IMG, &s_resNoSubMenu);
    s32Ret |= HI_GV_Res_GetResInfo(s_resNoSubMenu, &s_NoSubMenuHdl);
    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("CreateResHdl failed\n");
    }

    UI_SET_ShowList(HI_TRUE);

    HI_GV_Widget_Active(SET_LISTBOX);

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_SET_OnHide(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGD("\n");
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HI_GV_Res_ReleaseResInfo(s_resHaveSubMenu);
    s32Ret |= HI_GV_Res_DestroyID(s_resHaveSubMenu);
    s32Ret |= HI_GV_Res_ReleaseResInfo(s_resNoSubMenu);
    s32Ret |= HI_GV_Res_DestroyID(s_resNoSubMenu);
    if (s32Ret)
    {
        MLOGE("ReleaseResHdl failed\n");
    }

    return HIGV_PROC_GOON;
}

static HI_S32 UI_SET_OnReceiveMsgResult(HI_EVENT_S* pstEvent)
{
    PDT_UI_WINMNG_FinishWindow(BUSY_PAGE);
    UI_SET_ShowList(HI_FALSE);
    return HI_SUCCESS;
}

static HI_VOID UI_SET_OnClickRow(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32ActiveRow = 0;
    HI_U32 u32StartRow = 0;
    HI_U32 u32ItemNum = 0;
    HI_CHAR szLanId[SETTING_CONTENT_SIZE];
    PDT_UI_SET_ROW_S stSetRow = {};
    HIGV_DBROW_S stDbRow = {};
    HI_MESSAGE_S stMsg = {};
    HI_TIMEDTASK_ATTR_S stTimedTask = {};
    HI_PDT_WORKMODE_E enWorkMode = HI_PDT_WORKMODE_NORM_REC;
    HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkModeState = {};

    /*Get current work mode*/
    s32Ret = HI_PDT_STATEMNG_GetState(&stWorkModeState);

    enWorkMode = stWorkModeState.enWorkMode;

    stDbRow.Size = sizeof(PDT_UI_SET_ROW_S);
    stDbRow.pData = &stSetRow;

    s32Ret = HI_GV_List_GetStartItem(SET_LISTBOX, &u32StartRow);
    s32Ret |= HI_GV_List_GetSelItem(SET_LISTBOX, &u32ActiveRow);
    s32Ret |= HI_GV_List_GetItemNum(SET_LISTBOX, &u32ItemNum);
    if (s32Ret != HI_SUCCESS)
    {
        MLOGE("Error:%#x\n", s32Ret);
    }

    MLOGD("u32StartRow = %u, u32ActiveRow = %u\n", u32StartRow, u32ActiveRow);

    s32Ret = HI_GV_DDB_GetRow(s_DataBufferHdl, u32ActiveRow, &stDbRow);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("DDB GetRow failed. %#x\n", s32Ret);
        return;
    }

    switch(stSetRow.u32Action)
    {
        case PDT_UI_ACTION_SHOW_MODE_SETTINGS:
        case PDT_UI_ACTION_SHOW_MEDIA_SETTINGS:
        case PDT_UI_ACTION_SHOW_POWERON_ACTION:
        case PDT_UI_ACTION_SHOW_SYS_DORMANT:
        case PDT_UI_ACTION_SHOW_SCREEN_DORMANT:
        case PDT_UI_ACTION_SHOW_USB_MODE:
        case PDT_UI_ACTION_SHOW_LANGUAGE:

        case PDT_UI_ACTION_SHOW_DELAY_TIME:
        case PDT_UI_ACTION_SHOW_BURST_TYPE:
        case PDT_UI_ACTION_SHOW_LAPSE_INTERVAL:
        case PDT_UI_ACTION_SHOW_LOOP_TIME:
        case PDT_UI_ACTION_SHOW_MEDIAMODE:
        case PDT_UI_ACTION_SHOW_PHOTO_OUTPUT_FMT:
        case PDT_UI_ACTION_SHOW_PHOTO_SCENE:
        case PDT_UI_ACTION_SHOW_PROTUNE_EXP_EV:
        case PDT_UI_ACTION_SHOW_PROTUNE_EXP_TIME:
        case PDT_UI_ACTION_SHOW_PROTUNE_ISO:
        case PDT_UI_ACTION_SHOW_PROTUNE_WB:
        case PDT_UI_ACTION_SHOW_PROTUNE_METRY:
            UI_UpdateCurLevelInfo(u32StartRow, u32ActiveRow);
            UI_PushLevelInfo(stSetRow.u32Action, 0, 0);
            UI_SET_ShowList(HI_TRUE);
            break;

        case PDT_UI_ACTION_SHOW_DATE_PAGE:
            UI_UpdateCurLevelInfo(u32StartRow, u32ActiveRow);
            s32Ret = PDT_UI_WINMNG_StartWindow(DATE_SET_WINDOW, HI_TRUE);
            if(s32Ret)
            {
                MLOGE("StartWindow %x\n", s32Ret);
            }
            break;

        case PDT_UI_ACTION_SHOW_TIME_PAGE:
            UI_UpdateCurLevelInfo(u32StartRow, u32ActiveRow);
            s32Ret = PDT_UI_WINMNG_StartWindow(TIME_SET_WINDOW, HI_TRUE);
            if(s32Ret)
            {
                MLOGE("StartWindow %x\n", s32Ret);
            }
            break;

        case PDT_UI_ACTION_SHOW_FORMAT:
            s_u32WinCallDialog = STRING_FORMAT_SD;

            s32Ret = HI_GV_Widget_SetTextByID(DIALOG_WINDOW_LABEL_FORMAT_ASK, STRING_FORMAT_SD_AT_ONCE);
            if(s32Ret)
            {
                MLOGE("SetTextByID failed. %x\n", s32Ret);
            }

            s32Ret = PDT_UI_WINMNG_StartWindow(DIALOG_WINDOW, HI_FALSE);
            if(s32Ret)
            {
                MLOGE("StartWindow %x\n", s32Ret);
            }
            s32Ret = HI_GV_Widget_Show(DIALOG_WINDOW_GROUPBOX_FORMAT);
            s32Ret = HI_GV_Widget_Hide(DIALOG_WINDOW_GROUPBOX_CONTENT);
            s32Ret = HI_GV_Widget_Active(DIALOG_WINDOW_BUTTON_CANCEL);
            s32Ret = HI_GV_Widget_SetSkin(DIALOG_WINDOW_BUTTON_FORMAT_CHOOSE, HIGV_SKIN_NORMAL,
                SKIN_DIALOG_WINDOW_BUTTON_FORMAT_CHOSED_NORMAL);
            s32Ret = HI_GV_Widget_SetSkin(DIALOG_WINDOW_BUTTON_FORMAT_CHOOSE, HIGV_SKIN_ACITVE,
                SKIN_DIALOG_WINDOW_BUTTON_FORMAT_CHOSED_ACTIVE);
            break;

        case PDT_UI_ACTION_SHOW_RESET:
            s_u32WinCallDialog = STRING_RESET_SYSTEM;

            s32Ret = HI_GV_Widget_SetTextByID(DIALOG_WINDOW_LABEL_ASK, STRING_RESET_FACTORY_AT_ONCE);
            if(s32Ret)
            {
                MLOGE("SetTextByID failed. %x\n", s32Ret);
            }

            s32Ret = PDT_UI_WINMNG_StartWindow(DIALOG_WINDOW, HI_FALSE);
            if(s32Ret)
            {
                MLOGE("StartWindow %x\n", s32Ret);
            }
            s32Ret = HI_GV_Widget_Active(DIALOG_WINDOW_BUTTON_CANCEL);
            break;

        case PDT_UI_ACTION_SHOW_ABOUT:
            s32Ret = PDT_UI_WINMNG_StartWindow(ABOUT_WINDOW, HI_FALSE);
            if(s32Ret)
            {
                MLOGE("StartWindow %x\n", s32Ret);
            }
            break;

        case PDT_UI_ACTION_SHOW_VOLUME:
            UI_UpdateCurLevelInfo(u32StartRow, u32ActiveRow);
            s32Ret = PDT_UI_WINMNG_StartWindow(VOLUME_SET_WINDOW, HI_TRUE);
            if(s32Ret)
            {
                MLOGE("StartWindow %x\n", s32Ret);
            }
            break;

        /* now do set works */
        case PDT_UI_ACTION_SET_ENC_PAYLOAD_TYPE:
        case PDT_UI_ACTION_SET_PHOTO_OUTPUT_FMT:
        case PDT_UI_ACTION_SET_OSD:
        case PDT_UI_ACTION_SET_AUDIO:
        case PDT_UI_ACTION_SET_DIS:
        case PDT_UI_ACTION_SET_LDC:

        case PDT_UI_ACTION_SET_MEDIAMODE:
        case PDT_UI_ACTION_SET_PHOTO_SCENE:
        case PDT_UI_ACTION_SET_DELAY_TIME:
        case PDT_UI_ACTION_SET_LAPSE_INTERVAL:
        case PDT_UI_ACTION_SET_BURST_TYPE:
        case PDT_UI_ACTION_SET_LOOP_TIME:
        case PDT_UI_ACTION_SET_PROTUNE_EXP_EV:
        case PDT_UI_ACTION_SET_PROTUNE_EXP_TIME:
        case PDT_UI_ACTION_SET_PROTUNE_ISO:
        case PDT_UI_ACTION_SET_PROTUNE_WB:
        case PDT_UI_ACTION_SET_PROTUNE_METRY:
            UI_UpdateCurLevelInfo(u32StartRow, u32ActiveRow);
            stMsg.what = HI_EVENT_STATEMNG_SETTING;
            stMsg.arg1 = stSetRow.u32ParamType;
            stMsg.arg2 = enWorkMode;

            HI_CHAR* payload = (stMsg.aszPayload);
            *(HI_PDT_SCENE_METRY_TYPE_E *)payload = stSetRow.s32Value;


            if(PDT_UI_ACTION_SET_PROTUNE_METRY == stSetRow.u32Action
                && HI_PDT_SCENE_METRY_TYPE_SPOT == stSetRow.s32Value)
            {
                HI_PDT_SCENE_METRY_SPOT_PARAM_S stMetrySpotParam;

#if defined(CONFIG_VERTICAL_SCREEN)
                HIGV_ROTATE_E enRotate = HIGV_ROTATE_90;
#else
                HIGV_ROTATE_E enRotate = HIGV_ROTATE_NONE;
#endif

                /** rotate */
                if(HIGV_ROTATE_NONE == enRotate)
                {
                    stMetrySpotParam.stPoint.s32X = 50;
                    stMetrySpotParam.stPoint.s32Y = 50;
                    stMetrySpotParam.stSize.u32Width = ICON_SPOT_METERING_WIDTH_PERCENT;
                    stMetrySpotParam.stSize.u32Height = ICON_SPOT_METERING_HEIGHT_PERCENT;
                }
                else if(HIGV_ROTATE_90 == enRotate)
                {
                    stMetrySpotParam.stPoint.s32X = 50;
                    stMetrySpotParam.stPoint.s32Y = 50;
                    stMetrySpotParam.stSize.u32Width = ICON_SPOT_METERING_HEIGHT_PERCENT;
                    stMetrySpotParam.stSize.u32Height = ICON_SPOT_METERING_WIDTH_PERCENT;
                }

                memcpy((payload + sizeof(HI_PDT_SCENE_METRY_TYPE_E)), &stMetrySpotParam,
                    sizeof(HI_PDT_SCENE_METRY_SPOT_PARAM_S));

                MLOGD("Setting -----> [x:%d, y:%d, width:%d, height:%d]\n",
                stMetrySpotParam.stPoint.s32X,
                stMetrySpotParam.stPoint.s32Y,
                stMetrySpotParam.stSize.u32Width,
                stMetrySpotParam.stSize.u32Height);
            }

            s32Ret = PDT_UI_WINMNG_StartWindow(BUSY_PAGE, HI_FALSE);
            PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret,"StartWindow");

            MLOGD("Setting -----> [mode:%d, type:%d, value:%d]\n", stMsg.arg2, stMsg.arg1, stSetRow.s32Value);
            s32Ret = PDT_UI_COMM_SendAsyncMsg(&stMsg, UI_SET_OnReceiveMsgResult);
            if(s32Ret)
            {
                MLOGE("SendAsyncMsg failed. %x\n", s32Ret);
                s32Ret = PDT_UI_WINMNG_FinishWindow(BUSY_PAGE);
                PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret,"FinishWindow");
            }
            break;

        case PDT_UI_ACTION_SET_USB_MODE:
        {
            HI_USB_MODE_E enUsbMode;
            s32Ret = HI_USB_GetMode(&enUsbMode);
            PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "get usb mode");

            HI_USB_STATE_E enUsbState;
            s32Ret = HI_USB_GetState(&enUsbState);
            PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "get usb state");

            if ((HI_USB_MODE_CHARGE != enUsbMode) && (HI_USB_STATE_OUT != enUsbState))
            {
                MLOGW(YELLOW"current usb_mode[%d] usb_state[%d], can not set usb mode!\n"NONE,
                    enUsbMode, enUsbState);
                return ;
            }

            s32Ret = HI_USB_SetMode((HI_USB_MODE_E)stSetRow.s32Value);
            PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "SetUsbMode");
            /** no break */
        }
        case PDT_UI_ACTION_SET_POWERON_ACTION:
        case PDT_UI_ACTION_SET_KEYTONE:
            s32Ret = HI_PDT_PARAM_SetCommParam(stSetRow.u32ParamType, &(stSetRow.s32Value));
            if(s32Ret)
            {
                MLOGE("SetCommParam failed. ParamType:%d, value:%d\n", stSetRow.u32ParamType, stSetRow.s32Value);
            }
            UI_UpdateCurLevelInfo(u32StartRow, u32ActiveRow);
            UI_SET_ShowList(HI_FALSE);
            break;

        case PDT_UI_ACTION_SET_LANGUAGE:
            s32Ret = HI_PDT_PARAM_SetCommParam(stSetRow.u32ParamType, &(stSetRow.s32Value));
            if(s32Ret)
            {
                MLOGE("SetCommParam failed. ParamType:%d, value:%d\n", stSetRow.u32ParamType, stSetRow.s32Value);
            }

            s32Ret = PDT_UI_COMM_GetStringByString(stSetRow.szTitle, szLanId, SETTING_CONTENT_SIZE);
            PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "GetLanIdByString");

            s32Ret = HI_GV_Lan_Change(szLanId);
            PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "ChangeLanguage");

            UI_UpdateCurLevelInfo(u32StartRow, u32ActiveRow);
            UI_SET_ShowList(HI_FALSE);
            break;

        case PDT_UI_ACTION_SET_SYS_DORMANT:
        case PDT_UI_ACTION_SET_SCREEN_DORMANT:
            stTimedTask.bEnable = stSetRow.s32Value > 0 ? HI_TRUE : HI_FALSE;
            stTimedTask.u32Time_sec = stSetRow.s32Value;
            MLOGD("timed task value = %d\n", stSetRow.s32Value);
            s32Ret = HI_PDT_PARAM_SetCommParam(stSetRow.u32ParamType, &stTimedTask);
            if(s32Ret)
            {
                MLOGE("SetCommParam failed. ParamType:%d, value:%d\n", stSetRow.u32ParamType, stSetRow.s32Value);
            }

            HI_POWERCTRL_TASK_E enPwrCtrlType = (PDT_UI_ACTION_SET_SYS_DORMANT == stSetRow.u32Action
                ? HI_PWRCTRL_TASK_SYSTEMDORMANT : HI_PWRCTRL_TASK_SCREENDORMANT);
            s32Ret = HI_POWERCTRL_SetTaskAttr(enPwrCtrlType, &stTimedTask);
            if(s32Ret)
            {
                MLOGE("SetTaskAttr failed. PwrCtrlType:%d, value:%d\n", enPwrCtrlType, stSetRow.s32Value);
            }

            UI_UpdateCurLevelInfo(u32StartRow, u32ActiveRow);
            UI_SET_ShowList(HI_FALSE);
            break;

        case PDT_UI_ACTION_SET_SCREEN_BRIGHTNESS:

            break;

        case PDT_UI_ACTION_SET_DEBUG:
#ifdef CONFIG_RAWCAP_ON
            if(HI_PDT_WORKMODE_NORM_REC == enWorkMode || HI_PDT_WORKMODE_SING_PHOTO == enWorkMode)
            {
                UI_UpdateCurLevelInfo(u32StartRow, u32ActiveRow);

                memset(&stMsg, 0x0, sizeof(HI_MESSAGE_S));

                stMsg.what = HI_EVENT_STATEMNG_DEBUG_SWITCH;
                stMsg.arg2 = enWorkMode;

                s_s32DebugOn = (s_s32DebugOn == 0 ? 1 : 0);
                HI_CHAR* payload = (stMsg.aszPayload);
                *(HI_S32 *)payload = s_s32DebugOn;

                s32Ret = PDT_UI_WINMNG_StartWindow(BUSY_PAGE, HI_FALSE);
                PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret, "StartWindow");

                s32Ret = PDT_UI_COMM_SendAsyncMsg(&stMsg, UI_SET_OnReceiveMsgResult);
                if(HI_SUCCESS != s32Ret)
                {
                    MLOGE("SendAsyncMsg failed. %x\n", s32Ret);
                    s_s32DebugOn = (s_s32DebugOn == 0 ? 1 : 0);
                    s32Ret = PDT_UI_WINMNG_FinishWindow(BUSY_PAGE);
                    PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret, "FinishWindow");
                }
            }
#endif
            break;

        default:
            MLOGE("Action %d not processed\n", stSetRow.u32Action);
            break;
    }
}

HI_S32 PDT_UI_SET_OnKeyDown(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGD("wParam: %lx, lParam = %lx\n", (HI_UL)wParam, (HI_UL)lParam);
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32CurrentRow = 0;
    HI_U32 u32ItemNum = 0;

    if (PDT_UI_KEY_BACK == wParam)
    {
        UI_SET_Back();
    }
    else if (PDT_UI_KEY_MENU == wParam)
    {
        if (SET_BUTTON_BACK == WidgetHdl)
        {
            HI_GV_Widget_Active(SET_LISTBOX);
            HI_GV_List_SetSelItem(SET_LISTBOX, 0);
        }
        else
        {
            s32Ret = HI_GV_List_GetSelItem(SET_LISTBOX, &u32CurrentRow);
            s32Ret |= HI_GV_List_GetItemNum(SET_LISTBOX, &u32ItemNum);
            if (s32Ret != HI_SUCCESS)
            {
                MLOGE("Error:%#x\n", s32Ret);
            }

            MLOGD("ItemNum: %d, Current Row:%d\n", u32ItemNum, u32CurrentRow);

            if (0 == (u32CurrentRow+1)%u32ItemNum)
            {
                s32Ret = HI_GV_Widget_Active(SET_BUTTON_BACK);
            }
            else
            {
                s32Ret = HI_GV_List_SetSelItem(SET_LISTBOX, (u32CurrentRow+1)%u32ItemNum );
                s32Ret |=HI_GV_Widget_Paint(SET_LISTBOX, NULL);
            }

            if (s32Ret != HI_SUCCESS)
            {
                MLOGE("Error:%#x\n", s32Ret);
            }

        }
    }
    else if (PDT_UI_KEY_ENTER == wParam)
    {
        if (SET_BUTTON_BACK == WidgetHdl)
        {
            UI_SET_Back();
        }
        else
        {
            UI_SET_OnClickRow();
        }
    }
    else
    {
        MLOGE("KEY CODE %#lx not processed\n", (HI_UL)wParam);
    }

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_SET_OnGestureTap(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGD("widget:%d, pointer[%d,%d]\n", WidgetHdl, ((HIGV_GESTURE_EVENT_S*)lParam)->gesture.tap.pointer.x,
        ((HIGV_GESTURE_EVENT_S*)lParam)->gesture.tap.pointer.y);

    switch(WidgetHdl)
    {
        case SET_BUTTON_BACK:
            UI_SET_Back();
            break;
    }

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_SET_OnCellSelect(HIGV_HANDLE WidgetHdl,HI_PARAM wParam, HI_PARAM lParam)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32CurrentRow = 0;
    HI_U32 u32ItemNum = 0;

    s32Ret = HI_GV_List_GetSelItem(SET_LISTBOX, &u32CurrentRow);
    s32Ret |= HI_GV_List_GetItemNum(SET_LISTBOX, &u32ItemNum);
    MLOGD("ItemNum: %d, Current Row:%d\n", u32ItemNum, u32CurrentRow);

    s32Ret = HI_GV_Widget_Active(SET_LISTBOX);
    if(s32Ret)
    {
        MLOGE("Active %x\n", s32Ret);
    }

    UI_SET_OnClickRow();

    return HIGV_PROC_GOON;
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /*  __cplusplus  */

