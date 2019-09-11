/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2009-2019. All rights reserved.
 * Description: Parse moudle API
 * Author: NULL
 * Create: 2009-2-9
 */

#ifndef __HI_PARSE_H__
#define __HI_PARSE_H__

/* * add include here */
#include "hi_type.h"
#include "hi_go.h"
#include "hi_gv.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LOADALLVIEW (-1)
/* ****************************** API declaration **************************** */
/* * addtogroup      Parser */
/* * 【Parser module】 ; 【解析模块】  */
/* brief Parser init.CNcomment:模块初始化 */
HI_S32 HI_GV_PARSER_Init(HI_VOID);

/* brief Uninit parser.CNcomment:Parser模块去初始化 */
HI_VOID HI_GV_PARSER_Deinit(HI_VOID);

/**
* brief  Load HIGVBin file and parse the basic information of
* higvbin.CNcomment:HIGVBin文件加载，同时解析出HIGVBin文件基本信息 \attention \n N/A.CNcomment:无
* param[in] pFile  File name.CNcomment:HIGVBin文件名
* retval ::HI_SUCCESS
* retval ::HI_ERR_PARSER_NOINIT  it is not inited.CNcomment:未初始化
* retval ::HI_ERR_PARSER_FILELOADED  File error.CNcomment:文件错误
* retval ::HI_ERR_COMM_FILEOP      File opreration is error.CNcomment:文件操作错误
* retval ::HI_ERR_COMM_NORES  No memory. CNcomment: 内存不足
* retval ::HI_ERR_PARSER_VERIFY   CRC ERROR.CNcomment:CRC错误
*/
HI_S32 HI_GV_PARSER_LoadFile(const HI_CHAR *pFile);

/* brief Unload higvbin file, release resouce.CNcomment:卸载HIGVBin文件，释放资源 */
HI_S32 HI_GV_PARSER_UnLoadFile(HI_VOID);

/*
* brief Release the resouce when load higvbin file, This api will be called at the end of
* load.CNcomment:释放文件加载过程中申请的资源，在所有视图加载完成后调用 \attention \n N/A.CNcomment:无
* param N/A.CNcomment:无
*/
HI_S32 HI_GV_PARSER_ReleaseLoadRes(HI_VOID);

/*
* brief Get view number.CNcomment:获取视图总数
* param[out] pViewNum View number.CNcomment:视图总数
*/
HI_S32 HI_GV_PARSER_GetViewNum(HI_U32 *pViewNum);

/* brief Load the view by name of view.CNcomment:按视图名加载视图，加载时解析并创建视图 */
HI_S32 HI_GV_PARSER_LoadViewByName(const HI_CHAR *pViewName);

/**
* brief Load the view by ID of view.CNcomment:按视图ID加载视图，加载时解析并创建视图
* attention \n
* if(ViewId = LOADALLVIEW),load all view.CNcomment:如果ViewId为 LOADALLVIEW,则加载所有视图
* param[in] ViewId    View id.CNcomment:视图ID
* retval ::HI_SUCCESS
* retval ::HI_ERR_PARSER_NOINIT
* retval ::HI_ERR_COMM_LOST   The view is not exit.CNcomment:指定的视图不存在
* retval ::HI_ERR_PARSER_DATALEN
* retval ::HI_ERR_PARSER_DATAERR
* see:HI_GV_PARSER_LoadViewById
*/
HI_S32 HI_GV_PARSER_LoadViewById(HIGV_HANDLE ViewId);

/* brief Unload the view by ID of view.CNcomment:按视图ID卸载视图，销毁视图控件 */
HI_S32 HI_GV_PARSER_UnloadViewByID(HIGV_HANDLE ViewId);

/*
* brief Load the view by index.CNcomment:按视图索引加载视图，加载时解析并创建视图，这个函数线程安全
* param[in] ViewIndex   View index.CNcomment:视图索引
* retval ::HI_SUCCESS
* retval ::HI_ERR_PARSER_NOINIT
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_PARSER_DATALEN
* retval ::HI_ERR_PARSER_DATAERR
*/
HI_S32 HI_GV_PARSER_LoadViewByIndex(HI_U32 ViewIndex);

/*
* brief Delete the view exclude view.CNcomment:删除除指定的视图外其他所有视图
* param[in] ExcludeViewId  Exclude view that is not deleted.CNcomment:不需要删除视图ID数组
* param[in] ExcludeViewNum  The number of view that not need deleted.CNcomment:不需要删除视图数目
* retval ::HI_SUCCESS
* retval ::HI_ERR_PARSER_NOINIT
* retval ::HI_ERR_COMM_INVAL
* retval ::HI_FAILURE
*/
HI_S32 HI_GV_PARSER_UnLoadViewExclude(HIGV_HANDLE *ExcludeViewId, HI_U32 ExcludeViewNum);

/*
* brief Get windows handle by view id.CNcomment:通过视图ID 获取视图内所有窗口句柄
* param[out] ppWinArray  The window handle array .CNcomment:视图内窗口handle 数组
* param[out] pWinsNum  The number of view that not need deleted.CNcomment:视图内窗口数目
* retval ::HI_SUCCESS
* retval ::HI_FAILURE
*/
HI_S32 HI_GV_PARSER_ViewGetWinsHandle(HIGV_HANDLE ViewId, HIGV_HANDLE **ppWinsArray, HI_U32 *pWinsNum);

/*
* brief Get the version number.CNcomment:获取版本号
* param[out] pMainVersion  Main verison.CNcomment:主版本号
* param[out] pSubVersion   Sub version.CNcomment:子版本号
* retval ::HI_SUCCESS
* retval ::HI_ERR_PARSER_NOINIT
*/
HI_S32 HI_GV_PARSER_GetVersion(HI_U32 *pMainVersion, HI_U32 *pSubVersion);

/*
* brief Set the evnent function at the STC mode.CNcomment:设置STC模式下事件函数列表
* param[in] pEventProc Event functon.CNcomment: 事件函数数组指针
* param[in] Number      The number of event fucntion.CNcomment:事件函数数组中函数个数
* retval HI_SUCCESS
* retval HI_ERR_PARSER_NOINIT
* retval HI_ERR_COMM_INVAL
*/
HI_S32 HI_GV_PARSER_SetWidgetEventFunc(HIGV_MSG_PROC *pEventProc, HI_U32 Number);

/* * ==== API declaration end ==== */
#ifdef __cplusplus
}
#endif
#endif /* __HI_PARSE_H__ */
