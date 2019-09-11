/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2009-2019. All rights reserved.
 * Description: ImageEx wdiget
 * Author: NULL
 * Create: 2009-2-9
 */

#ifndef __HI_GV_IMAGEEX_H__
#define __HI_GV_IMAGEEX_H__

/* add include here */
#include "hi_type.h"
#include "hi_gv_conf.h"

#ifdef HIGV_USE_WIDGET_IMAGEEX
#ifdef __cplusplus
extern "C" {
#endif

/* ************************** Structure Definition *************************** */
/* * addtogroup      ImageEx     */
/* * ¡¾ImageEx widget¡, CN:¡¾ImageEx¿Ø¼þ¡¿ */
/* * Alignment of image , CNcomment:  Í¼Æ¬µÄ¶ÔÆë·½Ê½ */
typedef enum {
    HIGV_IMGPOS_CENTER,
    /* Center ; CNcomment: ¾ÓÖÐ */
    HIGV_IMGPOS_STRETCH,
    /* Stretch ; CNcomment: À­Éì */
    HIGV_IMGPOS_AUTOSTRETCH,
    /* Autostrech ; CNcomment: ×Ô¶¯À­Éì */
    HIGV_IMGPOS_BUTT
} HIGV_IMGPOS_E;

/* * Structure Definition end */
/* ****************************** API declaration **************************** */
/* * addtogroup      ImageEx     */
/* * ¡¾ImageEx widget¡¿ CNcomment: ¡¾ImageEx¿Ø¼þ¡¿ */
/**
* brief Set the content.CNcomment:ÉèÖÃÍ¼Æ¬ÄÚÈÝ
* param[in] hImageEx Imagex handle. CNcomment:Í¼Æ¬¿Ø¼þ¾ä±ú
* param[in] pImgFile  The path file of image.CNcomment:Í¼Æ¬ÎÄ¼þÂ·¾¶
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_INVAL
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_EXIST Timer is exist.CNcomment:¶¨Ê±Æ÷×ÊÔ´ÒÑ¾­´æÔÚ
* retval ::HI_ERR_COMM_NORES
*/
HI_S32 HI_GV_ImageEx_SetImage(HIGV_HANDLE hImageEx, const HI_CHAR *pImgFile);

/**
* brief Set the count of playback cycles.CNcomment:ÉèÖÃ¶¯»­Í¼Æ¬²¥·Å´ÎÊý
* param[in] hImageEx Imagex handle. CNcomment:Í¼Æ¬¿Ø¼þ¾ä±ú
* param[in] RepeatCount The count of playback cycles .CNcomment:¶¯»­Í¼Æ¬²¥·Å´ÎÊý
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_INVAL
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_NORES
*/
HI_S32 HI_GV_ImageEx_SetRepeatCount(HIGV_HANDLE hImageEx, HI_S32 RepeatCount);

/**
* brief Set the position of picture to the widget.CNcomment:ÉèÖÃÍ¼Æ¬ÔÚ¿Ø¼þÖÐµÄÏÔÊ¾Î»ÖÃ
* param[in] hImageEx Imagex handle.CNcomment:Í¼Æ¬¿Ø¼þ¾ä±ú
* param[in] PosType  Position type of picture.CNcomment:Í¼Æ¬Î»ÖÃÀàÐÍ
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_INVAL
* retval ::HI_ERR_COMM_LOST
*/
HI_S32 HI_GV_ImageEx_SetPos(HIGV_HANDLE hImageEx, HIGV_IMGPOS_E PosType);

/**
* brief Set the interval of image display.CNcomment:ÉèÖÃÍ¼Æ¬ÏÔÊ¾µÄÊ±¼ä¼ä¸ô
* attention ::
* it is only valid to dynamic gif format.
* CNcomment:¶Ô¶¯Ì¬GIFÍ¼Æ¬ÓÐÐ§
* param[in] hImageEx Imagex handle.CNcomment:Í¼Æ¬¿Ø¼þ¾ä±ú
* param[in] Interval Interval of picture display.CNcomment:Í¼Æ¬ÏÔÊ¾µÄÊ±¼ä¼ä¸ô£¬ÒÔmsÎªµ¥Î»
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_INVAL
* retval ::HI_ERR_COMM_LOST
*/
HI_S32 HI_GV_ImageEx_SetInterval(HIGV_HANDLE hImageEx, HI_U32 Interval);

/* ==== API declaration end ==== */
#ifdef __cplusplus
}
#endif
#endif
#endif /* __HI_GV_IMAGEEX_H__ */
