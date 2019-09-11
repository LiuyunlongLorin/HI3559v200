/**
* Copyright (C), 2016-2020, Hisilicon Tech. Co., Ltd.
* All rights reserved.
*
* @file      hi_LITEPLAYER.h
* @brief     hiLITEPLAYER module header file
* @author    HiMobileCam middleware develop team
* @date      2016.06.06
*/

#ifndef __HI_LITEPLAYER_H__
#define __HI_LITEPLAYER_H__

#include "hi_mw_type.h"
#include "hi_demuxer.h"

/** \addtogroup     PLAYER */
/** @{ */  /** <!-- [PLAYER] */

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#define HI_LITEPLAYER_MAX_FILE_SURFIX_CNT (10)
#define HI_LITEPLAYER_MAX_FILE_SURFIX_LEN (32)
#define HI_LITEPLAYER_FILE_PATH_MAX_LEN (1024)
#define HI_LITEPLAYER_AUD_DECODER_PATH_LEN (256)
#define HI_LITEPLAYER_DEMUX_PATH_LEN (256)

/** Error information of the player */
typedef enum hiLITEPLAYER_ERROR_E
{
    HI_LITEPLAYER_ERROR_VID_PLAY_FAIL = 0x0,         /**< The video fails to be played. */
    HI_LITEPLAYER_ERROR_AUD_PLAY_FAIL,         /**< The audio fails to be played. */
    HI_LITEPLAYER_ERROR_DEMUX_FAIL,             /**< The file fails to be played. */
    HI_LITEPLAYER_ERROR_TIMEOUT,               /**< Operation timeout. For example, reading data timeout. */
    HI_LITEPLAYER_ERROR_NOT_SUPPORT,           /**< The file format is not supportted. */
    HI_LITEPLAYER_ERROR_UNKNOW,                /**< Unknown error. */
    HI_LITEPLAYER_ERROR_ILLEGAL_STATEACTION,     /**< illegal action at cur state. */
    HI_LITEPLAYER_ERROR_BUTT,
} HI_LITEPLAYER_ERROR_E;

/** Player status */
typedef enum hiLITEPLAYER_STATE_E
{
    HI_LITEPLAYER_STATE_IDLE = 0,        /**< The player state before init . */
    HI_LITEPLAYER_STATE_INIT,    /**< The player is in the initial state. It changes to the initial state after being created. */
    HI_LITEPLAYER_STATE_PREPARED,  /**< The player is in the prepared state. */
    HI_LITEPLAYER_STATE_PLAY,       /**< The player is in the playing state. */
    HI_LITEPLAYER_STATE_TPLAY,       /**< The player is in the trick playing state. */
    HI_LITEPLAYER_STATE_PAUSE,      /**< The player is in the pause state. */
    HI_LITEPLAYER_STATE_ERR,      /**< The player is in the err state. */
    HI_LITEPLAYER_STATE_BUTT
} HI_LITEPLAYER_STATE_E;

typedef enum hiLITEPLAYER_EVENT_E
{
    HI_LITEPLAYER_EVENT_STATE_CHANGED = 0x0,   /**< the player status changed*/
    HI_LITEPLAYER_EVENT_EOF,          /**< the player is playing the end*/
    HI_LITEPLAYER_EVENT_SOF,          /**< the player backward tplay to the start of file*/
    HI_LITEPLAYER_EVENT_PROGRESS,      /**< current playing progress. it will be called every one second. the additional value that in the unit of ms is current playing time*/
    HI_LITEPLAYER_EVENT_SEEK_END,      /**< seek time jump, the additional value is the seek value*/
    HI_LITEPLAYER_EVENT_ERROR,             /**< play error*/
    HI_LITEPLAYER_EVENT_BUTT
} HI_LITEPLAYER_EVENT_E;

typedef enum hiLITEPLAYER_PLAY_SPEED_E
{
    HI_LITEPLAYER_PLAY_SPEED_BASE   = 100,
    HI_LITEPLAYER_PLAY_SPEED_2X_FAST   = 2 * HI_LITEPLAYER_PLAY_SPEED_BASE,    /**< 2 x speed fast */
    HI_LITEPLAYER_PLAY_SPEED_4X_FAST   = 4 * HI_LITEPLAYER_PLAY_SPEED_BASE,    /**< 4 x speed fast */
    HI_LITEPLAYER_PLAY_SPEED_8X_FAST   = 8 * HI_LITEPLAYER_PLAY_SPEED_BASE,    /**< 8 x speed fast */
    HI_LITEPLAYER_PLAY_SPEED_16X_FAST  = 16 * HI_LITEPLAYER_PLAY_SPEED_BASE,   /**< 16 x speed fast */
    HI_LITEPLAYER_PLAY_SPEED_32X_FAST  = 32 * HI_LITEPLAYER_PLAY_SPEED_BASE,   /**< 32 x speed fast */
    HI_LITEPLAYER_PLAY_SPEED_64X_FAST  = 64 * HI_LITEPLAYER_PLAY_SPEED_BASE,   /**< 64 x speed fast */
    HI_LITEPLAYER_PLAY_SPEED_128X_FAST  = 128 * HI_LITEPLAYER_PLAY_SPEED_BASE,   /**< 128 x speed fast */
    HI_LITEPLAYER_PLAY_SPEED_BUTT                                /**< Invalid speed value */
} HI_LITEPLAYER_PLAY_SPEED_E;


typedef enum hiLITEPLAYER_TPLAY_DIRECTION_E
{
    HI_LITEPLAYER_TPLAY_FORWARD,
    HI_LITEPLAYER_TPLAY_BACKWARD,
    HI_LITEPLAYER_TPLAY_BUTT
} HI_LITEPLAYER_TPLAY_DIRECTION_E;


typedef enum hiLITEPLAYER_DATA_PLAY_TYPE_E
{
    HI_LITEPLAYER_DATA_TYPE_JPEG,
    HI_LITEPLAYER_DATA_TYPE_BUTT
} HI_LITEPLAYER_DATA_PLAY_TYPE_E;

typedef enum hiLITEPLAYER_VOUT_TYPE_E
{
    HI_LITEPLAYER_VOUT_TYPE_VPSS,
    HI_LITEPLAYER_VOUT_TYPE_VO,
    HI_LITEPLAYER_VOUT_TYPE_BUTT
} HI_LITEPLAYER_VOUT_TYPE_E;

typedef struct hiLITEPLAYER_TPLAY_ATTR_S
{
    HI_LITEPLAYER_TPLAY_DIRECTION_E enDirection;
    HI_LITEPLAYER_PLAY_SPEED_E enSpeed;
} HI_LITEPLAYER_TPLAY_ATTR_S;


typedef struct hiLITEPLAYER_ATTR_S
{
    HI_S32 s32VidStreamId;   /**< Video stream ID.  */
    HI_S32 s32AudStreamId;   /**< Audio stream ID. */
} HI_LITEPLAYER_ATTR_S;

typedef struct hiLITEPLAYER_PARAM_S
{
    HI_U32 u32PlayPosNotifyIntervalMs;   /**< Current playing Time notify interval in Ms. */
    HI_U32 u32VideoEsBufSize; /**< video es stream cache buf size.[1k,200M] unit:byte;if 0,use the inner default value */
    HI_U32 u32AudioEsBufSize;/**< audio es stream cache buf size.[1k,100M] unit:byte;if 0,use the inner default value  */
    HI_BOOL bPauseMode;/**< pause after first frame render mode flag,HI_FALSE:do not pause, HI_TRUE:pause after first frame render. */
} HI_LITEPLAYER_PARAM_S;

typedef struct hiLITEPLAYER_EXTRA_FILESURFIX_S
{
    HI_U32 u32SurfixCnt;   /**< file surfixes count. */
    HI_CHAR aszFileSurfix[HI_LITEPLAYER_MAX_FILE_SURFIX_CNT][HI_LITEPLAYER_MAX_FILE_SURFIX_LEN];   /**< file surfixes count. */
} HI_LITEPLAYER_EXTRA_FILESURFIX_S;

typedef struct hiLITEPLAYER_VOUT_OPT_S
{
    HI_LITEPLAYER_VOUT_TYPE_E enVoutType;
    HI_HANDLE hModHdl;
    HI_HANDLE hChnHdl;
} HI_LITEPLAYER_VOUT_OPT_S;

typedef struct hiLITEPLAYER_AOUT_OPT_S
{
    HI_HANDLE hAudDevHdl;//device id
    HI_HANDLE hAudTrackHdl;//chn id
} HI_LITEPLAYER_AOUT_OPT_S;


typedef HI_VOID (*HI_LITEPLAYER_EVENT_FN)(HI_MW_PTR pPlayer, HI_LITEPLAYER_EVENT_E enEvent, HI_VOID* pData);

typedef  HI_S32 (*HI_LITEPLAYER_ON_VB_POOL_GET_FN)(HI_VOID* pPlayer, HI_U32 u32FrameBufSize, HI_U32 u32FrameBufCnt, HI_U32* pVbPoolId, HI_VOID* pPriv);

typedef  HI_S32 (*HI_LITEPLAYER_ON_VB_POOL_PUT_FN)(HI_VOID* pPlayer, HI_U32 u32VbPoolId, HI_VOID* pPriv);

/**
*   @brief init the player
*   @param[in] N/A
*   @retval  0 success,others failed
*/
HI_S32 HI_LITEPLAYER_Init(HI_VOID);
/**
*   @brief deinit of the player
*   @param[in] : N/A
*   @retval  0 success,others failed
*/
HI_S32 HI_LITEPLAYER_DeInit(HI_VOID);

/**
*   @brief  register other demuxer,with supported extra file surfixes
*   @param[in] dllName : HI_CHAR: demuxer lib file
*   @param[in] pstExtraFileSurfix : HI_LITEPLAYER_EXTRA_FILESURFIX_S: supported extra file surfixes
*   @retval  0 success,others failed
*/
HI_S32 HI_LITEPLAYER_RegisterDemuxer(const HI_CHAR* dllName, const HI_LITEPLAYER_EXTRA_FILESURFIX_S* pstExtraFileSurfix);

/**
*   @brief unregister other demuxer
*   @param[in] dllName : HI_CHAR: demuxer lib file
*   @retval  0 success,others failed
*/
HI_S32 HI_LITEPLAYER_UnRegisterDemuxer(const HI_CHAR* pszDllName);

/**
*   @brief  register other audio Decoder
*   @param[in] dllName : HI_CHAR: audio lib file
*   @retval  0 success,others failed
*/
HI_S32 HI_LITEPLAYER_RegisterAudioDecoder(const HI_CHAR* pszAudioDecLib);

/**
*   @brief unregister other audio Decoder
*   @param[in] dllName : HI_CHAR: audio Decoder lib file
*   @retval  0 success,others failed
*/
HI_S32 HI_LITEPLAYER_UnRegisterAudioDecoder(const HI_CHAR* pszAudioDecLib);

/**
*   @brief create the player
*   @param[out] ppPlayer : HI_MW_PTR*: handle of the player
*   @param[in] pstPlayParam : HI_LITEPLAYER_PARAM_S*: input player params
*   @retval  0 success,others failed
*/
HI_S32 HI_LITEPLAYER_Create(HI_MW_PTR* ppPlayer, const HI_LITEPLAYER_PARAM_S* pstCreateParam);

/**
*   @brief  set the vo handle to the player
*   @param[in] pPlayer : HI_MW_PTR: handle of the player
*   @param[in] pstVoutOpt : HI_LITEPLAYER_VOUT_OPT_S*: indicate vout attr
*   @retval  0 success,others failed
*/
HI_S32 HI_LITEPLAYER_SetVOHandle(HI_MW_PTR pPlayer, HI_LITEPLAYER_VOUT_OPT_S* pstVoutOpt);

/**
*   @brief  set the ao handle to the  player
*   @param[in] pPlayer : HI_MW_PTR: handle of the player
*   @param[in] pstAoutOpt : HI_LITEPLAYER_AOUT_OPT_S: attr of audio out device
*   @retval  0 success,others failed
*/
HI_S32 HI_LITEPLAYER_SetAOHandle(HI_MW_PTR pPlayer, HI_LITEPLAYER_AOUT_OPT_S* pstAoutOpt);

/**
*   @brief  destroy the player
*   @param[in] pPlayer : HI_MW_PTR: handle of the player
*   @retval  0 success,others failed
*/
HI_S32 HI_LITEPLAYER_Destroy(HI_MW_PTR pPlayer);

/**
*   @brief    set the file for playing
*   @param[in] pPlayer : HI_MW_PTR: handle of the player
*   @param[in] filePath : HI_CHAR: media file path
*   @retval  0 success,others failed
*/
HI_S32 HI_LITEPLAYER_SetDataSource(HI_MW_PTR pPlayer, const HI_CHAR* pszfilePath);

/**
*   @brief prepare for the playing
*   @param[in] pPlayer : HI_MW_PTR: handle of the player
*   @retval  0 success,others failed
*/
HI_S32 HI_LITEPLAYER_Prepare(HI_MW_PTR pPlayer);

/**
*   @brief  do play of the stream
*   @param[in] pPlayer : HI_MW_PTR: handle of the player
*   @retval  0 success,others failed
*/
HI_S32 HI_LITEPLAYER_Play(HI_MW_PTR pPlayer);

/**
*   @brief stop the stream playing, and release the resource
*   @param[in] pPlayer : HI_MW_PTR: handle of the player
*   @retval  0 success,others failed
*/
HI_S32 HI_LITEPLAYER_Stop(HI_MW_PTR pPlayer);

/**
*   @brief pause the stream playing
*   @param[in] hPlayer : HI_MW_PTR: handle of the player
*   @retval  0 success,others failed
*/
HI_S32 HI_LITEPLAYER_Pause(HI_MW_PTR pPlayer);

/**
*   @brief seek by the time
*   @param[in] pPlayer : HI_MW_PTR: handle of the player
*   @param[in] s64TimeInMs : HI_S64: seek time
*   @retval  0 success,others failed
*/
HI_S32 HI_LITEPLAYER_Seek(HI_MW_PTR pPlayer, HI_S64 s64TimeInMs);

/**
*   @brief register call back fun
*   @param[in] pPlayer : HI_MW_PTR: handle of the player
*   @param[in] pfnCallback : HI_LITEPLAYER_EVENT_FN: call back fun
*   @retval  0 success,others failed
*/
HI_S32 HI_LITEPLAYER_RegCallback(HI_MW_PTR pPlayer, HI_LITEPLAYER_EVENT_FN pfnCallback);

/**
*   @brief get the info about the media file
*   @param[in] pPlayer : HI_MW_PTR: handle of the player
*   @param[out] pstFormatInfo : HI_FORMAT_FILE_INFO_S: media info abort the media file
*   @retval  0 success,others failed
*/
HI_S32 HI_LITEPLAYER_GetFileInfo(HI_MW_PTR pPlayer, HI_FORMAT_FILE_INFO_S* pstFormatInfo);

/**
*   @brief set the attribute about the media file
*   @param[in] pPlayer : HI_MW_PTR: handle of the player
*   @param[out] pstMediaAttr : HI_LITEPLAYER_ATTR_S: attribute of the media file
*   @retval  0 success,others failed
*/
HI_S32 HI_LITEPLAYER_SetMedia(HI_MW_PTR pPlayer, HI_LITEPLAYER_ATTR_S* pstMediaAttr);


/**
*   @brief get the  current play status
*   @param[in] pPlayer : HI_MW_PTR: handle of the player
*   @param[out] penState : HI_LITEPLAYER_STATE_E*: play state
*   @retval  0 success,others failed
*/
HI_S32 HI_LITEPLAYER_GetPlayStatus(HI_MW_PTR pPlayer, HI_LITEPLAYER_STATE_E* penState);


/**
*   @brief trick play current video stream
*   @param[in] pPlayer : HI_MW_PTR: handle of the player
*   @param[out] pstTrickPlayAttr : HI_LITEPLAYER_TPLAY_ATTR_S: trick play attribute
*   @retval  0 success,others failed
*/
HI_S32 HI_LITEPLAYER_TPlay(HI_MW_PTR pPlayer, HI_LITEPLAYER_TPLAY_ATTR_S* pstTrickPlayAttr);


/** @}*/  /** <!-- ==== PLAYER End ====*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif
