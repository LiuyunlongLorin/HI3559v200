/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2009-2019. All rights reserved.
 * Description: ADM API
 * Author: NULL
 * Create: 2009-2-9
 */

#ifndef __HI_GV_ADM_H__
#define __HI_GV_ADM_H__

#include "hi_type.h"
#include "hi_gv_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ************************** Structure Definition *************************** */
/* * ADM  */
/* *  【abstract data model】; CNcomment: 【ADM模块】 */

/* *ADM and DDB support  MAX filed number ; CNconmment: 数据库和默认缓存支持的最大字段个数 */
#define HIGV_DB_MAX_FIELDNUM      (64)
#define HIGV_DB_MAX_FIELD_STR_LEN (4096)

/* * The type of data are supported.; CNcomment:支持的数据类型定义 */
typedef enum {
    HIGV_DT_S8 = 0, /* char */
    HIGV_DT_U8,     /* unsigned char */
    HIGV_DT_S16,    /* short */
    HIGV_DT_U16,    /* unsigned short */
    HIGV_DT_S32,    /* int */
    HIGV_DT_U32,    /* unsigned int */
    HIGV_DT_S64,    /* long long */
    HIGV_DT_U64,    /* unsigned long long */
    HIGV_DT_F32,    /* float */
    HIGV_DT_D64,    /* double */
    HIGV_DT_STRING, /* char * */
    HIGV_DT_HIMAGE, /* image handle */
    HIGV_DT_STRID,  /* multi-langugae  string ID */
    HIGV_DT_BUTT
} HIGV_DT_E;

/* * The type of data change. ; CNconmment:数据变化的类型 */
typedef enum {
    HIGV_DC_INSERT = 0,
    /* *< insert data ; CNcomment:插入 */
    HIGV_DC_MODIFY,
    /* *< modify data ; CNcomment:修改 */
    HIGV_DC_DELETE,
    /* *< delete data ; CNcomment:删除 */
    HIGV_DC_UPDATE,
    /* *< updata data,sealed operations ; CNcomment:数据变化，操作未知 */
    HIGV_DC_CLEAR,
    /* *< clear all the data ; CNcomment:数据全部销毁 */
    HIGV_DC_DESTROY,
    /* *< destroy the data source ; CNcomment:数据源销毁 */
    HIGV_DC_BUTT
} HIGV_DC_E;

/* * The data change info.  ; CNconmment:数据变化信息 */
typedef struct {
    HIGV_DC_E Action; /**< the type of data change */     /**< CNcomment:变化动作描述 */
    HI_U16 StartRow; /**< the start row of data change */ /**< CNcomment:变化的起始行 */
    HI_U16 Rows; /**< rows aggregate have data change */ /**< CNcomment:变化的总行数 */
} HIGV_DCINFO_S;

/* * Cell attribute.CNconmment:字段属性 */
typedef struct hiHIGV_CELLATTR_S {
    HIGV_DT_E eDataType; /* *< data type ; CNcomment:数据类型 */
    /* *< max field str len, value must less than ::HIGV_DB_MAX_FIELD_STR_LEN */
    /* *< CNcomment:最大长度，大小必须小于::HIGV_DB_MAX_FIELD_STR_LEN */
    HI_U32 MaxSize;
} HIGV_FIELDATTR_S;

/* * Row deferable.CNcomment: 行数据存储 */
typedef struct hiHIGV_DBROW_S {
    HI_U32 Size; /* data size  , CNcomment:数据长度 */
    HI_VOID *pData; /* data pointer , CNcomment:数据 */
} HIGV_DBROW_S;

typedef HI_S32 (*GetCountFunc)(HIGV_HANDLE DBSource, HI_U32 *RowCnt);
typedef HI_S32 (*GetRowValueFunc)(HIGV_HANDLE DBSource, HI_U32 Row, HI_U32 Num, HI_VOID *pData,
               HI_U32 *pRowNum);
typedef HI_S32 (*RegisterDataChangeFunc)(HIGV_HANDLE DBSource, HIGV_HANDLE hADM);
typedef HI_S32 (*UnregisterDataChangeFunc)(HIGV_HANDLE DBSource, HIGV_HANDLE hADM);

typedef struct hiADM_OPT_S {
    HI_U32 DBSource; /* *< data source ; CNcomment:数据源标识 */
    HI_U32 FieldCount; /* *< data source field count ; CNcomment:数据源的字段总数 */
    HIGV_FIELDATTR_S *pFieldAttr; /* *< field attribute ; CNcomment:数据源的每个字段属性 */
    HI_U32 BufferRows; /* *< rows at buffer ; CNcomment:希望的缓冲行数 */
    GetCountFunc GetCount; /* * get rows count function. ; CNcomment:获取总数接口 */
    GetRowValueFunc GetRowValue;
    /* * get rows value from  appoint rows function. ; CNcomment:从指定行处获取n行数据 */
    RegisterDataChangeFunc RegisterDataChange;
    /* * Register the function when data change ; CNcomment:注册数据变化通知接口 */
    UnregisterDataChangeFunc UnregisterDataChange;
    /* * Unregister the function when data change. ; CNcomment:取消数据变化通知接口 */
} HIGV_ADMOPT_S;

/* * ==== Structure Definition end ==== */
/* ****************************** API declaration **************************** */
/* * addtogroup      ADM  */
/* *【abstract data model】; CNcomment: 【ADM模块】 */
/**
* brief CNcomment:Creates a abstract data model example. CNcomment:创建一个抽象数据模型实例
* param[in] pDataSource CNcomment: 数据源操作属性
* param[out] phADM  ADM handle.CNcomment: 数据模型句柄
* retval ::HI_SUCCESS
* retval ::HI_ERR_ADM_NULLPTR
* retval ::HI_ERR_ADM_ZEROFIELD
* retval ::HI_ERR_ADM_OUTOFRANGE
* retval ::HI_ENORES
*/
HI_S32 HI_GV_ADM_Create(HIGV_ADMOPT_S *pDataSource, HIGV_HANDLE *phADM);

/**
* brief Create ADM by handle.CNcomment:根据指定的Handle创建一个抽象数据模型实例，给XML2Bin使用
* param[in] pDataSource operation of datasouce.CNcomment:数据源操作属性
* param[in] hADM   ADM handle.CNcomment: 数据模型句柄
* retval ::HI_SUCCESS
* retval ::HI_ERR_ADM_NULLPTR
* retval ::HI_ERR_ADM_ZEROFIELD
* retval ::HI_ERR_ADM_OUTOFRANGE
* retval ::HI_ENORES
*/
HI_S32 HI_GV_ADM_CreateByHandle(HIGV_ADMOPT_S *pDataSource, HIGV_HANDLE hADM);

/**
* brief Create DDB by default.CNcomment:根据默认的数据缓冲创建数据模型
* param[in] hDB DB handle.CNcomment:默认的数据缓冲句柄
* param[in] FieldCount Field count.CNcomment:字段个数
* param[in] pFieldAttr  Field attribute.CNcomment:字段属性
* param[out] phADM ADM handle.CNcomment:数据模型句柄
* retval ::HI_SUCCESS
* retval ::HI_ERR_ADM_NULLPTR
* retval ::HI_ERR_ADM_ZEROFIELD
* retval ::HI_ERR_ADM_OUTOFRANGE
* retval ::HI_ENORES
*/
HI_S32 HI_GV_ADM_CreateDefault(HIGV_HANDLE hDB, HI_U32 FieldCount, HIGV_FIELDATTR_S *pFieldAttr,
                               HIGV_HANDLE *phADM);

/**
* brief Create default DDB by handle.CNcomment:通过指定的Handle，根据默认的数据缓冲创建数据模型
* param[in] hDB  DB handle.CNcomment:默认的数据缓冲句柄
* param[in] FieldCount Field count.CNcomment:字段个数
* param[in] pFieldAttr  Field attribute.CNcomment:字段属性
* param[in] hADM ADM handle.CNcomment:数据模型句柄
* retval ::HI_SUCCESS
* retval ::HI_ERR_ADM_NULLPTR
* retval ::HI_ERR_ADM_OUTOFRANGE
* retval ::HI_ENORES
*/
HI_S32 HI_GV_ADM_CreateDefaultByHandle(HIGV_HANDLE hDB, HI_U32 FieldCount, HIGV_FIELDATTR_S *pFieldAttr,
                                       HIGV_HANDLE hADM);

/**
* brief Destroy ADM.CNcomment:删除一个数据模型实例
* param[in] hADM ADM handle.CNcomment:数据模型句柄
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
*/
HI_S32 HI_GV_ADM_Destroy(HIGV_HANDLE hADM);

/**
* brief Bind adm to widget.CNcomment:将一个控件绑定到数据模型
* param[in] hWidget Widget handle.CNcomment:控件句柄
* param[in] hADM ADM handle.CNcomment:数据模型句柄
* retval ::HI_SUCCESS
*/
HI_S32 HI_GV_ADM_Bind(HIGV_HANDLE hWidget, HIGV_HANDLE hADM);

/**
* brief Unbinde adm to widget.CNcomment:解除一个控件和数据模型的绑定
* param[in] hWidget Widget handle.CNcomment:控件句柄
* param[in] hADM ADM handle.CNcomment:数据模型句柄
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
*/
HI_S32 HI_GV_ADM_UnBind(HIGV_HANDLE hWidget, HIGV_HANDLE hADM);

/**
* brief Get the data type of field. CNcomment:获取指定字段的数据类型
* param[in] hADM ADM handle.CNcomment:数据模型句柄
* param[in] Col Column index.CNcomment:列号
* param[out] pFieldAttr Field attribute.CNcomment:字段属性
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_ADM_NULLPTR
* retval ::HI_ERR_ADM_OUTOFRANGE
*/
HI_S32 HI_GV_ADM_GetFieldAttr(HIGV_HANDLE hADM, HI_U32 Col, HIGV_FIELDATTR_S *pFieldAttr);

/**
* brief Get total row number.CNcomment:获取数据总行数
* param[in] hADM ADM handle.CNcomment:数据模型句柄
* param[out] pCount Row count.CNcomment:数据总数
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_ADM_NULLPTR
*/
HI_S32 HI_GV_ADM_GetRowCount(HIGV_HANDLE hADM, HI_U32 *pCount);

/**
* brief Get the data by row and column index.CNcomment:获取指定行指定列的数据
* param[in] hADM ADM handle.CNcomment:数据模型句柄
* param[in] Row Row index.CNcomment:行号
* param[in] Col Column index.CNcomment:列号
* param[out] pData CNcomment:数据内容
* param[in] Len pData CNcomment:所能容纳的字节数
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_ADM_NULLPTR
* retval ::HI_ERR_ADM_OUTOFRANGE
* retval ::HI_ERR_ADM_BUFFSMALL
* retval ::HI_ERR_ADM_GETDATA
*/
HI_S32 HI_GV_ADM_GetCellData(HIGV_HANDLE hADM, HI_U32 Row, HI_U32 Col, HI_VOID *pData, HI_U32 Len);

/**
* brief Get the data of column.CNcomment:获取指定行指定列的数据，以字符串表示
* param[in] hADM ADM handle. CNcomment:数据模型句柄
* param[in] Row Row index.CNcomment:行号
* param[in] Col Column index.CNcomment:列号
* param[out] pDataString Data content.CNcomment:数据内容
* param[in] Len pDataString buffer size.CNcomment:pDataString所能容纳的字节数
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_ADM_NULLPTR
* retval ::HI_ERR_ADM_OUTOFRANGE
* retval ::HI_ERR_ADM_BUFFSMALL pData
* retval ::HI_ERR_ADM_GETDATA
*/
HI_S32 HI_GV_ADM_GetCellDataString(HIGV_HANDLE hADM, HI_U32 Row, HI_U32 Col, HI_CHAR *pDataString, HI_U32 Len);

/**
* brief Set the callback function when data change.CNcomment:设置数据变化的回调函数
* param[in] hADM ADM handle.CNcomment:数据模型句柄
* param[in] CustomProc notify callback of data change.CNcomment:数据变化通知回调函数
* param[in] ProcOrder callback of time choice.CNcomment:回调的时间选择
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
*/
HI_S32 HI_GV_ADM_SetDCCallBack(HIGV_HANDLE hADM, HIGV_MSG_PROC CustomProc, HIGV_PROCORDER_E ProcOrder);

/**
* brief Get the DDB handle by ADM handle.CNcomment:根据ADM获取DDB
* param[in] ADM handle.CNcomment:hADM  数据模型句柄
* param[out] phDDB DDB handle.CNcomment:DDB句柄
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_ADM_NULLPTR
*/
HI_S32 HI_GV_ADM_GetDDBHandle(HIGV_HANDLE hADM, HIGV_HANDLE *phDDB);

/**
* brief Get column number.CNcomment:获取列总数
* param[in] hADM ADM handle.CNcomment:数据模型句柄
* param[out] pColNum colunm count.CNcomment:列总数
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_ADM_NULLPTR
*/
HI_S32 HI_GV_ADM_GetColNum(HIGV_HANDLE hADM, HI_U32 *pColNum);

/**
* brief Sync between  ADM and DB. CNcomment:ADM与DB同步接口,调用此接口后以后获取的数据就是与数据库同步的
* param[in] ADM handle.CNcomment:hADM 数据模型句柄
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
*/
HI_S32 HI_GV_ADM_Sync(HIGV_HANDLE hADM);

/**
* brief Clear all cache data in the ADM.CNcomment:清除所有ADM中的缓存数据
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
*/
HI_S32 HI_GV_ADM_ClearAllData(HI_VOID);

/* * ==== API declaration end ==== */
#ifdef __cplusplus
}
#endif
#endif /* __HI_GV_ADM_H__ */
