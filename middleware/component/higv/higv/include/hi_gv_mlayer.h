/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2009-2019. All rights reserved.
 * Description: Multi-layer module API
 * Author: NULL
 * Create: 2009-2-9
 */

#ifndef __HI_GV_MLAYER_H__
#define __HI_GV_MLAYER_H__

/* add include here */
#include "hi_go.h"

#ifdef __cplusplus
extern "C" {
#endif
/* ****************************** API declaration **************************** */
/* * addtogroup      MultiLayer   */
/* *  【multi-layer module】 CNcomment:  【多图层模块】 */
/* * The event of window move or size change */ /* * CNcomment: 窗口移动或尺寸修改后产生的窗口事件ID */
#define HIGV_MSG_WINDOW_MOVE 0x502

typedef enum {
    HIGV_ROTATE_NONE = 0,
    HIGV_ROTATE_90,
    /* <Rotate 90 degrees clockwise  , CNcomment:  顺时针旋转90度 */
    HIGV_ROTATE_180,
    /* <Rotate 180 degrees clockwise , CNcomment: 顺时针旋转１８０度 */
    HIGV_ROTATE_270,
    /* <Rotate 270 degrees clockwise , CNcomment: 顺时针旋转２７０度 */
    HIGV_ROTATE_BUTT
} HIGV_ROTATE_E;

/**
* brief Create layer.CNcomment: 创建图层
* param[in]  pLayerInfo Layer infomation.CNcomment: 图层信息
* param[out] pLayer The pointer of layer handle.CNcomment: 存放图层句柄的指针
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
*/
HI_S32 HI_GV_Layer_Create(const HIGO_LAYER_INFO_S *pLayerInfo, HIGV_HANDLE *pLayer);

/**
* brief Create Layer, the handle is used by xml.CNcomment: 创建图层，用于在XML中已指定句柄使用
* param[in]  pLayerInfo Layer infomation.CNcomment: 图层信息
* param[in]  hLayer Layer handle.CNcomment: 图层句柄
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
*/
HI_S32 HI_GV_Layer_CreateEx(const HIGO_LAYER_INFO_S *pLayerInfo, HIGV_HANDLE hLayer);

/**
* brief Destroy layer.CNcomment: 销毁图层
* param[in] hLayer Layer handle.CNcomment: 图层句柄
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
*/
HI_S32 HI_GV_Layer_Destroy(HIGV_HANDLE hLayer);

/**
* brief show/hide the layer.CNcomment: 显示/隐藏图层
* param[in] bVisible The flag indicat show or hide.CNcomment: HI_TRUE表示显示，HI_FALSE表示隐藏
* retval ::HI_SUCCESS
*/
HI_S32 HI_GV_Layer_Show(HIGV_HANDLE hLayer, HI_BOOL bVisible);

/**
* brief Set default layer.CNcomment: 设置缺省图层，设置缺省图层后，如果创建窗口不制定图层，则
*        默认为此图层
* param[in] hLayer Layer handle.CNcomment:  图层句柄
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
*/
HI_S32 HI_GV_Layer_SetDefault(HIGV_HANDLE hLayer);

/**
* brief Get default layer.CNcomment: 获取缺省图层
* param[out] pDefaultLayer The pointer of default layer.CNcomment:  获取默认图层指针
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_EMPTY
*/
HI_S32 HI_GV_Layer_GetDefault(HIGV_HANDLE *pDefaultLayer);

/**
* brief Get the active window handle.CNcomment: 获取图层活动窗口句柄
* param[in] hLayer Layer handle.CNcomment: 图层句柄
* param[out] pWindow Active window handle.CNcomment: 活动窗口句柄指针
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_EMPTY
* retval ::HI_ERR_COMM_LOST
*/
HI_S32 HI_GV_Layer_GetActiveWindow(HIGV_HANDLE hLayer, HIGV_HANDLE *pWindow);

/**
* brief Get the higo layer by higv layer handle.CNcomment: 通过HIGV图层句柄获取HIGO图层图层句柄
* param[in] hLayer Layer handle.CNcomment: HIGV图层句柄
* param[out] pHigoLayer  higo layer handle.CNcomment: HIGO图层句柄指针
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_EMPTY
*/
HI_S32 HI_GV_Layer_GetHigoLayer(HIGV_HANDLE hLayer, HIGV_HANDLE *pHigoLayer);

/**
* brief Set the layer the share source.CNcomment: 设置两个层同源输出
* param[in] hOwnerLayer   Layer handle.CNcomment: 已经拥有输出内容的layer
* param[in] hClientLayer  The share source between the layer.CNcomment: 需要与hOwnerLayer共享源的layer
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_UNSUPPORT
*/
HI_S32 HI_GV_Layer_ShareSource(HIGV_HANDLE hOwnerLayer, HIGV_HANDLE hClientLayer);

/**
* brief Get the active layer handle.CNcomment: 获取活动图层句柄
* param[out] pLayer   Layer handle.CNcomment: 活动图层句柄
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_EMPTY
*/
HI_S32 HI_GV_Layer_GetActiveLayer(HIGV_HANDLE *pLayer);

/**
* brief Set active layer.CNcomment: 设置活动图层句柄
* param[in] hLayer   Layer handle.CNcomment: 活动图层句柄
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
*/
HI_S32 HI_GV_Layer_SetActiveLayer(HIGV_HANDLE hLayer);

/**
* brief Move window zorder to top.CNcomment: 设置该图层的z序为最顶层
* attention:
* The layer has z order, The z order of is more bigger, the layer is displayed close to user, this interface is to set the
* layer show on top of screen.
* param[in] hLayer   Layer handle.CNcomment: 图层句柄
* retval ::HI_SUCCESS
* retval ::HI_FAILURE
* see:HI_GV_Layer_MoveToBottom
*/
HI_S32 HI_GV_Layer_MoveToTop(HIGV_HANDLE hLayer);

/**
* brief move window zorder to bottom.CNcomment: 设置该图层的z序为最底层
* param[in] hLayer   Layer handle.CNcomment: 图层句柄
* retval ::HI_SUCCESS
* retval ::HI_FAILURE
* see : HI_GV_Layer_MoveToTop
*/
HI_S32 HI_GV_Layer_MoveToBottom(HIGV_HANDLE hLayer);

/**
* brief Move up window zorder.CNcomment: 提高窗口的z序等级
* attention
* The layer has z order, The z order of is more bigger, the layer is displayed close to user, this interface is to set the
* layer show on top of screen.
* param[in] hLayer   Layer handle.CNcomment: 图层句柄
* retval ::HI_SUCCESS
* retval ::HI_FAILURE
* see : HI_GV_Layer_MoveDown
*/
HI_S32 HI_GV_Layer_MoveUp(HIGV_HANDLE hLayer);

/**
* brief Move down window zorder.CNcomment: 降低窗口的z序等级
* param[in] hLayer   Layer handle.CNcomment: 图层句柄
* retval ::HI_SUCCESS
* retval ::HI_FAILURE
* see : HI_GV_Layer_MoveUp
*/
HI_S32 HI_GV_Layer_MoveDown(HIGV_HANDLE hLayer);

/**
* brief set rotate mode.CNcomment: 设置图形旋转度数
* attention depend on higo support\n
* attention only support HIGO_PF_4444 & HIGO_PF_1555 Pixel Format \n
* param[in] hLayer   Layer handle.CNcomment: 图层句柄
* param[in] rotate   rotate.CNcomment: 旋转度数
* retval ::HI_SUCCESS
* retval ::HI_FAILURE
* see : HI_GV_Layer_SetRotateMode
*/
HI_S32 HI_GV_Layer_SetRotateMode(HIGV_HANDLE hLayer, HIGV_ROTATE_E rotate);

/*  ==== API declaration end ==== */
#ifdef __cplusplus
}
#endif
#endif /* __HI_GV_MLAYER_H__ */
