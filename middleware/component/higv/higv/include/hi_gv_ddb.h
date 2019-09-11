/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2009-2019. All rights reserved.
 * Description: hi_gv_ddb.h
 * Author: NULL
 * Create: 2009-2-9
 */

#ifndef __HI_GV_DDB_H__
#define __HI_GV_DDB_H__

/* add include here */
#include "hi_type.h"
#include "hi_gv_adm.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ************************** Structure Definition *************************** */
/* * addtogroup      DDB  */
/* * 【DDB】 ; CNcomment: 【数据缓存】 */

/* *Insert position ; CNcomment:插入类型 */
typedef enum {
    DDB_INSERT_PRE = 0,
    /*  Insert before,   CNcomment: 插入该行之前 */
    DDB_INSERT_NEXT,
    /*  Insert after,   CNcomment: 插入该行之后 */
    DDB_INSERT_BUTT
} HIGV_DDBINSERT_E;

/* *  ==== Structure Definition end ==== */
/* ****************************** API declaration **************************** */
/* * addtogroup      DDB  */
/* * 【DDB】 ;  CNcomment:【数据缓存】 */
/**
* brief Create a DDB.CNcomment:创建一个数据缓存实例
* param[in] FieldCount Field count.CNcomment:字段个数
* param[in] pFieldAttr Field attribute.CNcomment:字段属性
* param[out] phDDB    DDB handle. CNcomment:数据缓存句柄
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_INVAL
*/
HI_S32 HI_GV_DDB_Create(HI_U32 FieldCount, const HIGV_FIELDATTR_S *pFieldAttr, HIGV_HANDLE *phDDB);

/**
* brief Destroy a DDB.CNcomment:删除一个数据缓存实例
* param[in] hDDB DDB handle.CNcomment:数据缓存句柄
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
*/
HI_S32 HI_GV_DDB_Destroy(HIGV_HANDLE hDDB);

/**
* brief Insert a recoder to the databuffer.CNcomment:向数据缓存中插入一个记录
* param[in] hDDB DDB handle.CNcomment:数据缓存句柄
* param[in] Row Row position.CNcomment:标识插入数据的位置
* param[in] pData The pointer of recoder.CNcomment:记录的数据，注意pData->size不能小于各列数据长度之和
* param[in] eInsert The postion of insert.CNcomment:标识插入数据的位置
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_DDB_NULLPTR
* retval ::HI_ERR_DDB_INVAIDPARA
*/
HI_S32 HI_GV_DDB_Insert(HIGV_HANDLE hDDB, HI_U32 Row, const HIGV_DBROW_S *pData, HIGV_DDBINSERT_E eInsert);

/**
* brief Modify a recoder of data buffer.CNcomment:修改数据缓存中的一个记录
* param[in] hDDB DDB handle.CNcomment:数据缓存句柄
* param[in] Row Row index.CNcomment:要修改的行号
* param[in] pdata Recoder data.CNcomment:记录的数据，注意pData->size不能小于各列数据长度之和
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_DDB_NULLPTR
* retval ::HI_ERR_DDB_OUTOFRANGE
*/
HI_S32 HI_GV_DDB_Modify(HIGV_HANDLE hDDB, HI_U32 Row, const HIGV_DBROW_S *pData);

/**
* brief Add a recoder to the tail of data buffer.CNcomment:向数据缓存尾部添加一个记录
* param[in] DDB handle.CNcomment:hDDB 数据缓存句柄
* param[in] pData Recoder data.CNcomment:记录的数据，注意pData->size不能小于各列数据长度之和
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_DDB_NULLPTR
* retval ::HI_ERR_DDB_BUFFSMALL
*/
HI_S32 HI_GV_DDB_Append(HIGV_HANDLE hDDB, const HIGV_DBROW_S *pData);

/**
* brief Clear the recoder in the data buffer.CNcomment:清除数据缓存中的记录
* param[in] hDDB DDB handle.
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
*/
HI_S32 HI_GV_DDB_Clear(HIGV_HANDLE hDDB);

/**
* brief Delete a recoder by key.CNcomment:根据key值删除一条记录
* param[in] hDDB DDB handle.CNcomment:数据缓存句柄
* param[in] Row Row index.CNcomment:要操作记录的行号
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_DDB_OUTOFRANGE
*/
HI_S32 HI_GV_DDB_Del(HIGV_HANDLE hDDB, HI_U32 Row);

/**
* brief Get the recoder by row number.CNcomment:根据行号获取一条记录
* param[in] hDDB DDB handle.CNcomment:数据缓存句柄
* param[in] Row The row of recoder.CNcomment:要操作记录的位置
* param[out] pData Recoder data.CNcomment:记录的数据
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_DDB_NULLPTR
* retval ::HI_ERR_DDB_OUTOFRANGE
*/
HI_S32 HI_GV_DDB_GetRow(HIGV_HANDLE hDDB, HI_U32 Row, HIGV_DBROW_S *pData);

/**
* brief Get n rows form the DDB.CNcomment:从指定行出获取n行的数据
* param[in] hDDB DDB handle.数据缓存句柄
* param[in] Row  Row index.CNcomment:要获取数据的起始行号
* param[in] Num  Row number.CNcomment:要获取的总行数
* param[out] pData    Data cache.CNcomment:数据缓存
* param[out] pRowNum  Row number.CNcomment:实际获取的行数
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_DDB_NULLPTR
* retval ::HI_ERR_DDB_OUTOFRANGE
* retval ::HI_ERR_DDB_INVAIDPARA
*/
HI_S32 HI_GV_DDB_GetRows(HIGV_HANDLE hDDB, HI_U32 Row, HI_U32 Num, HI_VOID *pData, HI_U32 *pRowNum);

/**
* brief Get the column index of data.CNcomment:按列号获取数据
* param[in]  hDDB DDB handle.CNcomment:数据缓存句柄
* param[in]  Row Row index.CNcomment:行号
* param[in]  Col Column index.CNcomment:列号
* param[out] pFieldData CNcomment: 字段buffer
* param[in]  Len Buffer size.CNcomment:字段buffer大小
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_DDB_NULLPTR
* retval ::HI_ERR_DDB_OUTOFRANGE
* retval ::HI_ERR_DDB_BUFFSMALL
*/
HI_S32 HI_GV_DDB_GetCellData(HIGV_HANDLE hDDB, HI_U32 Row, HI_U32 Col, HI_VOID *pFieldData, HI_U32 Len);

/**
* brief Set the column index of data.CNcomment:按列号设置数据
* param[in]  hDDB DDB handle.CNcomment:数据缓存句柄
* param[in]  Row Row index.CNcomment:行号
* param[in]  Col Column index.CNcomment:列号
* param[in]  pFieldData CNcomment: 字段buffer
* param[in]  Len Buffer size.CNcomment:字段buffer大小
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_DDB_NULLPTR
* retval ::HI_ERR_DDB_OUTOFRANGE
* retval ::HI_ERR_DDB_OUTOFRANGE
*/
HI_S32 HI_GV_DDB_SetCellData(HIGV_HANDLE hDDB, HI_U32 Row, HI_U32 Col, HI_VOID *pFieldData, HI_U32 Len);

/**
* brief Get the total row number.CNcomment:获取总行数
* param[in] hDDB DDB handle.CNcomment:默认数据缓存句柄
* param[out] pRowCount Row count.CNcomment:总行数
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_DDB_NULLPTR
*/
HI_S32 HI_GV_DDB_GetRowCount(HIGV_HANDLE hDDB, HI_U32 *pRowCount);

/**
* brief Register the function when the data change.CNcomment:注册数据变化通知函数
* param[in] hDDB DDB handle.CNcomment:默认数据缓存句柄
* param[in] hADM The object which notify.CNcomment:需要通知的对象
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
*/
HI_S32 HI_GV_DDB_RegisterDataChange(HIGV_HANDLE hDDB, HIGV_HANDLE hADM);

/**
* brief Unregister the function when data change.CNcomment:取消数据变化通知
* param[in] hDDB DDB handle.CNcomment:默认数据缓存句柄
* param[in] hADM The object which notify.CNcomment:需要通知的对象
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
*/
HI_S32 HI_GV_DDB_UnRegisterDataChange(HIGV_HANDLE hDDB, HIGV_HANDLE hADM);

/**
* brief Register callback when data change.CNcomment:数据变化通知使能开关
* param[in] hDDB  DDB handle. CNcomment: 数据缓存句柄
* param[in] bEnable Enable flag. CNcomment:数据变化通知使能开关，HI_TRUE表示使能，HI_FALSE表示去使能
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
*/
HI_S32 HI_GV_DDB_EnableDataChange(HIGV_HANDLE hDDB, HI_BOOL bEnable);

/* *==== API declaration end ==== */
#ifdef __cplusplus
}
#endif
#endif /* __HI_GV_DDB_H__ */
