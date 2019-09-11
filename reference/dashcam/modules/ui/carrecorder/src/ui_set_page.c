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
#include "hi_hal_wifi.h"
#include "hi_hal_screen.h"
#include "hi_product_media.h"
#include "hi_recordmng.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /*  __cplusplus  */

static HI_RESID s_ResIcon;
static HIGV_HANDLE s_resHaveSubMenu;
static HIGV_HANDLE s_DataBufferHdl = INVALID_HANDLE;
static HI_U32 s_u32CurLevel = 0;
static PDT_UI_SET_LEVEL_S s_astLevels[MAX_SET_LEVELS] = {{PDT_UI_ACTION_SHOW_FIRST_VIEW, 0, 0} };

#ifdef CONFIG_RAWCAP_ON
static HI_BOOL s_b32DebugOn = HI_FALSE;
#endif

static const PDT_UI_SET_ACTION_AND_PARAM_TYPE_S s_astActionAndParamType[] =
{
    {PDT_UI_ACTION_SET_MEDIAMODE,          HI_PDT_PARAM_TYPE_MEDIAMODE },
    {PDT_UI_ACTION_SET_ENC_PAYLOAD_TYPE,   HI_PDT_PARAM_TYPE_ENC_PAYLOAD_TYPE },

    {PDT_UI_ACTION_SET_OSD,                HI_PDT_PARAM_TYPE_OSD },
    {PDT_UI_ACTION_SET_AUDIO,              HI_PDT_PARAM_TYPE_AUDIO },
    {PDT_UI_ACTION_SET_FLIP,               HI_PDT_PARAM_TYPE_FLIP },
    {PDT_UI_ACTION_SET_MIRROR,             HI_PDT_PARAM_TYPE_MIRROR },
    {PDT_UI_ACTION_SET_LDC,                HI_PDT_PARAM_TYPE_LDC },
    {PDT_UI_ACTION_SET_WDR,                HI_PDT_PARAM_TYPE_WDR },
    {PDT_UI_ACTION_SET_VIDEOMODE,          HI_PDT_PARAM_TYPE_VIDEOMODE },

    {PDT_UI_ACTION_SET_DEV_INFO,           HI_PDT_PARAM_TYPE_DEV_INFO },
    {PDT_UI_ACTION_SET_VOLUME,             HI_PDT_PARAM_TYPE_VOLUME },
    {PDT_UI_ACTION_SET_KEYTONE,            HI_PDT_PARAM_TYPE_KEYTONE},
    {PDT_UI_ACTION_SET_BOOTSOUND,          HI_PDT_PARAM_TYPE_BOOTSOUND},
    {PDT_UI_ACTION_SET_SCREEN_BRIGHTNESS,  HI_PDT_PARAM_TYPE_SCREEN_BRIGHTNESS },
    {PDT_UI_ACTION_SET_SCREEN_DORMANT,     HI_PDT_PARAM_TYPE_SCREEN_DORMANT_MODE },
    {PDT_UI_ACTION_SET_USB_MODE,           HI_PDT_PARAM_TYPE_USB_MODE },
    {PDT_UI_ACTION_SET_LANGUAGE,           HI_PDT_PARAM_TYPE_LANGUAGE },
    {PDT_UI_ACTION_SET_SENSITIVITY,        HI_PDT_PARAM_TYPE_GSENSOR_SENSITIVITY},
    {PDT_UI_ACTION_SET_PARKING,            HI_PDT_PARAM_TYPE_GSENSOR_PARKING},
#ifndef ONE_SENSOR_CONNECT
    {PDT_UI_ACTION_SET_BACK_REC,           HI_PDT_PARAM_TYPE_REC},
#endif
    {PDT_UI_ACTION_SET_BRECORD_TYPE,       HI_PDT_PARAM_RECORD_TYPE},
    {PDT_UI_ACTION_SET_LAPSE_INTERVAL,     HI_PDT_PARAM_TYPE_LAPSE_INTERVAL},
    {PDT_UI_ACTION_SET_RECSPLITTIME,       HI_PDT_PARAM_TYPE_SPLITTIME},
#ifdef CONFIG_MOTIONDETECT_ON
    {PDT_UI_ACTION_SET_MD_SENSITIVITY,       HI_PDT_PARAM_MD_SENSITIVITY},
#endif

};

/* constant data in first level view */
static const PDT_UI_SET_DATA_S s_astFirstLevelItems[] = {
    {PDT_UI_ACTION_SHOW_WORKMODE,          HI_PDT_PARAM_TYPE_BUTT,                 STRING_WORK_MODE,         HI_FALSE},
#ifndef ONE_SENSOR_CONNECT
    {PDT_UI_ACTION_SHOW_VO_SWITCH,          HI_PDT_PARAM_TYPE_BUTT,                STRING_VO_SWITCH,         HI_FALSE},
#endif
    {PDT_UI_ACTION_SHOW_CAM_ATTR,           HI_PDT_PARAM_TYPE_BUTT,                STRING_CAM_ATTR,          HI_TRUE},
    {PDT_UI_ACTION_SHOW_MEDIA_SETTINGS,     HI_PDT_PARAM_TYPE_BUTT,                STRING_MEDIA,             HI_TRUE},
    {PDT_UI_ACTION_SHOW_SCREEN_BRIGHTNESS,  HI_PDT_PARAM_TYPE_SCREEN_BRIGHTNESS,   STRING_SCREEN_BRIGHTNESS, HI_TRUE},
    {PDT_UI_ACTION_SHOW_SCREEN_DORMANT,     HI_PDT_PARAM_TYPE_SCREEN_DORMANT_MODE, STRING_SCREEN_DORMANT,    HI_TRUE},
    {PDT_UI_ACTION_SHOW_USB_MODE,           HI_PDT_PARAM_TYPE_USB_MODE,            STRING_USB_MODE,          HI_TRUE},
    {PDT_UI_ACTION_SHOW_SENSITIVITY,        HI_PDT_PARAM_TYPE_GSENSOR_SENSITIVITY, STRING_SENSITIVITY,       HI_TRUE},
    {PDT_UI_ACTION_SHOW_PARKING,            HI_PDT_PARAM_TYPE_GSENSOR_PARKING,     STRING_PARKING,           HI_TRUE},
#ifndef ONE_SENSOR_CONNECT
    {PDT_UI_ACTION_SET_PREVIEWPIP,          HI_PDT_PARAM_TYPE_BUTT,                STRING_PREVIEWPIP,        HI_FALSE},
#endif
    {PDT_UI_ACTION_SHOW_TIME_PAGE,          HI_PDT_PARAM_TYPE_BUTT,                STRING_SET_TIME,          HI_TRUE},
    {PDT_UI_ACTION_SHOW_LANGUAGE,           HI_PDT_PARAM_TYPE_LANGUAGE,            STRING_LANGUAGE,          HI_TRUE},
    {PDT_UI_ACTION_SHOW_FORMAT,             HI_PDT_PARAM_TYPE_BUTT,                STRING_FORMAT_SD,         HI_TRUE},
    {PDT_UI_ACTION_SHOW_RESET,              HI_PDT_PARAM_TYPE_BUTT,                STRING_RESET_SYSTEM,      HI_TRUE},
#ifdef CONFIG_WIFI_ON
    {PDT_UI_ACTION_SHOW_WIFI_STATUS,         HI_PDT_PARAM_TYPE_BUTT,                STRING_WIFI,             HI_FALSE},
#endif
    {PDT_UI_ACTION_SHOW_DEBUG,              HI_PDT_PARAM_TYPE_BUTT,                STRING_DEBUG,             HI_FALSE},
    {PDT_UI_ACTION_SHOW_ABOUT,              HI_PDT_PARAM_TYPE_BUTT,                STRING_ABOUT,             HI_TRUE},
};

static const PDT_UI_SET_DATA_S s_astRecordMediaCamItems[] = {
    {PDT_UI_ACTION_SHOW_MEDIAMODE,        HI_PDT_PARAM_TYPE_MEDIAMODE,        STRING_RESOLUTION,   HI_TRUE},
    {PDT_UI_ACTION_SET_ENC_PAYLOAD_TYPE,  HI_PDT_PARAM_TYPE_ENC_PAYLOAD_TYPE, STRING_VIDEO_ENCODE, HI_FALSE},
    {PDT_UI_ACTION_SHOW_FLIP,              HI_PDT_PARAM_TYPE_FLIP,             STRING_FLIP,         HI_TRUE},
    {PDT_UI_ACTION_SHOW_MIRROR,            HI_PDT_PARAM_TYPE_MIRROR,           STRING_MIRROR,       HI_TRUE},
    {PDT_UI_ACTION_SHOW_LDC,              HI_PDT_PARAM_TYPE_LDC,              STRING_LDC,          HI_TRUE},
    {PDT_UI_ACTION_SHOW_WDR,              HI_PDT_PARAM_TYPE_WDR,              STRING_WDR,          HI_TRUE},
    {PDT_UI_ACTION_SHOW_VIDEOMODE,        HI_PDT_PARAM_TYPE_VIDEOMODE,        STRING_VIDEOMODE,    HI_TRUE},
    {PDT_UI_ACTION_SET_OSD,               HI_PDT_PARAM_TYPE_OSD,              STRING_OSD,          HI_FALSE},
};

static const PDT_UI_SET_DATA_S s_astPhotoMediaCamItems[] = {
    {PDT_UI_ACTION_SHOW_MEDIAMODE,        HI_PDT_PARAM_TYPE_MEDIAMODE,        STRING_RESOLUTION,   HI_TRUE},
    {PDT_UI_ACTION_SHOW_FLIP,              HI_PDT_PARAM_TYPE_FLIP,             STRING_FLIP,         HI_TRUE},
    {PDT_UI_ACTION_SHOW_MIRROR,            HI_PDT_PARAM_TYPE_MIRROR,           STRING_MIRROR,       HI_TRUE},
    {PDT_UI_ACTION_SHOW_LDC,              HI_PDT_PARAM_TYPE_LDC,              STRING_LDC,          HI_TRUE},
    {PDT_UI_ACTION_SET_OSD,               HI_PDT_PARAM_TYPE_OSD,              STRING_OSD,          HI_FALSE},
};


static const PDT_UI_SET_DATA_S s_astRecordMediaCommonItems[] = {
    {PDT_UI_ACTION_SET_AUDIO,           HI_PDT_PARAM_TYPE_AUDIO,        STRING_AUDIO,       HI_FALSE},
    {PDT_UI_ACTION_SHOW_VOLUME,         HI_PDT_PARAM_TYPE_VOLUME,       STRING_VOLUME,      HI_TRUE},
    {PDT_UI_ACTION_SET_KEYTONE,         HI_PDT_PARAM_TYPE_KEYTONE,      STRING_KEYTONE,     HI_FALSE},
    {PDT_UI_ACTION_SET_BOOTSOUND,       HI_PDT_PARAM_TYPE_BOOTSOUND,    STRING_BOOTSOUND,   HI_FALSE},
#ifdef CONFIG_MOTIONDETECT_ON
    {PDT_UI_ACTION_SHOW_MD_SENSITIVITY,   HI_PDT_PARAM_MD_SENSITIVITY,    STRING_MD_SENSITIVITY,   HI_TRUE},
#endif
#ifndef ONE_SENSOR_CONNECT
    {PDT_UI_ACTION_SET_BACK_REC,            HI_PDT_PARAM_TYPE_REC,                 STRING_BACK_REC,          HI_FALSE},
#endif
    {PDT_UI_ACTION_SHOW_RECSPLITTIME,   HI_PDT_PARAM_TYPE_SPLITTIME,    STRING_SPLITTIME,   HI_TRUE},
    {PDT_UI_ACTION_SHOW_BRECORD_TYPE,   HI_PDT_PARAM_RECORD_TYPE,        STRING_REC_TYPE,   HI_TRUE},
    {PDT_UI_ACTION_SHOW_LAPSE_INTERVAL,   HI_PDT_PARAM_TYPE_LAPSE_INTERVAL,    STRING_LAPSE_REC_TIME,   HI_TRUE},
};

static const PDT_UI_SET_DATA_S s_astPhotoMediaCommonItems[] = {
    {PDT_UI_ACTION_SHOW_VOLUME,         HI_PDT_PARAM_TYPE_VOLUME,       STRING_VOLUME,      HI_TRUE},
    {PDT_UI_ACTION_SET_KEYTONE,         HI_PDT_PARAM_TYPE_KEYTONE,      STRING_KEYTONE,     HI_FALSE},
    {PDT_UI_ACTION_SET_BOOTSOUND,       HI_PDT_PARAM_TYPE_BOOTSOUND,    STRING_BOOTSOUND,   HI_FALSE},
};

static HI_S32 UI_GetParamTypeByAction(PDT_UI_ACTION_E enAction, HI_PDT_PARAM_TYPE_E * pType)
{
    HI_U32 i = 0;
    for(i=0; i<UI_ARRAY_SIZE(s_astActionAndParamType); i++)
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
    pstSetRow->resIcon = pstSetData->bHaveSubMenu ? s_resHaveSubMenu : INVALID_HANDLE;
    s32Ret = PDT_UI_COMM_GetStringByID(pstSetData->u32Title, pstSetRow->szTitle, sizeof(pstSetRow->szTitle));
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("GetStringByID failed. %#x\n", s32Ret);
        snprintf(pstSetRow->szTitle, sizeof(pstSetRow->szTitle), "%s", "????");
    }
}


static HI_VOID UI_LoadFirstLevelList(HI_VOID)
{
    HI_S32 s32Ret = 0;
    HI_U32 u32Index = 0;
    HIGV_DBROW_S stDbRow = {};
    PDT_UI_SET_ROW_S stSetRow = {};
    HI_S32 s32CamID = 0;
    HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkModeState = {};

    s32Ret = HI_PDT_STATEMNG_GetState(&stWorkModeState);
    PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret,"GetState");


    s32CamID = HI_PDT_STATEMNG_GetPreViewCamID();

    for(u32Index = 0; u32Index < UI_ARRAY_SIZE(s_astFirstLevelItems); u32Index++)
    {
        memset(&stDbRow, 0, sizeof(stDbRow));
        memset(&stSetRow, 0, sizeof(stSetRow));

        UI_ParseSetData2SetRow(&s_astFirstLevelItems[u32Index], &stSetRow);

        switch(stSetRow.u32Action)
        {
            case PDT_UI_ACTION_SHOW_WORKMODE:
                if(HI_PDT_WORKMODE_PHOTO == stWorkModeState.enWorkMode)
                {
                    PDT_UI_COMM_GetStringByID(STRING_TAKE_PHOTO, stSetRow.szContent, sizeof(stSetRow.szContent));
                }
                else if(HI_PDT_WORKMODE_NORM_REC == stWorkModeState.enWorkMode)
                {
                    PDT_UI_COMM_GetStringByID(STRING_RECORD, stSetRow.szContent, sizeof(stSetRow.szContent));
                }
                break;

#ifndef ONE_SENSOR_CONNECT
            case PDT_UI_ACTION_SHOW_VO_SWITCH:
                if(PDT_UI_COMM_isBackCamConnecting())
                {
                    if (0 == s32CamID)
                    {
                        PDT_UI_COMM_GetStringByID(STRING_FRONT_CAM, stSetRow.szContent, sizeof(stSetRow.szContent));
                    }
                    else if (1 == s32CamID)
                    {
                        PDT_UI_COMM_GetStringByID(STRING_BACK_CAM, stSetRow.szContent, sizeof(stSetRow.szContent));
                    }
                }
                else
                {
                    continue;  /**back cam disconnet, hide */
                }
                break;


            case PDT_UI_ACTION_SHOW_CAM_ATTR:
                if (0 == s32CamID)
                {
                    PDT_UI_COMM_GetStringByID(STRING_FRONT_CAM, stSetRow.szContent, sizeof(stSetRow.szContent));
                }
                else if (1 == s32CamID)
                {
                    PDT_UI_COMM_GetStringByID(STRING_BACK_CAM, stSetRow.szContent, sizeof(stSetRow.szContent));
                }
                break;
#endif
            case PDT_UI_ACTION_SHOW_VOLUME:
            case PDT_UI_ACTION_SHOW_SCREEN_BRIGHTNESS:
            case PDT_UI_ACTION_SHOW_SCREEN_DORMANT:
            case PDT_UI_ACTION_SHOW_USB_MODE:
            case PDT_UI_ACTION_SHOW_SENSITIVITY:
            case PDT_UI_ACTION_SHOW_PARKING:
            case PDT_UI_ACTION_SHOW_LANGUAGE:
                s32Ret = PDT_UI_COMM_GetParamValueDesc(HI_PDT_WORKMODE_BUTT, s32CamID, stSetRow.u32ParamType,
                        stSetRow.szContent, sizeof(stSetRow.szContent));
                break;

#ifdef CONFIG_WIFI_ON
            case PDT_UI_ACTION_SHOW_WIFI_STATUS:
            {
                HI_BOOL bCurValue = HI_FALSE;
                s32Ret = HI_HAL_WIFI_GetStartedStatus((HI_BOOL*)&bCurValue);
                PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret,"HI_HAL_WIFI_GetStartedStatus");

                if(bCurValue)
                {
                    PDT_UI_COMM_GetStringByID(STRING_ON, stSetRow.szContent, sizeof(stSetRow.szContent));
                }
                else
                {
                    PDT_UI_COMM_GetStringByID(STRING_OFF, stSetRow.szContent, sizeof(stSetRow.szContent));
                }
            }
                break;
#endif

            case PDT_UI_ACTION_SHOW_DEBUG:
#ifdef CONFIG_RAWCAP_ON
                if(HI_PDT_WORKMODE_NORM_REC == stWorkModeState.enWorkMode) /** support debug */
                {
                    s32Ret = PDT_UI_COMM_GetStringByID(HI_FALSE == s_b32DebugOn ? STRING_OFF : STRING_ON,
                    stSetRow.szContent, sizeof(stSetRow.szContent));
                    PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret, "GetStringByID");
                }
                else /** unsupport debug, hide */
                {
                    continue;
                }
#else
                continue;
#endif
                break;

            case PDT_UI_ACTION_SET_PREVIEWPIP:
            {
                HI_PDT_PARAM_MEDIA_COMM_CFG_S stMediaCommCfg = {};
                s32Ret = HI_PDT_PARAM_GetMediaCommCfg(&stMediaCommCfg);
                PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "HI_PDT_PARAM_GetMediaCommCfg");
                if(stMediaCommCfg.stVideoOutCfg.astDispCfg[0].u32WindowCnt <= 1)
                {
                    continue;
                }
                if(PDT_UI_COMM_isBackCamConnecting())
                {
                    s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_PREVIEWPIP,&stSetRow.s32Value);
                    PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret, "Get comm param");
                    if (stSetRow.s32Value)
                    {
                        PDT_UI_COMM_GetStringByID(STRING_ON, stSetRow.szContent, sizeof(stSetRow.szContent));
                    }
                    else
                    {
                        PDT_UI_COMM_GetStringByID(STRING_OFF, stSetRow.szContent, sizeof(stSetRow.szContent));
                    }
                }
                else
                {
                     continue;
                }
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

static HI_BOOL UI_IsSupportVideoMode(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;

    HI_PDT_PARAM_CAM_MEDIAMODE_CFG_S stCamCfg;
    HI_PDT_PARAM_CAM_CONTEXT_S astCamCtx[HI_PDT_MEDIA_VCAP_DEV_MAX_CNT];
    HI_S32 i = 0;
    s32Ret = HI_PDT_PARAM_GetMediaModeCfg(HI_PDT_WORKMODE_NORM_REC,&stCamCfg);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "HI_PDT_PARAM_GetMediaModeCfg");
    for(i = 0;i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT;i++)
    {
        astCamCtx[i].stMediaModeCfg.s32CamID = stCamCfg.astMediaModeCfg[i].s32CamID;
        astCamCtx[i].stMediaModeCfg.enMediaMode = stCamCfg.astMediaModeCfg[i].enMediaMode;
        s32Ret = HI_PDT_PARAM_GetCamParam(HI_PDT_WORKMODE_NORM_REC, i, HI_PDT_PARAM_TYPE_CAM_STATUS,
            &astCamCtx[i].bEnable);
    }
    HI_PDT_MEDIA_CFG_S stMediaCfg;
    s32Ret = HI_PDT_PARAM_GetMediaCfg(HI_PDT_WORKMODE_NORM_REC, astCamCtx, &stMediaCfg);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "HI_PDT_PARAM_GetMediaCfg");

    if(stMediaCfg.stVideoCfg.stVcapCfg.astVcapDevAttr[0].astVcapPipeAttr[0].stFrameRate.s32SrcFrameRate < 30)
    {
        return HI_FALSE;
    }

    return HI_TRUE;
}


static HI_VOID UI_LoadMediaCamSettings(const PDT_UI_SET_DATA_S astRecordMediaCamItems[], HI_S32 S32Size)
{
    HI_S32 s32Ret = 0;
    HI_U32 u32Index = 0;
    HI_S32 s32Value = 0;
    HIGV_DBROW_S stDbRow = {};
    PDT_UI_SET_ROW_S stSetRow = {};
    HI_S32 s32CamID = 0;
    HI_BOOL bValue = HI_FALSE;

    s32CamID = HI_PDT_STATEMNG_GetPreViewCamID();

    HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkModeState = {};
    s32Ret = HI_PDT_STATEMNG_GetState(&stWorkModeState);
    if(s32Ret)
    {
        MLOGE("Get state failed %#x\n", s32Ret);
    }

    /* work mode common items */
    for(u32Index = 0; u32Index < S32Size; u32Index++)
    {
        memset(&stDbRow, 0, sizeof(stDbRow));
        memset(&stSetRow, 0, sizeof(stSetRow));

        /* static data */
        UI_ParseSetData2SetRow(&astRecordMediaCamItems[u32Index], &stSetRow);

        /* dynamic data */
        stSetRow.s32CamID = s32CamID;
        s32Ret = PDT_UI_COMM_GetParamValueDesc(stWorkModeState.enWorkMode, s32CamID, stSetRow.u32ParamType, stSetRow.szContent, sizeof(stSetRow.szContent));
        if(s32Ret)
        {
            MLOGE("GetCurParamString %x\n", s32Ret);
        }

        switch((PDT_UI_ACTION_E)stSetRow.u32Action)
        {
            case PDT_UI_ACTION_SET_ENC_PAYLOAD_TYPE:
                s32Ret = HI_PDT_PARAM_GetCamParam(stWorkModeState.enWorkMode, s32CamID, stSetRow.u32ParamType, &s32Value);
                if(s32Ret)
                {
                    MLOGE("GetCamParam failed %#x\n", s32Ret);
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
            case PDT_UI_ACTION_SET_OSD:
               s32Ret = HI_PDT_PARAM_GetCamParam(stWorkModeState.enWorkMode,s32CamID, stSetRow.u32ParamType, &bValue);
               if(s32Ret)
               {
                   MLOGE("GetWorkModeParam failed %#x\n", s32Ret);
               }
               stSetRow.s32Value = !bValue;
               break;
            case PDT_UI_ACTION_SHOW_VIDEOMODE:
                if (HI_FALSE == UI_IsSupportVideoMode())
                {
                    continue;
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

static HI_VOID UI_LoadMediaSettings(const PDT_UI_SET_DATA_S astRecordMediaCamItems[], HI_S32 S32Size)
{
    HI_S32 s32Ret = 0;
    HI_U32 u32Index = 0;
    HI_S32 s32Value = 0;
    HI_S32 s32CamID = 0;
    HIGV_DBROW_S stDbRow = {};
    PDT_UI_SET_ROW_S stSetRow = {};
    HI_REC_TYPE_E enRecType;

    for(u32Index = 0; u32Index < S32Size; u32Index++)
    {
        memset(&stDbRow, 0, sizeof(stDbRow));
        memset(&stSetRow, 0, sizeof(stSetRow));

        UI_ParseSetData2SetRow(&astRecordMediaCamItems[u32Index], &stSetRow);

        s32Ret = PDT_UI_COMM_GetParamValueDesc(HI_PDT_WORKMODE_BUTT, s32CamID, stSetRow.u32ParamType, stSetRow.szContent, sizeof(stSetRow.szContent));
        if(s32Ret)
        {
            MLOGE("GetCurParamString %x\n", s32Ret);
        }

        switch((PDT_UI_ACTION_E)stSetRow.u32Action)
        {
            case PDT_UI_ACTION_SET_KEYTONE:
#ifdef CONFIG_SYS_SOUND_ON
                s32Ret = HI_PDT_PARAM_GetCommParam(stSetRow.u32ParamType, &s32Value);
                if(s32Ret)
                {
                    MLOGE("GetWorkModeParam failed %#x\n", s32Ret);
                }
                stSetRow.s32Value = (HI_BOOL)s32Value ? HI_FALSE : HI_TRUE;
                break;
#else
                continue;
#endif

            case PDT_UI_ACTION_SET_BOOTSOUND:
                s32Ret = HI_PDT_PARAM_GetCommParam(stSetRow.u32ParamType, &s32Value);
                if(s32Ret)
                {
                    MLOGE("GetWorkModeParam failed %#x\n", s32Ret);
                }
                stSetRow.s32Value = (HI_BOOL)s32Value ? HI_FALSE : HI_TRUE;
                break;

            case PDT_UI_ACTION_SET_AUDIO:
                s32Ret = HI_PDT_PARAM_GetCommParam(stSetRow.u32ParamType, &s32Value);
                if(s32Ret)
                {
                    MLOGE("GetWorkModeParam failed %#x\n", s32Ret);
                }
                stSetRow.s32Value = (HI_BOOL)s32Value ? HI_FALSE : HI_TRUE;
                break;
            case PDT_UI_ACTION_SHOW_LAPSE_INTERVAL:
                    s32CamID = HI_PDT_STATEMNG_GetPreViewCamID();
                    s32Ret = HI_PDT_PARAM_GetCamParam(HI_PDT_WORKMODE_NORM_REC, s32CamID, HI_PDT_PARAM_RECORD_TYPE, &enRecType);
                    if(HI_SUCCESS != s32Ret)
                    {
                        MLOGE("GetCamParam failed! s32Ret: %#x\n", s32Ret);
                    }
                    if(HI_REC_TYPE_NORMAL == enRecType)   /** if rec_type is norm_rec, hide this contex*/
                    {
                        continue;
                    }
                break;
            case PDT_UI_ACTION_SET_VOLUME:
                break;

            case PDT_UI_ACTION_SET_BACK_REC:
                if(PDT_UI_COMM_isBackCamConnecting())
                {
                    s32Ret = PDT_UI_COMM_GetParamValueDesc(HI_PDT_WORKMODE_BUTT, BACK_CAM_ID, stSetRow.u32ParamType,
                            stSetRow.szContent, sizeof(stSetRow.szContent));
                }
                else
                {
                    continue;  /**back cam disconnet, hide */
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


static HI_S32 UI_LoadSettingValueSet(PDT_UI_ACTION_E enAction)
{
    /*get value set and current value*/

    /* add to DDB */

    HI_S32 s32Ret = 0;
    HI_S32 s32Value = 0;
    HI_U32 i = 0;
    HI_PDT_PARAM_TYPE_E enParamType = HI_PDT_PARAM_TYPE_BUTT;
    HI_PDT_ITEM_VALUESET_S stValueSet = {};
    HI_TIMEDTASK_ATTR_S stTimedTaskAttr = {};
    HI_S32 s32CamID = 0;

    HIGV_DBROW_S stDbRow = {};
    PDT_UI_SET_ROW_S stSetRow = {};

    HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkModeState = {};
    s32Ret = HI_PDT_STATEMNG_GetState(&stWorkModeState);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "GetState");

    /*Get current CamID*/

    s32CamID = HI_PDT_STATEMNG_GetPreViewCamID();

    s32Ret = UI_GetParamTypeByAction(enAction, &enParamType);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "UI_GetParamTypeByAction");

    switch(enParamType)
    {
        case HI_PDT_PARAM_TYPE_MEDIAMODE:
        case HI_PDT_PARAM_TYPE_ENC_PAYLOAD_TYPE:
        case HI_PDT_PARAM_TYPE_WDR:
        case HI_PDT_PARAM_TYPE_VIDEOMODE:
        case HI_PDT_PARAM_RECORD_TYPE:
        case HI_PDT_PARAM_TYPE_LAPSE_INTERVAL:
        case HI_PDT_PARAM_TYPE_LDC:
        case HI_PDT_PARAM_TYPE_FLIP:
        case HI_PDT_PARAM_TYPE_MIRROR:
        case HI_PDT_PARAM_TYPE_SPLITTIME:
#ifdef CONFIG_MOTIONDETECT_ON
        case HI_PDT_PARAM_MD_SENSITIVITY:
#endif
            s32Ret = HI_PDT_PARAM_GetCamParam(stWorkModeState.enWorkMode, s32CamID, enParamType, &s32Value);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret,"GetWorkModeParam");

            s32Ret = HI_PDT_PARAM_GetCamItemValues(stWorkModeState.enWorkMode, s32CamID, enParamType, &stValueSet);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret,"GetWorkModeItemValues");
            break;

        case HI_PDT_PARAM_TYPE_SCREEN_BRIGHTNESS:
        case HI_PDT_PARAM_TYPE_VOLUME:
        case HI_PDT_PARAM_TYPE_USB_MODE:
        case HI_PDT_PARAM_TYPE_GSENSOR_SENSITIVITY:
        case HI_PDT_PARAM_TYPE_GSENSOR_PARKING:
        case HI_PDT_PARAM_TYPE_LANGUAGE:
            s32Ret = HI_PDT_PARAM_GetCommParam(enParamType, &s32Value);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "GetCommParam");
            s32Ret = HI_PDT_PARAM_GetCommItemValues(enParamType, &stValueSet);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret,"GetCommItemValues");
            break;

        case HI_PDT_PARAM_TYPE_SCREEN_DORMANT_MODE:
            s32Ret = HI_PDT_PARAM_GetCommParam(enParamType, &stTimedTaskAttr);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret,"GetCommParam");
            s32Value = 0;
            if(stTimedTaskAttr.bEnable)
            {
                s32Value = stTimedTaskAttr.u32Time_sec;
            }
            s32Ret = HI_PDT_PARAM_GetCommItemValues(enParamType, &stValueSet);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret,"GetCommItemValues");
            break;

        default:
            MLOGE("param type %u not processed\n", enParamType);
            return HI_FAILURE;
    }

    for(i = 0; i < stValueSet.s32Cnt && i < HI_PDT_ITEM_VALUESET_MAX_NUM; i++)
    {
        memset(&stDbRow,  0, sizeof(stDbRow));
        memset(&stSetRow, 0, sizeof(stSetRow));

        s32Ret = PDT_UI_COMM_Translate(stValueSet.astValues[i].szDesc, stSetRow.szTitle, sizeof(stSetRow.szTitle));
        if(HI_SUCCESS != s32Ret)
        {
            snprintf(stSetRow.szTitle, sizeof(stSetRow.szTitle), "%s", stValueSet.astValues[i].szDesc);
        }

        if(s32Value == stValueSet.astValues[i].s32Value)
        {
            snprintf(stSetRow.szContent, sizeof(stSetRow.szContent), "*");
        }

        stSetRow.resIcon = INVALID_HANDLE;
        stSetRow.u32Action = enAction;
        stSetRow.u32ParamType = enParamType;
        stSetRow.s32CamID = s32CamID;
        stSetRow.s32Value = stValueSet.astValues[i].s32Value;

        stDbRow.pData = &stSetRow;
        stDbRow.Size = sizeof(PDT_UI_SET_ROW_S);
        MLOGD("Load Values [action:%d, camID:%d, type:%d, value:%d]\n", enAction, s32CamID, enParamType, stSetRow.s32Value);

        s32Ret = HI_GV_DDB_Append(s_DataBufferHdl, &stDbRow);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("DDB_Append failed!\n");
        }
    }

    return  HI_SUCCESS;
}

static HI_VOID UI_SET_ShowList(HI_VOID)
{
    HI_S32 s32Ret = 0;
    PDT_UI_ACTION_E enAction = s_astLevels[s_u32CurLevel].enAction;
    HI_U32 u32StartIndex = s_astLevels[s_u32CurLevel].u32StartRowIndex;
    HI_U32 u32SelectIndex = s_astLevels[s_u32CurLevel].u32ActiveRowIndex;

    MLOGD("u32StartIndex = %u, u32SelectIndex = %u\n", u32StartIndex, u32SelectIndex);

    HI_GV_DDB_EnableDataChange(s_DataBufferHdl, HI_FALSE);
    HI_GV_DDB_Clear(s_DataBufferHdl);


    HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkModeState = {};
    s32Ret = HI_PDT_STATEMNG_GetState(&stWorkModeState);
    PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret,"GetState");

    switch(enAction)
    {
        case PDT_UI_ACTION_SHOW_FIRST_VIEW:
            UI_LoadFirstLevelList();
            break;
        case PDT_UI_ACTION_SHOW_CAM_ATTR:
            if(HI_PDT_WORKMODE_PHOTO == stWorkModeState.enWorkMode)
            {
                UI_LoadMediaCamSettings(s_astPhotoMediaCamItems, UI_ARRAY_SIZE(s_astPhotoMediaCamItems));
            }
            else
            {
                UI_LoadMediaCamSettings(s_astRecordMediaCamItems, UI_ARRAY_SIZE(s_astRecordMediaCamItems));
            }
            break;
        case PDT_UI_ACTION_SHOW_MEDIA_SETTINGS:
            if(HI_PDT_WORKMODE_PHOTO == stWorkModeState.enWorkMode)
            {
                UI_LoadMediaSettings(s_astPhotoMediaCommonItems, UI_ARRAY_SIZE(s_astPhotoMediaCommonItems));
            }
            else
            {
                UI_LoadMediaSettings(s_astRecordMediaCommonItems, UI_ARRAY_SIZE(s_astRecordMediaCommonItems));
            }

            break;

        case PDT_UI_ACTION_SHOW_MEDIAMODE:
        case PDT_UI_ACTION_SHOW_VOLUME:
        case PDT_UI_ACTION_SHOW_SCREEN_BRIGHTNESS:
        case PDT_UI_ACTION_SHOW_SCREEN_DORMANT:
        case PDT_UI_ACTION_SHOW_USB_MODE:
        case PDT_UI_ACTION_SHOW_SENSITIVITY:
        case PDT_UI_ACTION_SHOW_PARKING:
        case PDT_UI_ACTION_SHOW_LANGUAGE:
        case PDT_UI_ACTION_SHOW_WDR:
        case PDT_UI_ACTION_SHOW_FLIP:
        case PDT_UI_ACTION_SHOW_MIRROR:
        case PDT_UI_ACTION_SHOW_VIDEOMODE:
        case PDT_UI_ACTION_SHOW_BRECORD_TYPE:
        case PDT_UI_ACTION_SHOW_LAPSE_INTERVAL:
        case PDT_UI_ACTION_SHOW_LDC:
        case PDT_UI_ACTION_SHOW_RECSPLITTIME:
#ifdef CONFIG_MOTIONDETECT_ON
        case PDT_UI_ACTION_SHOW_MD_SENSITIVITY:
 #endif
            UI_LoadSettingValueSet((PDT_UI_ACTION_E)(enAction + PDT_UI_SET_AND_SHOW_OFFSET));
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

    s32Ret = HI_GV_Widget_Paint(SET_LISTBOX, NULL);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("Widget_Paint: %#x\n", s32Ret);
    }
}

static HI_S32 UI_SET_OnReceiveMsgResult(HI_EVENT_S* pstEvent)
{
    HI_APPCOMM_CHECK_POINTER(pstEvent, HI_FAILURE);
    MLOGD("EventID:%#x, arg1:%#x, arg2:%#x, result:%d\n", pstEvent->EventID, pstEvent->arg1,
          pstEvent->arg2,pstEvent->s32Result);

    PDT_UI_WINMNG_FinishWindow(BUSY_PAGE);

    if (PDT_UI_WINMNG_WindowIsShow(HOME_PAGE))
    {
        return HI_SUCCESS;
    }

    UI_SET_ShowList();

    return HI_SUCCESS;
}


static HI_VOID UI_SET_Back(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;

    if( 0 == s_u32CurLevel  ||  s_u32CurLevel >= MAX_SET_LEVELS)
    {
#ifdef CFG_TIME_STAMP_ON
        PDT_UI_COMM_SwitchPageTime_begin();
#endif
        s32Ret = PDT_UI_WINMNG_FinishWindow(SET_PAGE);
        PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret, "Finish SET_PAGE Window");
        HI_EVENT_S stEvent;
        stEvent.arg1 = 0;
        s32Ret = HI_EVTHUB_GetEventHistory(HI_EVENT_RECMNG_MUXER_CNT_CHANGE, &stEvent);
        if(((HI_SUCCESS != s32Ret) ||(-1 == stEvent.arg1)) && PDT_UI_COMM_isMdReady())
        {
            HI_MESSAGE_S stMessage = {};
            s32Ret = PDT_UI_COMM_CheckSd();
            if (s32Ret != HI_SUCCESS)
            {
                MLOGE(" SD state is not normal \n");
                return ;
            }

            stMessage.what = HI_EVENT_STATEMNG_START;
            stMessage.arg2 = HI_PDT_WORKMODE_NORM_REC;
            s32Ret = HI_PDT_STATEMNG_SendMessage(&stMessage);
            PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret, "SendMessage");
        }
        else
        {
            MLOGI("task[%d] is not zero ,can not start rec!\n",stEvent.arg1);
        }
        s_u32CurLevel = 0;
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
        UI_SET_ShowList();
    }
}
HI_S32 PDT_UI_SET_OnShow(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGI("\n");
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HI_GV_ADM_GetDDBHandle(DATAMODEL_SETTINGS, &s_DataBufferHdl);
    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("GetDDBHandle failed, s32Ret:%d\n",s32Ret);
        return s32Ret;
    }

    s32Ret = HI_GV_Res_CreateID(ICON_HAVE_SUB_MENU_PATH, HIGV_RESTYPE_IMG, &s_ResIcon);
    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("CreateID failed, s32Ret:%d\n",s32Ret);
    }
    s32Ret = HI_GV_Res_GetResInfo(s_ResIcon, &s_resHaveSubMenu);
    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("GetResInfo failed, s32Ret:%d\n",s32Ret);
    }

    UI_SET_ShowList();

    s32Ret = HI_GV_Widget_Active(SET_LISTBOX);
    s32Ret |= HI_GV_List_SetStartItem(SET_LISTBOX, s_astLevels[s_u32CurLevel].u32StartRowIndex);
    s32Ret |= HI_GV_List_SetSelItem(SET_LISTBOX, s_astLevels[s_u32CurLevel].u32ActiveRowIndex);
    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("SetSelItem failed, s32Ret:%d\n",s32Ret);
        return s32Ret;
    }

#ifdef CFG_TIME_STAMP_ON
    PDT_UI_COMM_SwitchPageTime_end();
#endif
    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_SET_OnHide(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGD("\n");
    HI_S32 s32Ret = HI_SUCCESS;
    s32Ret = HI_GV_Res_ReleaseResInfo(s_ResIcon);
    if (s32Ret)
    {
        MLOGE("ReleaseResInfo failed, s32Ret:%d\n",s32Ret);
    }
    s32Ret = HI_GV_Res_DestroyID(s_ResIcon);
    if (s32Ret)
    {
        MLOGE("DestroyID failed, s32Ret:%d\n",s32Ret);
    }
    s_u32CurLevel = 0;
    memset(s_astLevels, 0, sizeof(PDT_UI_SET_LEVEL_S)*MAX_SET_LEVELS);
    s_astLevels[s_u32CurLevel].enAction = PDT_UI_ACTION_SHOW_FIRST_VIEW;

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_SET_OnTimer(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGD("wParam: %#lx, lParam = %#lx\n", (HI_UL)wParam, (HI_UL)lParam);

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_SET_OnEvent(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGD("wParam: %#lx, lParam = %#lx\n", (HI_UL)wParam, (HI_UL)lParam);

    return HIGV_PROC_GOON;
}


HI_S32 PDT_UI_SET_OnKeyUp(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGD("wParam: %#lx, lParam = %#lx\n", (HI_UL)wParam, (HI_UL)lParam);


    return HIGV_PROC_GOON;
}

#ifndef ONE_SENSOR_CONNECT
HI_S32 PDT_UI_SET_VoSwitch(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_MESSAGE_S stMessage;

    HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkModeState;
    s32Ret = HI_PDT_STATEMNG_GetState(&stWorkModeState);
    if (stWorkModeState.bStateMngInProgress)
    {
        MLOGE("WorkMode: %u, StateMngInProgress!\n", stWorkModeState.enWorkMode);
        return HI_FAILURE;
    }

    s32Ret = PDT_UI_WINMNG_StartWindow(BUSY_PAGE, HI_FALSE);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "StartWindow");

    memset(&stMessage, 0, sizeof(HI_MESSAGE_S));
    HI_S32 s32PreviewCamID = -1;

    s32PreviewCamID = HI_PDT_STATEMNG_GetPreViewCamID();
    s32PreviewCamID = (s32PreviewCamID + 1) % HI_PDT_MEDIA_VCAP_DEV_MAX_CNT;

    stMessage.what = HI_EVENT_STATEMNG_VO_SWITCH;
    memcpy(stMessage.aszPayload,&s32PreviewCamID,sizeof(HI_S32));

    MLOGD("VoSwitch -----> [CamID:%d]\n", s32PreviewCamID);
    s32Ret = PDT_UI_COMM_SendAsyncMsg(&stMessage, UI_SET_OnReceiveMsgResult);
    if(s32Ret)
    {
        MLOGE("SendAsyncMsg failed. %x\n", s32Ret);
        s32Ret = PDT_UI_WINMNG_FinishWindow(BUSY_PAGE);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "FinishWindow");
    }

    return HI_SUCCESS;
}


static HI_S32 UI_SET_BackRec(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_MESSAGE_S stMessage;
    HI_S32 s32Value = 0;

    HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkModeState;
    s32Ret = HI_PDT_STATEMNG_GetState(&stWorkModeState);
    if (stWorkModeState.bStateMngInProgress)
    {
        MLOGE("WorkMode: %u, StateMngInProgress!\n", stWorkModeState.enWorkMode);
        return HI_FAILURE;
    }

    s32Ret = HI_PDT_PARAM_GetCamParam(HI_PDT_WORKMODE_BUTT, BACK_CAM_ID, HI_PDT_PARAM_TYPE_REC, &s32Value);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret,"GetWorkModeParam");

    memset(&stMessage, 0, sizeof(HI_MESSAGE_S));
    s32Value = !s32Value;

    stMessage.what = HI_EVENT_STATEMNG_SETTING;
    stMessage.arg1 = HI_PDT_PARAM_TYPE_REC;
    stMessage.arg2 = HI_PDT_WORKMODE_NORM_REC;
    HI_S32 *ps32Value = (HI_S32*)stMessage.aszPayload;
    *ps32Value = s32Value;

    s32Ret = PDT_UI_WINMNG_StartWindow(BUSY_PAGE, HI_FALSE);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "StartWindow");

    s32Ret = PDT_UI_COMM_SendAsyncMsg(&stMessage, UI_SET_OnReceiveMsgResult);
    if(s32Ret)
    {
        MLOGE("SendAsyncMsg failed. %x\n", s32Ret);
        s32Ret = PDT_UI_WINMNG_FinishWindow(BUSY_PAGE);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "FinishWindow");
    }

    return HI_SUCCESS;
}

#endif

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
    HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkModeState = {};
    HI_S32 s32CurValue = 0;

    s32Ret = HI_PDT_STATEMNG_GetState(&stWorkModeState);
    PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret,"GetState");

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

    s32Ret = HI_GV_DDB_GetRow(s_DataBufferHdl, u32ActiveRow, &stDbRow);  /* get data */
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("DDB GetRow failed. %#x\n", s32Ret);
        return;
    }

    switch(stSetRow.u32Action)
    {

        case PDT_UI_ACTION_SHOW_WORKMODE:
            UI_UpdateCurLevelInfo(u32StartRow, u32ActiveRow);

            HI_PDT_WORKMODE_E enWorkmode = HI_PDT_WORKMODE_BUTT;
            if(HI_PDT_WORKMODE_NORM_REC == stWorkModeState.enWorkMode)
            {
                enWorkmode = HI_PDT_WORKMODE_PHOTO;
            }
            else
            {
                enWorkmode = HI_PDT_WORKMODE_NORM_REC;
            }
            s32Ret = PDT_UI_WINMNG_StartWindow(BUSY_PAGE, HI_FALSE);
            if (s32Ret)
            {
                MLOGE("Window process failed. %x\n", s32Ret);
            }

            stMsg.what = HI_EVENT_STATEMNG_SWITCH_WORKMODE;
            stMsg.arg2 = enWorkmode;
            s32Ret = PDT_UI_COMM_SendAsyncMsg(&stMsg, UI_SET_OnReceiveMsgResult);
            if(s32Ret)
            {
                MLOGE("SendAsyncMsg failed. %x\n", s32Ret);
                s32Ret = PDT_UI_WINMNG_FinishWindow(BUSY_PAGE);
                PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret,"FinishWindow");
            }
            UI_SET_ShowList();
            break;

#ifndef ONE_SENSOR_CONNECT
        case PDT_UI_ACTION_SHOW_VO_SWITCH:
            UI_UpdateCurLevelInfo(u32StartRow, u32ActiveRow);
            s32Ret = PDT_UI_SET_VoSwitch();
            if(s32Ret)
            {
                MLOGE("PDT_UI_SET_VoSwitch %x\n", s32Ret);
            }
            UI_SET_ShowList();
            break;
        case PDT_UI_ACTION_SET_BACK_REC:
            UI_UpdateCurLevelInfo(u32StartRow, u32ActiveRow);
            s32Ret = UI_SET_BackRec();
            if(s32Ret)
            {
                MLOGE("PDT_UI_SET_VoSwitch %x\n", s32Ret);
            }
            UI_SET_ShowList();
            break;
#endif
        case PDT_UI_ACTION_SHOW_CAM_ATTR:
        case PDT_UI_ACTION_SHOW_MEDIA_SETTINGS:
        case PDT_UI_ACTION_SHOW_MEDIAMODE:
        case PDT_UI_ACTION_SHOW_VOLUME:
        case PDT_UI_ACTION_SHOW_SCREEN_BRIGHTNESS:
        case PDT_UI_ACTION_SHOW_SCREEN_DORMANT:
        case PDT_UI_ACTION_SHOW_USB_MODE:
        case PDT_UI_ACTION_SHOW_SENSITIVITY:
        case PDT_UI_ACTION_SHOW_PARKING:
        case PDT_UI_ACTION_SHOW_LANGUAGE:
        case PDT_UI_ACTION_SHOW_WDR:
        case PDT_UI_ACTION_SHOW_FLIP:
        case PDT_UI_ACTION_SHOW_MIRROR:
        case PDT_UI_ACTION_SHOW_VIDEOMODE:
        case PDT_UI_ACTION_SHOW_BRECORD_TYPE:
        case PDT_UI_ACTION_SHOW_LAPSE_INTERVAL:
        case PDT_UI_ACTION_SHOW_LDC:
        case PDT_UI_ACTION_SHOW_RECSPLITTIME:
#ifdef CONFIG_MOTIONDETECT_ON
        case PDT_UI_ACTION_SHOW_MD_SENSITIVITY:
#endif
            UI_UpdateCurLevelInfo(u32StartRow, u32ActiveRow);
            UI_PushLevelInfo(stSetRow.u32Action, 0, 0);
            UI_SET_ShowList();
            break;

        case PDT_UI_ACTION_SHOW_TIME_PAGE:
            UI_UpdateCurLevelInfo(u32StartRow, u32ActiveRow);
            s32Ret = PDT_UI_WINMNG_StartWindow(TIME_SET_WINDOW, HI_FALSE);
            if(s32Ret)
            {
                MLOGE("StartWindow %x\n", s32Ret);
            }
            break;

        case PDT_UI_ACTION_SHOW_FORMAT:
        {
            HI_STORAGEMNG_CFG_S stStorageCfg = {};
            HI_STORAGE_STATE_E enStorageState = HI_STORAGE_STATE_IDEL;

            s32Ret = HI_PDT_PARAM_GetStorageCfg(&stStorageCfg);
            if(s32Ret)
            {
                MLOGE("HI_PDT_PARAM_GetStorageCfg %x\n", s32Ret);
            }

            s32Ret = HI_STORAGEMNG_GetState(stStorageCfg.szMntPath, &enStorageState);
            if(s32Ret)
            {
                MLOGE("HI_STORAGEMNG_GetState %x\n", s32Ret);
            }

            if(HI_STORAGE_STATE_DEV_UNPLUGGED == enStorageState)
            {
                PDT_UI_COMM_ShowAlarm(STRING_NO_SD_CARD);
                MLOGD("sd not exit, can not format sd!\n");
            }
            else
            {
                s32Ret = PDT_UI_WINMNG_StartWindow(SD_FORMAT_WINDOW, HI_FALSE);
                if(s32Ret)
                {
                    MLOGE("StartWindow %x\n", s32Ret);
                }
            }
        }
            break;

        case PDT_UI_ACTION_SHOW_RESET:
            s32Ret = PDT_UI_WINMNG_StartWindow(RESET_FACTORY_WINDOW, HI_FALSE);
            if(s32Ret)
            {
                MLOGE("StartWindow %x\n", s32Ret);
            }
            break;
        case PDT_UI_ACTION_SHOW_ABOUT:
            s32Ret = PDT_UI_WINMNG_StartWindow(ABOUT_WINDOW, HI_FALSE);
            if(s32Ret)
            {
                MLOGE("StartWindow %x\n", s32Ret);
            }
            break;

        /* now do set works */
        case PDT_UI_ACTION_SET_ENC_PAYLOAD_TYPE:
        case PDT_UI_ACTION_SET_OSD:
        case PDT_UI_ACTION_SET_AUDIO:
        case PDT_UI_ACTION_SET_FLIP:
        case PDT_UI_ACTION_SET_MIRROR:
        case PDT_UI_ACTION_SET_LDC:
        case PDT_UI_ACTION_SET_WDR:
        case PDT_UI_ACTION_SET_VIDEOMODE:
        case PDT_UI_ACTION_SET_MEDIAMODE:
        case PDT_UI_ACTION_SET_RECSPLITTIME:
        case PDT_UI_ACTION_SET_BRECORD_TYPE:
        case PDT_UI_ACTION_SET_LAPSE_INTERVAL:
#ifdef CONFIG_MOTIONDETECT_ON
        case PDT_UI_ACTION_SET_MD_SENSITIVITY:
 #endif
            UI_UpdateCurLevelInfo(u32StartRow, u32ActiveRow);
            stMsg.what = HI_EVENT_STATEMNG_SETTING;
            stMsg.arg1 = stSetRow.u32ParamType;
            stMsg.arg2 = stWorkModeState.enWorkMode;
            HI_S32 *ps32Value = (HI_S32*)stMsg.aszPayload;
            *ps32Value = stSetRow.s32Value;

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

        case PDT_UI_ACTION_SET_SCREEN_BRIGHTNESS:
            s32Ret = HI_PDT_PARAM_SetCommParam(stSetRow.u32ParamType, &(stSetRow.s32Value));
            if(s32Ret)
            {
                MLOGE("SetCommParam failed. ParamType:%d, value:%d\n", stSetRow.u32ParamType, stSetRow.s32Value);
            }
#ifdef CONFIG_SCREEN
            s32Ret = HI_HAL_SCREEN_SetLuma(HI_HAL_SCREEN_IDX_0, (HI_U32)stSetRow.s32Value);
            if(s32Ret)
            {
                MLOGE("set screen brightness failed. ParamType:%d, value:%d\n", stSetRow.u32ParamType, stSetRow.s32Value);
            }
#endif
            UI_UpdateCurLevelInfo(u32StartRow, u32ActiveRow);
            UI_SET_ShowList();
            break;

        case PDT_UI_ACTION_SET_SCREEN_DORMANT:
            stTimedTask.bEnable = stSetRow.s32Value > 0 ? HI_TRUE : HI_FALSE;
            stTimedTask.u32Time_sec = stSetRow.s32Value;
            MLOGD("timed task value = %d\n", stSetRow.s32Value);
            s32Ret = HI_PDT_PARAM_SetCommParam(stSetRow.u32ParamType, &stTimedTask);
            if(s32Ret)
            {
                MLOGE("SetCommParam failed. ParamType:%d, value:%d\n", stSetRow.u32ParamType, stSetRow.s32Value);
            }
            HI_POWERCTRL_TASK_E enPwrCtrlType = HI_PWRCTRL_TASK_SCREENDORMANT;
            s32Ret = HI_POWERCTRL_SetTaskAttr(enPwrCtrlType, &stTimedTask);
            if(s32Ret)
            {
                MLOGE("SetTaskAttr failed. PwrCtrlType:%d, value:%d\n", enPwrCtrlType, stSetRow.s32Value);
            }
            UI_UpdateCurLevelInfo(u32StartRow, u32ActiveRow);
            UI_SET_ShowList();
            break;

        case PDT_UI_ACTION_SET_VOLUME:
            MLOGD("volume value = %d\n", stSetRow.s32Value);
            s32Ret = HI_PDT_PARAM_SetCommParam(stSetRow.u32ParamType, &stSetRow.s32Value);
            if(s32Ret)
            {
                MLOGE("SetCommParam failed. ParamType:%d, value:%d\n", stSetRow.u32ParamType, stSetRow.s32Value);
            }

            HI_PDT_PARAM_MEDIA_COMM_CFG_S stMediaCommCfg = {};
            s32Ret = HI_PDT_PARAM_GetMediaCommCfg(&stMediaCommCfg);
            PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret, "HI_PDT_PARAM_GetMediaCommCfg");

            s32Ret = HI_PDT_MEDIA_SetAoVolume(
                stMediaCommCfg.stAudioOutCfg.astAoCfg[0].AoHdl, stSetRow.s32Value);
            PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret, "HI_PDT_MEDIA_SetAoVolume");

            UI_UpdateCurLevelInfo(u32StartRow, u32ActiveRow);
            UI_SET_ShowList();
            break;

        case PDT_UI_ACTION_SET_USB_MODE:
        case PDT_UI_ACTION_SET_SENSITIVITY:
        case PDT_UI_ACTION_SET_PARKING:
            {
                if (stSetRow.u32Action == PDT_UI_ACTION_SET_USB_MODE)
                {
                    HI_USB_MODE_E enUsbMode = HI_USB_MODE_CHARGE;
                    s32Ret = HI_USB_GetMode(&enUsbMode);
                    PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "get usb mode");

                    HI_USB_STATE_E enUsbState = HI_USB_STATE_OUT;
                    s32Ret = HI_USB_GetState(&enUsbState);
                    PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "get usb state");

                    if ((HI_USB_MODE_CHARGE != enUsbMode) && (HI_USB_STATE_OUT != enUsbState))
                    {
                        MLOGW("current usb_mode[%d] usb_state[%d]; can not set usb mode!\n", enUsbMode, enUsbState);
                        return ;
                    }

                    s32Ret = HI_USB_SetMode((HI_USB_MODE_E)stSetRow.s32Value);
                    PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "set usb mode failed");
                    if (HI_SUCCESS == s32Ret)
                    {
                        s32Ret = HI_PDT_PARAM_SetCommParam(stSetRow.u32ParamType, &stSetRow.s32Value);
                        PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret, "SetUsbMode");
                    }
                }
                else if (stSetRow.u32Action == PDT_UI_ACTION_SET_SENSITIVITY)
                {
                    s32Ret = HI_GSENSORMNG_SetSensitity(stSetRow.s32Value);
                    PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret, "SetUsbMode");
                    if (s32Ret == HI_SUCCESS)
                    {
                        s32Ret = HI_PDT_PARAM_SetCommParam(stSetRow.u32ParamType, &stSetRow.s32Value);
                    }
                }
                else if (stSetRow.u32Action == PDT_UI_ACTION_SET_PARKING)
                {
                    s32Ret = HI_PDT_PARAM_SetCommParam(stSetRow.u32ParamType, &stSetRow.s32Value);
                    PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret, "SetUsbMode");
                }
                UI_UpdateCurLevelInfo(u32StartRow, u32ActiveRow);
                UI_SET_ShowList();
            }
            break;

#ifdef CONFIG_WIFI_ON
        case PDT_UI_ACTION_SHOW_WIFI_STATUS:
            UI_UpdateCurLevelInfo(u32StartRow, u32ActiveRow);
            s32Ret = HI_HAL_WIFI_GetStartedStatus((HI_BOOL*)&s32CurValue);
            PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret,"HI_HAL_WIFI_GetStartedStatus");

            if(HI_FALSE == s32CurValue)
            {
                HI_HAL_WIFI_APMODE_CFG_S stApCfg = {};
                HI_HAL_WIFI_CFG_S stCfg = {};

                stCfg.enMode = HI_HAL_WIFI_MODE_AP;
                s32Ret = HI_HAL_WIFI_Init(stCfg.enMode);
                PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret,"HI_HAL_WIFI_Init");

                /* after init wifi,do get ap param and start wifi */
                s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_WIFI_AP, &stApCfg);
                PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret,"GetCommParam");
                memcpy(&stCfg.unCfg.stApCfg, &stApCfg, sizeof(HI_HAL_WIFI_APMODE_CFG_S));

                s32Ret = HI_HAL_WIFI_Start(&stCfg);
                PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret,"HI_HAL_WIFI_Start");
                MLOGD("HI_HAL_WIFI_Start\n");
            }
            else
            {
                s32Ret = HI_HAL_WIFI_Stop();
                PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret,"HI_HAL_WIFI_Stop");

                s32Ret = HI_HAL_WIFI_Deinit();
                PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret,"HI_HAL_WIFI_Deinit");
                MLOGD("HI_HAL_WIFI_Stop\n");
            }
            UI_SET_ShowList();
            break;
#endif

        case PDT_UI_ACTION_SET_DEV_INFO:

            break;

        case PDT_UI_ACTION_SHOW_DEBUG:
#ifdef CONFIG_RAWCAP_ON
            if(HI_PDT_WORKMODE_NORM_REC == stWorkModeState.enWorkMode)
            {
                UI_UpdateCurLevelInfo(u32StartRow, u32ActiveRow);

                memset(&stMsg, 0x0, sizeof(HI_MESSAGE_S));

                stMsg.what = HI_EVENT_STATEMNG_DEBUG_SWITCH;
                stMsg.arg2 = stWorkModeState.enWorkMode;

                s_b32DebugOn = (s_b32DebugOn == HI_FALSE ? HI_TRUE: HI_FALSE);
                char* payload = (stMsg.aszPayload);
                *(HI_BOOL *)payload = s_b32DebugOn;

                s32Ret = PDT_UI_WINMNG_StartWindow(BUSY_PAGE, HI_FALSE);
                PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret, "StartWindow");
                s32Ret = PDT_UI_COMM_SendAsyncMsg(&stMsg, UI_SET_OnReceiveMsgResult);
                if(HI_SUCCESS != s32Ret)
                {
                    MLOGE("SendAsyncMsg failed. %x\n", s32Ret);
                    s_b32DebugOn = (s_b32DebugOn == HI_FALSE ? HI_TRUE : HI_FALSE);
                    s32Ret = PDT_UI_WINMNG_FinishWindow(BUSY_PAGE);
                    PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret, "FinishWindow");
                }
            }
#endif
            break;

        case PDT_UI_ACTION_SET_KEYTONE:
#ifdef CONFIG_SYS_SOUND_ON
            s32Ret = HI_PDT_PARAM_SetCommParam(stSetRow.u32ParamType, &(stSetRow.s32Value));
            if(s32Ret)
            {
                MLOGE("SetCommParam failed. ParamType:%d, value:%d\n", stSetRow.u32ParamType, stSetRow.s32Value);
            }
            UI_UpdateCurLevelInfo(u32StartRow, u32ActiveRow);
            UI_SET_ShowList();
#endif
            break;

        case PDT_UI_ACTION_SET_BOOTSOUND:
            s32Ret = HI_PDT_PARAM_SetCommParam(stSetRow.u32ParamType, &(stSetRow.s32Value));
            if(s32Ret)
            {
                MLOGE("SetCommParam failed. ParamType:%d, value:%d\n", stSetRow.u32ParamType, stSetRow.s32Value);
            }
            UI_UpdateCurLevelInfo(u32StartRow, u32ActiveRow);
            UI_SET_ShowList();
            break;

        case PDT_UI_ACTION_SET_LANGUAGE:
            s32Ret = HI_PDT_PARAM_SetCommParam(stSetRow.u32ParamType, &(stSetRow.s32Value));
            if(s32Ret)
            {
                MLOGE("SetCommParam failed. ParamType:%d, value:%d\n", stSetRow.u32ParamType, stSetRow.s32Value);
            }

            s32Ret = PDT_UI_COMM_GetStringByString(stSetRow.szTitle, szLanId, SETTING_CONTENT_SIZE);
            PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "GetLanIdByString");
            MLOGI("szLanId:%s\n", szLanId);
            s32Ret = HI_GV_Lan_Change(szLanId);
            PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "ChangeLanguage");

            UI_UpdateCurLevelInfo(u32StartRow, u32ActiveRow);
            UI_SET_ShowList();
            break;
        case PDT_UI_ACTION_SET_PREVIEWPIP:
            {
                HI_S32 s32NewValue = 0;
                UI_UpdateCurLevelInfo(u32StartRow, u32ActiveRow);
                s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_PREVIEWPIP,&s32CurValue);
                s32NewValue = (HI_TRUE == (HI_BOOL)s32CurValue) ? HI_FALSE : HI_TRUE;
                memset(&stMsg, 0, sizeof(stMsg));
                stMsg.what = HI_EVENT_STATEMNG_PREVIEW_PIP;
                stMsg.arg2 = stWorkModeState.enWorkMode;
                char* payload = (stMsg.aszPayload);
                *(HI_BOOL*)payload = (HI_BOOL)s32NewValue;

                s32Ret = PDT_UI_WINMNG_StartWindow(BUSY_PAGE, HI_FALSE);
                PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "show busy page");

                s32Ret = PDT_UI_COMM_SendAsyncMsg(&stMsg, UI_SET_OnReceiveMsgResult);
                if (HI_SUCCESS != s32Ret)
                {
                    MLOGE("SendAsyncMsg failed. %x\n", s32Ret);
                    s32Ret = PDT_UI_WINMNG_FinishWindow(BUSY_PAGE);
                    PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "close busy page");
                }
            }
            break;
        default:
            MLOGE("Action %d not processed\n", stSetRow.u32Action);
            break;
    }
}

HI_S32 PDT_UI_SET_OnKeyDown(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGD("wParam: %#lx, lParam = %#lx\n", (HI_UL)wParam, (HI_UL)lParam);
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32CurrentRow = 0;
    HI_U32 u32ItemNum = 0;

    if (PDT_UI_KEY_BACK == wParam)
    {
        UI_SET_Back();
    }
    else if (PDT_UI_KEY_MENU == wParam)
    {
        s32Ret = HI_GV_List_GetSelItem(SET_LISTBOX, &u32CurrentRow);
        s32Ret |= HI_GV_List_GetItemNum(SET_LISTBOX, &u32ItemNum);
        if (s32Ret != HI_SUCCESS)
        {
            MLOGE("Error:%#x\n", s32Ret);
        }

        s32Ret = HI_GV_List_SetSelItem(SET_LISTBOX, (u32CurrentRow+1)%u32ItemNum);
        s32Ret |=HI_GV_Widget_Paint(SET_LISTBOX, NULL);

        if (s32Ret != HI_SUCCESS)
        {
            MLOGE("Error:%#x\n", s32Ret);
            return HIGV_PROC_GOON;
        }
        MLOGD("ItemNum: %d, Current Row:%d\n", u32ItemNum, (u32CurrentRow+1)%u32ItemNum);
    }
    else if(PDT_UI_KEY_EXPAND == wParam)
    {
        s32Ret = HI_GV_List_GetSelItem(SET_LISTBOX, &u32CurrentRow);
        s32Ret |= HI_GV_List_GetItemNum(SET_LISTBOX, &u32ItemNum);
        if (s32Ret != HI_SUCCESS)
        {
            MLOGE("Error:%#x\n", s32Ret);
        }

        u32CurrentRow = u32CurrentRow ? (u32CurrentRow - 1) : (u32ItemNum - 1);
        s32Ret = HI_GV_List_SetSelItem(SET_LISTBOX, u32CurrentRow);
        s32Ret |=HI_GV_Widget_Paint(SET_LISTBOX, NULL);

        if (s32Ret != HI_SUCCESS)
        {
            MLOGE("Error:%#x\n", s32Ret);
            return HIGV_PROC_GOON;
        }
        MLOGD("ItemNum: %d, Current Row:%d\n", u32ItemNum, u32CurrentRow);
    }
    else if (PDT_UI_KEY_OK == wParam)
    {
        UI_SET_OnClickRow();
    }
    else
    {
        MLOGE("KEY CODE %#lx not processed\n", (HI_UL)wParam);
        return HIGV_PROC_GOON;
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


