/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2009-2019. All rights reserved.
 * Description: Listbox wiget API
 * Author: NULL
 * Create: 2009-2-9
 */

#ifndef __HI_GV_LIST_H__
#define __HI_GV_LIST_H__
#include "hi_gv_conf.h"
#ifdef HIGV_USE_WIDGET_LISTBOX
/* add include here */
#include "hi_gv_resm.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ************************** Structure Definition *************************** */
/* * addtogroup      Listbox     */
/* *  【listbox widget】; CNcomment:  【listbox控件】 */
/* * Max col number */
#define HIGV_LISTBOX_COL_MAX 20

/* * Type of column ; CNcomment: ListBox控件列类型枚举 */
typedef enum {
    LIST_COLTYPE_TXT = 0,
    /* Text ; CNcomment:  文本 */
    LIST_COLTYPE_IMG,
    /* Picture ; CNcomment:  图片 */
    LIST_COLTYPE_TXTICONLEFT,
    /* Text and icon,icon on left ; CNcomment:  文本和图标，图标在左 */
    LIST_COLTYPE_TXTICONRIGHT,
    /* Text and icon,icon on right ; CNcomment:  文本和图标，图标在右 */
    LIST_COLTYPE_BUTT
} HIGV_LIST_COLTYPE_E;

/* * Callback function of convert string ; CNcomment: 字符串转换函数:将DB中保存的字符串转换为显示时需要的字符串 */
typedef HI_S32 (*HIGV_LIST_CONV_CB)(HIGV_HANDLE hList, HI_U32 Col, HI_U32 Item, const HI_CHAR *pSrcStr,
                                    HI_CHAR *pDstStr, HI_U32 Length);

/* At cell foucs mode, the callback function is used to get the cell width */
/* CNcomment:
* 单元格焦点模式下，控件用来获取单元格宽度的回调函数，HIGV会将所有需
* 要显示的单元格的行和列依次传入，根据返回值0-100来设置单元格宽度。0表
* 示隐藏单元格，100表示此单元格占满一行。返回值不在0-100内的不对宽度重
* 新设置
*/
typedef HI_S32 (*HIGV_GET_WIDTH_CB)(HIGV_HANDLE hList, HI_U32 Item, HI_U32 Col);

/* * Attribute structure of listbox Column ; CNcomment: Listbox控件列属性结构 */
typedef struct {
    HIGV_LIST_COLTYPE_E Type; /*  Column type ; CNcomment: 列类型 */
    HI_U32 Width; /*  Column width ; CNcomment: 列宽 */
    HI_U32 Align; /*  Alignment text ; CNcomment: 文本对齐方式 */
    HI_COLOR Fgidx; /*  Font color of column text; CNcomment: 列文本字体颜色 */
    HIGV_HANDLE hImage;  /* Picture surface handle (only support type is LIST_COLTYPE_IMG)
                            CNcomment:  当Type=LIST_COLTYPE_IMG时有效，需要显示的图片surface句柄 */
    HI_U32 FieldColIdxInDb; /* Column index in db ; CNcomment:  该列在数据库中的列号 */
    HIGV_LIST_CONV_CB ConvString;  /* function which convert string (only support type is LIST_COLTYPE_TXT)
                                    CNcomment:  字符串转换函数，只对LIST_COLTYPE_TXT有效 */
} HIGV_LIST_COLATTR_S;

/* * Attribute structure of listbox ;  CNcomment: Listbox控件属性结构 */
typedef struct {
    HI_U32 RowNum; /* Row number ; CNcomment:  行数目 */
    HI_U32 ColNum; /* Column number ; CNcomment:  列数目 */
    HI_BOOL NoFrame; /* Is no frame Draw ; CNcomment:  是否无边框 */
    HI_BOOL Cyc; /* Is cycle ; CNcomment:  是否循环滚动 */
    HI_BOOL IsCellActive; /* Is cell active ; CNcomment:  是否单元格焦点 */
    HI_BOOL AutoSwitchCell; /* If cell active auto suwich active cell ; CNcomment:  单元格焦点是否自动切换焦点 */
    HI_BOOL Scroll; /* Is scroll ; CNcomment:  是否滚动单元格显示 */
    HI_BOOL Fromleft; /* Is Scroll from left ; CNcomment:  是否从左边开始滚动 */
    HI_U32 Timeinterval; /* Time interval ; CNcomment:  滚动时间间隔 */
    HI_U32 Step; /* Scroll step ; CNcomment:  滚动单元格的滚动步长 */
    HI_U32 ScrollCol; /* If item active,scroll col of item ; CNcomment:  条目焦点时可滚动列 */
    HIGV_GET_WIDTH_CB GetWidthCb;  /*  Function get cell width from call back ;
                                    CNcomment:  控件内部回调用户设置单元格宽度函数 */
    HIGV_LIST_COLATTR_S *pColAttr;  /* List column attributes array ;
                                        CNcomment:  列表框控件列属性结构数组，有ColNum个元素 */
} HIGV_LIST_ATTRIBUTE_S;

/* *  Structure Definition end */
/* ****************************** API declaration **************************** */
/* *  addtogroup      Listbox     */
/* *  【listbox widget】 CNcomment:  【listbox控件】 */
/**
* brief Init the listbox.CNcomment: ListBox控件初始化
* attention ::
* CNcomment: Set the head of listbox.CNcomment: 设置listbox控件的表头(列数及列属性)和行数
* param[in] hList    CNcomment: ListBox控件句柄
* param[in] RowNum  Row number of dispaly. CNcomment: 显示行数
* param[in] ColNum   Column number.CNcomment: 列数
* param[in] pColAttr Column attribute array.CNcomment: 列属性数组(包含ColNum个元素)
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_INVAL
* retval ::HI_ERR_COMM_NORES
*/
HI_S32 HI_GV_List_Init(HIGV_HANDLE hList, HI_U32 RowNum, HI_U32 ColNum, const HIGV_LIST_COLATTR_S *pColAttr);

/* * addtogroup      Listbox     */
/* * 【listbox widget】 ; CNcomment:  【listbox控件】 */
/**
* brief Init the listbox.CNcomment: ListBox控件初始化
* attention ::
* CNcomment: Set the head of listbox.CNcomment: 设置listbox控件的表头(列数及列属性)和行数
* param[in] hList    CNcomment: ListBox控件句柄
* param[in] pListAttr  Init listbox attributes. CNcomment: 初始化列表框属性
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_INVAL
* retval ::HI_ERR_COMM_NORES
*/
HI_S32 HI_GV_List_InitEx(HIGV_HANDLE hList, const HIGV_LIST_ATTRIBUTE_S *pListAttr);

/**
* brief Get the content of listbox.CNcomment: 获取ListBox控件数据格的内容
* param[in] hList   Listbox handle.CNcomment: ListBox控件句柄
* param[in] Item  Item index. CNcomment: 行
* param[in] Col   Column index. CNcomment: 列
* param[out] pValue Cell's content.CNcomment: 单元格内容
* param[in] Length Length of pValue.CNcomment: pValue字符串长度
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST CNcomment: 句柄无效或者对就行和列没有数据
* retval ::HI_ERR_COMM_INVAL
*/
HI_S32 HI_GV_List_GetCell(HIGV_HANDLE hList, HI_U32 Item, HI_U32 Col, HI_CHAR *pValue, HI_U32 Length);

/**
* brief Set skin fo focus item.CNcomment: 设置焦点行的皮肤
* param[in] hList    Listbox handle. CNcomment: 控件句柄
* param[in] SelSkin   The skin of Selected status.CNcomment: 控件处于焦点状态下选中行皮肤
* param[in] NormSkin  The skin of normal status.CNcomment: 控件处于非焦点状态下选中行皮肤
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_INVAL
*/
HI_S32 HI_GV_List_SetRowSkin(HIGV_HANDLE hList, HIGV_HANDLE SelSkin, HIGV_HANDLE NormSkin);

/**
* brief Set selected item of listbox.CNcomment: 设置 ListBox当前条目
* attention ::The api is only support the selected item and apply this api at the init stage.
* CNcomment: 该接口仅设置当前选中条目，不刷新控件，主要在初始化Listbox控件时使用
* param[in] hList Listbox handle. CNcomment: 控件句柄
* param[in] Item  Item index.CNcomment: 条目编号
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_INVAL
* retval ::HI_ERR_COMM_PAERM Foucs moe error.CNcomment: 焦点模式错误
* see :: HI_GV_List_GetSelItem
*/
HI_S32 HI_GV_List_SetSelItem(HIGV_HANDLE hList, HI_U32 Item);

/**
* brief Get the item of listbox.CNcomment: 获取ListBox当前条目
* param[in] hList  Listbox handle.CNcomment: 控件句柄
* param[out] pItem Item index.CNcomment: 条目编号
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_INVAL
* see :: HI_GV_List_SetSelItem
*/
HI_S32 HI_GV_List_GetSelItem(HIGV_HANDLE hList, HI_U32 *pItem);

/**
* brief Get the total item number of listbox.CNcomment: 获取ListBox当前数据总条数
* param[in]  hList  Listbox handle. CNcomment:  控件句柄
* param[out] pItemNum Toal item number.CNcomment: 总条目数
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_INVAL
*/
HI_S32 HI_GV_List_GetItemNum(HIGV_HANDLE hList, HI_U32 *pItemNum);

/**
* brief CNcomment: 是否是ListBox控件类型
* param[in]  hList  CNcomment: 控件句柄
* retval ::HI_TRUE
* retval ::HI_FALSE
*/
HI_BOOL HI_GV_List_IsListBoxType(HIGV_HANDLE hList);

/**
* brief Set the attribute of listbox.CNcomment: 设置网格属性
* param[in]  hList  Listbox handle.CNcomment: 控件句柄
* param[in]  HLHeight  The height of horizontal line.CNcomment: 水平线高度
* param[in]  HLColor  The color of horizontal line.CNcomment: 水平线颜色
* param[in]  VLWidth The width of vertical line.CNcomment:  竖直线宽度
* param[in]  VLColor  The color of vertical line.CNcomment: 竖直线颜色
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_INVAL
* see ::HI_GV_List_GetGridding
*/
HI_S32 HI_GV_List_SetGridding(HIGV_HANDLE hList, HI_U32 HLHeight, HI_COLOR HLColor, HI_U32 VLWidth,
                              HI_COLOR VLColor);

/**
* brief Get the attribute of listbox.CNcomment: 获取网格属性
* param[in]  hList  Listbox handle.控件句柄
* param[out]  pHLHeight The height of horizontal line. CNcomment: 水平线高度
* param[out]  pHLColor  The color of horizontal line.CNcomment: 水平线颜色
* param[out]  pVLWidth  The width of vertical line.CNcomment: 竖直线宽度
* param[out]  pVLColor  The color of vertical line.CNcomment: 竖直线颜色
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_INVAL
* see ::HI_GV_List_SetGridding
*/
HI_S32 HI_GV_List_GetGridding(HIGV_HANDLE hList, HI_U32 *pHLHeight, HI_COLOR *pHLColor, HI_U32 *pVLWidth,
                              HI_COLOR *pVLColor);

/**
* brief Change the image of column.CNcomment: 改变显示的图片
* param[in] hList Listbox handle.CNcomment: 控件句柄
* param[in] Col    Column index.CNcomment: 列号
* param[in] hImage Picture handle.CNcomment: 图片句柄，如果设为无效句柄，则清除之前设置的有效句柄
* retval N/A.CNcomment:无
*/
HI_S32 HI_GV_List_ChangeImage(HIGV_HANDLE hList, HI_U32 Col, HIGV_HANDLE hImage);

/* brief Get the start index of current page. CNcomment: 获取当前显示页面首项的序号 */
HI_S32 HI_GV_List_GetStartItem(HIGV_HANDLE hList, HI_U32 *Item);

/* brief Set the start index of current page. CNcomment: 设置当前显示页面首项的序号 */
HI_S32 HI_GV_List_SetStartItem(HIGV_HANDLE hList, HI_U32 Item);

/* brief Get the index of the end item.CNcomment: 获取当前显示页面末项的序号 */
HI_S32 HI_GV_List_GetEndItem(HIGV_HANDLE hList, HI_U32 *Item);

/**
* brief Set the foucus of listbox.CNcomment: 设置ListBox当前焦点单元格
* attention : The api is only support the selected cell.
* CNcomment: 该接口仅设置当前选中单元格，不刷新控件
* param[in] hList  Listbox handle. CNcomment: ListBox控件句柄
* param[in] Item   Row index.CNcomment: 行
* param[in] Col    Column index.CNcomment: 列
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_INVAL
* retval ::HI_ERR_COMM_PAERM Error at focus mode.CNcomment: 焦点模式错误
* see ::HI_GV_List_GetSelCell
*/
HI_S32 HI_GV_List_SetSelCell(HIGV_HANDLE hList, HI_U32 Item, HI_U32 Col);

/**
* brief Get the position of cell which selected.CNcomment: 获取ListBox当前选中单元格位置
* param[in] hList  Listbox handle. CNcomment: ListBox控件句柄
* param[out] pItem   row index.CNcomment: 行
* param[out] pCol   Column index. CNcomment: 列
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_INVAL
* see ::HI_GV_List_SetSelCell
*/
HI_S32 HI_GV_List_GetSelCell(HIGV_HANDLE hList, HI_U32 *pItem, HI_U32 *pCol);

/**
* brief Get the width of cell at focus mode.CNcomment: 单元格焦点模式下获取单元格宽度
* attention : if the cell not display, it will return 0.
* CNcomment: 未显示的单元格返回0
* param[in] hList  Listbox handle. CNcomment: ListBox控件句柄
* param[in] Item   item index.CNcomment: 行
* param[in] Col    Column index.CNcomment: 列
* param[out] pWidth  The pointer of width.CNcomment: 获取宽度的指针
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_INVAL
*/
HI_S32 HI_GV_List_GetCellWidth(HIGV_HANDLE hList, HI_U32 Item, HI_U32 Col, HI_U32 *pWidth);

/**
* brief Set scroll step of text.CNcomment: 设置文本的滚动步长
* attention : it is only valid at foucs mode.
* CNcomment: 单元格焦点模式下有效
* param[in] hList Listbox handle.CNcomment: 控件句柄
* param[in] Step Scroll step(unit:pixel, default:10piexl).CNcomment: 滚动步长(以pixel为单位，默认为10pixel)
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_INVAL
* retval ::HI_ERR_COMM_PAERM Error at focus mode.CNcomment: 焦点模式错误
*/
HI_S32 HI_GV_List_SetStep(HIGV_HANDLE hList, HI_U32 Step);

/**
* brief Set the scroll direction of listbox.CNcomment: 设置滚动方向
* attention : it is only valid at foucs mode.
* CNcomment: 单元格焦点模式下有效
* param[in] hList Listbox handle.CNcomment: 控件句柄
* param[in] FromLeft Scroll direction.CNcomment: 滚动方向，默认HI_FALSE自右向左,设置HI_TRUE表示从左向右滚动
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_INVAL
* retval ::HI_ERR_COMM_PAERM Error at focus mode.CNcomment: 焦点模式错误
*/
HI_S32 HI_GV_List_SetDirection(HIGV_HANDLE hList, HI_BOOL FromLeft);

/**
* brief  Set the status of widget.CNcomment: 设置控件状态
* attention : it is only valid at foucs mode.
* CNcomment: 单元格焦点模式下有效
* param[in] hList Listbox handle.CNcomment: 控件句柄
* param[in] Scroll Scroll status.CNcomment: 滚动状态(TRUE为滚动，FALSE为停止滚动，默认为FALSE)
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_INVAL
* retval ::HI_ERR_COMM_PAERM Error at focus mode.CNcomment: 焦点模式错误
*/
HI_S32 HI_GV_List_SetScroll(HIGV_HANDLE hList, HI_BOOL Scroll);

/**
* brief Set the foreground color of column.CNcomment: 设置列前景颜色
* attention :it is only support the column type is LIST_COLTYPE_TXT,
* if col is -1, it will effect the color of text to all column .
* CNcomment: 文本类型为LIST_COLTYPE_TXT时有效,Col为-1时设置所有列字体颜色
* param[in] hList Listbox handle.CNcomment: 控件句柄
* param[in] Col   Column index.CNcomment: 列
* param[in] FgIdx Foreground color index.CNcomment: 颜色值
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_INVAL
* retval ::HI_ERR_COMM_PAERM Unsupport the column to set picture.CNcomment: 不支持设置图片列
*/
HI_S32 HI_GV_List_SetColFgIdx(HIGV_HANDLE hList, HI_U32 Col, HI_COLOR FgIdx);

/**
* brief Set the exterior line of listbox.CNcomment: 设置是否绘制网格外框
* param[in] hList Listbox handle.CNcomment: 控件句柄
* param[in] NoFrame Is not draw.CNcomment: 是否绘制(TRUE不绘，FALSE绘，默认为FALSE)
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
*/
HI_S32 HI_GV_List_SetNoFrame(HIGV_HANDLE hList, HI_BOOL NoFrame);

/**
* brief Set the foucs whether cycle.CNcomment: 设置是否循环焦点
* attention :It is only valid at focus mode.
* CNcomment: 条目焦点模式下有效
* param[in] hList Listbox handle.CNcomment: 控件句柄
* param[in] Cyc Focus cycle.CNcomment: TRUE循环，FALSE不循环，默认为FALSE
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
*/
HI_S32 HI_GV_List_SetCyc(HIGV_HANDLE hList, HI_BOOL Cyc);

/**
* brief Set scroll param.CNcomment:设置滑动灵敏度系数(默认为1.0，大于等于1.0)
\attention \n
N/A.CNcomment:无
* param[in] hList Listbox handle.    CNcomment:控件句柄
* param[in] ScrollParam scroll index.    CNcomment:滑动灵敏度系数
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_INVAL
*/
HI_S32 HI_GV_List_SetScrollParam(HIGV_HANDLE hList, HI_FLOAT ScrollParam);

/**
* brief Set fling param.CNcomment:设置轻扫灵敏度系数(默认为2，大于等于1)
* param[in] hList Listbox handle.    CNcomment:控件句柄
* param[in] FlingParam fling index.    CNcomment:轻扫灵敏度系数
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_INVAL
*/
HI_S32 HI_GV_List_SetFlingParam(HIGV_HANDLE hList, HI_S32 FlingParam);

/* API declaration end ==== */
#ifdef __cplusplus
}
#endif
#endif
#endif /* __HI_GV_LIST_H__ */
