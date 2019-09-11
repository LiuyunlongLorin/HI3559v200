/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2009-2019. All rights reserved.
 * Description: HIGV resouce manger API
 * Author: NULL
 * Create: 2009-2-9
 */

#ifndef __HI_GV_RESM_H__
#define __HI_GV_RESM_H__

/* add include here */
#include "hi_type.h"
#include "hi_go.h"
#include "hi_gv_conf.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UNLIMITED_REPEATE_COUNT -1

/* ************************** Structure Definition *************************** */
/* * addtogroup      Resource   */
/* * 【Resource manger】 */
/* * 【资源管理】 */
/* * Invalid resource ID ; CNcomment: 无效的资源ID */
#define INVALID_RESID 0
/* * The max length of resource file name */
/* * CNcomment: 资源文件名的最大长度 */
#define HIGV_RESM_MAX_FILENAME_LEN 256

/* * The max alpha value */
/* * CNcomment: 最大的Alpha值 */
#define HIGV_RESM_MAX_ALPHA 255

/* *Resouce ID */
/* *CNcomment: 资源ID */
typedef HI_PARAM HI_RESID;

/* *Resouce type */
/* *CNcomment:  资源类别 */
typedef enum hiHIGV_RESTYPE_E {
    HIGV_RESTYPE_IMG,
    /* < Picture resouce */
    /* <CNcomment:  图片资源 */
    HIGV_RESTYPE_FONT,
    /* <Font resource */
    /* <CNcomment:  字体资源 */
    HIGV_RESTYPE_BUTT
} HIGV_RESTYPE_E;

/* *Skin type */
/* *CNcomment:  皮肤类型 */
typedef enum hiHIGV_STYLE_TYPE_E {
    HIGV_STYLETYPE_COLOR = 0,
    /* <Color type <CNcomment:  颜色 */
    HIGV_STYLETYPE_PIC /* <Picture type <CNcomment:  图片 */
} HIGV_STYLE_TYPE_E;

/*  Style union */
/*  CNcomment: style联合 */
typedef union hiHIGV_STYLE_MEMBER_U {
    HI_COLOR Color; /* < Color value  ; CNcomment:  颜色值 */
    HI_RESID ResId; /* <Resouce ID ; CNcomment:  资源ID */
} HIGV_STYLE_MEMBER_U;

/*  The style structure ; CNcomment: style结构 */
typedef struct hiHIGV_STYLE_S {
    HIGV_STYLE_TYPE_E StyleType; /* <Style type ; CNcomment:  Style类型 */
    HIGV_STYLE_MEMBER_U
    Top;     /* <Top side (color or picture resource)  ; CNcomment:  上边缘颜色值或者图片资源ID */
    HIGV_STYLE_MEMBER_U Bottom;
    /* <Bottom side (color or picture resource) */
    /* <CNcomment:  下边缘颜色值或者图片资源ID */
    HIGV_STYLE_MEMBER_U Left; /* <Left side (color or picture resource ; CNcomment:  左边缘颜色值或者图片资源ID */
    HIGV_STYLE_MEMBER_U Right;    /* <Right side (color or picture resource) */
                                 /* <CNcomment:  右边缘颜色值或者图片资源ID */
    HIGV_STYLE_MEMBER_U LeftTop;    /* <Top left co (color or picture resource) */
                                     /* <CNcomment:  左上角颜色值或者图片资源ID */
    HIGV_STYLE_MEMBER_U LeftBottom;    /* <Top side (color or picture resource) */
                                         /* <CNcomment:  左下角颜色值或者图片资源ID */
    HIGV_STYLE_MEMBER_U RightTop;    /* <Right top side (color or picture resource) */
                                    /* <CNcomment:  右上角颜色值或者图片资源ID */
    HIGV_STYLE_MEMBER_U RightBottom;    /* <Right bottom side (color or picture resource) */
                                         /* <CNcomment:  右下角颜色值或者图片资源ID */
    HIGV_STYLE_MEMBER_U BackGround;    /* <Background (color or picture resource) */
                                          /* <CNcomment:  背景颜色值或图片资源ID */
    HI_COLOR FontColor; /* <Font color <CNcomment:  文字颜色值  */
    HI_U32 bNoDrawBg;   /* <Is not draw background <CNcomment:  控制是否画背景颜色值 */
    HI_U32 LineWidth;   /* <Line width <CNcomment:  控制填充颜色宽度 */
} HIGV_STYLE_S;

typedef struct hiHIGV_FONT_S {
    HI_RESID SbFontID;
    HI_RESID MbFontID;
    HI_U32 Size;
    HI_BOOL bBold;
    HI_BOOL bItalic;
} HIGV_FONT_S;

typedef struct hiHIGV_DEC_SUFINFO_S {
    HIGO_PF_E PixFormat;    /* <CNcomment: 解码后表面的像素格式，默认为HIGO_PF_8888 */
    HIGO_MEMTYPE_E MemType; /* <CNcomment: 解码后表面的存储类别，默认为MMZ */
    HI_BOOL IsPubPalette;   /* <CNcomment: 解码图片是否使用公共调色板,只对输出格式有效 */
} HIGV_DEC_SUFINFO_S;

typedef struct {
    HI_CHAR *pTopName;         /* <Resouce file name of top side <CNcomment:  顶部的资源文件名 */
    HI_CHAR *pBottomName;      /* <Resouce file name of bottom side <CNcomment:  底部的资源文件名 */
    HI_CHAR *pLeftName;        /* <Resouce file name of left side <CNcomment:  左边的资源文件名 */
    HI_CHAR *pRightName;       /* <Resouce file name of right side <CNcomment:  右边的资源文件名 */
    HI_CHAR *pLeftTopName;     /* <Resouce file name of top and left side <CNcomment:  左上的资源文件名 */
    HI_CHAR *pLeftBottomName;  /* <Resouce file name of bottom and left side <CNcomment:  左下的资源文件名 */
    HI_CHAR *pRightTopName;    /* <Resouce file name of right and up side <CNcomment:  右上的资源文件名 */
    HI_CHAR *pRightBottomName; /* <Resouce file name of right and bottom  side <CNcomment:  右下的资源文件名 */
    HI_CHAR *pBackGroundName;  /* <Resouce file name of background <CNcomment:  背景的资源文件名 */
} HIGV_STYLEFILENAME_S;

/*  The Anim structure */
/*  CNcomment: Anim结构 */
/*  Anim与ID对应关系表 */
/*  Anim type */ /* *CNcomment:  动画类型 */
typedef enum hiHIGV_ANIM_TYPE_E {
    /* <Translate type */
    /* <CNcomment: 平移 */
    HIGV_ANIMTYPE_TRANSLATE = 0,
    /* <Alpha type */
    /* <CNcomment:  Alpha */
    HIGV_ANIMTYPE_ALPHA,
    /* <Roll type */
    /* <CNcomment:  卷帘 */
    HIGV_ANIMTYPE_ROLL,
    /* <Any type */
    /* <CNcomment:  任意 */
    HIGV_ANIMTYPE_ANY,
    HIGV_ANIMTYPE_INVALIDATE
} HIGV_ANIM_TYPE_E;

/* *Animation repeat type */
/* *CNcomment:  动画重复类型 */
typedef enum hiHIGV_ANIM_REPEAT_TYPE_E {
    /* <Repeate type restart */
    /* <CNcomment: 从头开始 */
    HIGV_REPEATE_TYPE_RESTART = 0,
    /* <Repeate type reverse */
    /* <CNcomment: 从结束的位置继续 */
    HIGV_REPEATE_TYPE_REVERSE
} HIGV_ANIM_REPEAT_TYPE_E;

/* * Animation roll type */
/* * CNcomment:  Animation roll type卷帘方向 */
typedef enum {
    HIGV_ANIM_ROLL_UP = 0, /**< Up */
    HIGV_ANIM_ROLL_DOWN,   /**< Down */
    HIGV_ANIM_ROLL_LEFT,   /**< Letf */
    HIGV_ANIM_ROLL_RIGHT   /**< Right */
} HIGV_ANIM_ROLL_DIRECTION_E;

/* * Translate info */
/* * CNcomment:  Translate info 平移信息 */
typedef struct hiHIGV_ANIM_TRANSLATE_INFO_S {
    HI_S32 FromX; /* < 开始坐标X */
    HI_S32 FromY; /* < 开始坐标Y */
    HI_S32 ToX;   /* < 结束坐标X */
    HI_S32 ToY;   /* < 结束坐标Y */
} HIGV_ANIM_TRANSLATE_INFO_S;

/* * Alpha info */
/* * CNcomment:  Alpha info Alpha信息 */
typedef struct hiHIGV_ANIM_ALPHA_INFO_S {
    HI_U32 FromApha; /* < 开始Alpha */
    HI_U32 ToAlpha;  /* < 结束Alpha */
} HIGV_ANIM_ALPHA_INFO_S;

/* * Any info */
/* * CNcomment:  Any info 通用信息 */
typedef struct hiHIGV_ANIM_ANY_INFO_S {
    HI_S32 FromValue; /* < 开始 Value */
    HI_S32 ToValue;   /* < 结束 Value */
} HIGV_ANIM_ANY_INFO_S;

/* * Roll info */
/* * CNcomment:  Roll info 卷帘信息 */
typedef struct hiHIGV_ANIM_ROLL_INFO_S {
    HIGV_ANIM_ROLL_DIRECTION_E DirectionType; /* < 卷帘方向 */
    HI_S32 FromLoc;                           /* < 开始位置 */
    HI_S32 ToLoc;                             /* < 结束位置 */
} HIGV_ANIM_ROLL_INFO_S;

/* * Animation info */
/* * CNcomment:  Animation info 动画信息 */
typedef struct hiHIGV_ANIM_INFO_S {
    HI_U32 AnimHandle;                  /* < Anim ID */
    HI_U32 DurationMs;                  /* < 持续时间 */
    HI_S32 RepeatCount;                 /* < 循环次数 */
    HIGV_ANIM_REPEAT_TYPE_E RepeatMode; /* < 循环模式 */
    HI_U32 DelayStart;                  /* < 延迟执行时间 */
    HIGV_ANIM_TYPE_E AnimType;          /* < 动画类型 */
    union {
        HIGV_ANIM_TRANSLATE_INFO_S Translate;
        HIGV_ANIM_ALPHA_INFO_S Alpha;
        HIGV_ANIM_ROLL_INFO_S Roll;
        HIGV_ANIM_ANY_INFO_S Any;
    } AnimParam;
} HIGV_ANIM_INFO_S;

/* * ==== Structure Definition end ==== */
/* ****************************** API declaration **************************** */
/* * addtogroup      Resource   */
/* * 【Resource manager】; CNcomment: 【资源管理】 */
/**
* brief Create resource ID. CNcomment: 创建资源ID
* attention \n
* 1. The picture use ID in the system.
* 2. This function is effected by resouce envirnment var
* CNcomment:
* 1. 图片在系统内以ID标识
* 2. 该接口受环境变量影响，如果设置了资源路径前缀环境变量，则会自动在资源路径前面增加该前缀
*     字体资源相对路径前缀( HIGV_RES_FONT_PATH),
*     图片资源相对路径前缀(HIGV_RES_IMAGE_PATH)
* param[in]  pFileName    Picture file name. CNcomment: 图片文件名
* param[in]  ResType Resouce type. CNcomment: 资源类型
* param[out] pResID   Resource ID. CNcomment: 资源ID
* retval ::HI_SUCCESS Success.CNcomment: 成功
* retval ::HI_ERR_COMM_LOST Invalid handle. CNcomment: 句柄无效
* retval ::HI_ERR_COMM_NORES No memory. CNcomment: 系统中没有内存
* see ::HI_GV_Res_DestroyID
*/
HI_S32 HI_GV_Res_CreateID(const HI_CHAR *pFileName, HIGV_RESTYPE_E ResType, HI_RESID *pResID);

/**
* brief Create resource ID no res prefix path. CNcomment: 创建资源ID不读取资源前缀环境变量
* attention:
* The picture use ID in the system.
* CNcomment: 图片在系统内以ID标识
* param[in]  pFileName    Picture file name. CNcomment: 图片文件名
* param[in]  ResType Resouce type. CNcomment: 资源类型
* param[out] pResID   Resource ID. CNcomment: 资源ID
* retval ::HI_SUCCESS Success.CNcomment: 成功
* retval ::HI_ERR_COMM_LOST Invalid handle. CNcomment: 句柄无效
* retval ::HI_ERR_COMM_NORES No memory. CNcomment: 系统中没有内存
* see ::HI_GV_Res_DestroyID
*/
HI_S32 HI_GV_Res_CreateID_NoPrefixPath(const HI_CHAR *pFileName, HIGV_RESTYPE_E ResType, HI_RESID *pResID);

/**
* brief Disable application use resource enviroment variables. CNcomment: 强制应用不使用资源路径的环境变量
* If you set the reource enviroment variable, and don't want usnet the env variable, you can call this fuction disable.
* [HIGV_RES_FONT_PATH, HIGV_RES_IMAGE_PATH] \attention \n N/A.CNcomment:无 \param[in]  disable  boolean parameter.
* CNcomment: 使能布尔值
* retval ::HI_SUCCESS Success.CNcomment: 成功
* see :: HI_GV_Res_CreateID_NoPrefixPath
*/
HI_S32 HI_GV_Res_DisablePrefixPath(HI_BOOL disable);

/**
* brief Destroy resource id. CNcomment: 销毁资源句柄
* param[in] ResID Rouce ID. CNcomment: 资源ID
* retval ::HI_SUCCESS Success.CNcomment: 成功
* retval ::HI_ERR_RES_UNKNOWRES Coun't find the resouce id .CNcomment: 待删除的资源ID在系统中找不到
* retval ::HI_ERR_RES_USED The resouce is used.CNcomment: 资源正在使用
* see :: HI_GV_Res_CreateID
*/
HI_S32 HI_GV_Res_DestroyID(HI_RESID ResID);

/**
* brief Generate style. CNcomment: 生成Style
* param[in]  pStyle   Generate the infomation of the skin. CNcomment: 皮肤各子项资源ID信息
* param[out] phStyle  Skin handle.CNcomment: 皮肤句柄
* retval ::HI_SUCCESS Success.CNcomment: 成功
* retval ::HI_ERR_COMM_LOST Invalid handle.CNcomment: 句柄无效
* retval ::HI_ERR_COMM_NORES No memory.CNcomment: 系统中没有内存
* retval ::HI_ERR_COMM_INVAL Invalid paramater.CNcomment: 无效参数
\* see :: HI_GV_Res_DestroyStyle
*/
HI_S32 HI_GV_Res_CreateStyle(const HIGV_STYLE_S *pStyle, HI_RESID *phStyle);

/**
* brief Generate Style by the resource ID. CNcomment: 根据指定的RESID生成Style
* param[in] pStyle Style pointer.CNcomment: 创建参数
* param[in] hStyle  Style handle.CNcomment: 句柄
* retval ::HI_SUCCESS Success.CNcomment: 成功
* retval ::HI_ERR_COMM_NORES No memory.CNcomment: 系统中没有内存
* retval ::HI_ERR_COMM_INVAL Invalid parater.CNcomment: 无效参数
*/
HI_S32 HI_GV_Res_CreateStyleByHandle(const HIGV_STYLE_S *pStyle, HIGV_HANDLE hStyle);

/**
* brief Destroy style. CNcomment: 销毁Style
* param[in] hStyle  Style handle.CNcomment: Style句柄
* retval ::HI_SUCCESS Success.CNcomment: 成功
* retval ::HI_ERR_RES_UNKNOWRES Coun't find the resouce id.CNcomment: 待删除的资源ID在系统中找不到
* see ::HI_GV_Res_CreateStyle
*/
HI_S32 HI_GV_Res_DestroyStyle(HIGV_HANDLE hStyle);

/**
* brief Set the resource to persistent memory. CNcomment: 设置资源常驻内存
* param[in] ResID Resource handle.CNcomment: 资源句柄
* retval ::HI_SUCCESS Success.CNcomment: 成功
* retval ::HI_ERR_RES_UNKNOWRES Coun't find the resouce id.CNcomment: 资源ID在系统中找不到
*/
HI_S32 HI_GV_Res_SetResident(HI_RESID ResID);

/**
* brief Create font. CNcomment: 创建字体
* param[in]  pFontInfo the point of Font information. CNcomment: 字体创建信息
* param[out] pFont     Font handle. CNcomment: 字体句柄
* retval ::HI_SUCCESS Success. CNcomment: 成功
* retval ::HI_ERR_COMM_LOST Invalid handle.CNcomment: 句柄无效
* retval ::HI_ERR_COMM_NORES No memory. CNcomment: 系统中没有内存
* retval ::HI_ERR_RES_UNKNOWRES Coun't find the resouce id.CNcomment: 资源ID在系统中找不到
* see ::HI_GV_Font_Destroy
*/
HI_S32 HI_GV_Font_Create(const HIGV_FONT_S *pFontInfo, HIGV_HANDLE *pFont);

/**
* brief Create font by the handle. CNcomment: 根据指定的句柄创建字体，为XML2Bin提供
* param[in] pFontInfo CNcomment:  字体创建信息
* param[in] hFont     CNcomment: 字体句柄
* retval ::HI_SUCCESS Success.CNcomment: 成功
* retval ::HI_ERR_COMM_LOST Invalid handle.CNcomment: 句柄无效
* retval ::HI_ERR_COMM_NORES No memory. CNcomment: 系统中没有内存
* retval ::HI_ERR_RES_UNKNOWRES CNcomment: 资源ID在系统中找不到
*/
HI_S32 HI_GV_Font_CreateByHandle(const HIGV_FONT_S *pFontInfo, HIGV_HANDLE hFont);

/**
* brief Destroy font.CNcomment: 删除字体
* param[in] hFont  Font hanlde.CNcomment: 字体句柄
* retval ::HI_SUCCESS Success.CNcomment: 成功
* retval ::HI_ERR_RES_UNKNOWRES Coun't find the font.CNcomment: 系统中没有找到对应的字体
* see ::HI_GV_Font_Create
*/
HI_S32 HI_GV_Font_Destroy(HIGV_HANDLE hFont);

/**
* brief Set system default font. CNcomment: 设置系统默认字体
* param[in] hFont Font handle.CNcomment: 字体资源句柄
* retval ::HI_SUCCESS CNcomment: 成功
* see ::HI_GV_Font_GetSystemDefault
*/
HI_S32 HI_GV_Font_SetSystemDefault(HIGV_HANDLE hFont);

/**
* brief Get system default font.CNcomment: 获取系统缺省字体
* param[out] pFont Font handle.CNcomment: 字体句柄
* retval ::HI_SUCCESS Success.CNcomment: 成功
* retval ::HI_ERR_COMM_INVAL Invalid handle.CNcomment: 无效参数
* see ::HI_GV_Font_SetSystemDefault
*/
HI_S32 HI_GV_Font_GetSystemDefault(HIGV_HANDLE *pFont);

/**
* brief Get String width and height.CNcomment: 获取字符串的宽高
* param[in] hFont Font handle.CNcomment: 字体资源句柄
* param[in] hStr String handle.CNcomment: 字符串句柄
* param[out] pWidth String width.CNcomment: 字符串宽度
* param[out] pHeight String height.CNcomment: 字符串高度
* retval ::HI_SUCCESS Success.CNcomment: 成功
* retval ::HI_ERR_COMM_INVAL Invalid handle.CNcomment: 无效参数
*/
HI_S32 HI_GV_Font_GetTextExtent(HIGV_HANDLE hFont, HI_RESID hStr, HI_S32 *pWidth, HI_S32 *pHeight);

/**
* brief Destroy all style. CNcomment: 释放所有Style
* attention \n
* Release the resource of style and don't rlease the picture resouce.
* CNcomment: 释放Style本身资源，并不释放Style引用的图片资源
* param N/A.CNcomment:无
* retval ::HI_SUCCESS      CNcomment: 删除成功
* retval ::HI_ERR_RES_USED CNcomment: 资源正被使用
*/
HI_S32 HI_GV_Resm_DestroyAllStyle(HI_VOID);

/**
* brief Destroy all resource. CNcomment: 释放所有图片、字体资源
* param N/A.CNcomment:无
* retval ::HI_SUCCESS   Success.CNcomment: 删除成功
*/
HI_S32 HI_GV_Resm_DestroyAllRes(HI_VOID);

/**
* brief Force to releaee all resouce.CNcomment: 强制释放所有图片、字体资源
* param N/A.CNcomment:无
* retval ::HI_SUCCESS   Success.CNcomment: 删除成功
*/
HI_S32 HI_GV_Resm_ForceUnloadAllRes(HI_VOID);

/**
* brief Set the information of decode surface. CNcomment: 设置解码后表面信息
* param[in] DecSufinfo The infomation of decode surface.CNcomment: 解码表面信息
* retval ::HI_SUCCESS Success.CNcomment: 成功
* retval ::HI_ERR_COMM_INVAL Invalid paramater.CNcomment: 无效参数
*/
HI_S32 HI_GV_Resm_SetDecSurfInfo(HIGV_DEC_SUFINFO_S *pDecSufinfo);

/**
* brief Get the information of decode surface. CNcomment: 获取解码后表面信息
* param[out] pDecSufinfo The surface infomation of decode surface.CNcomment: 解码表面信息
* retval ::HI_SUCCESS Success.CNcomment: 成功
* retval ::HI_ERR_COMM_INVAL Invalid paramater.CNcomment: 无效参数
*/
HI_S32 HI_GV_Resm_GetDecSurfInfo(HIGV_DEC_SUFINFO_S *pDecSufinfo);

/**
* brief Get the information of the surface which is for picture reource ID.CNcomment: 获取图片资源ID对应的surface信息
* param[in]  ResID Resouce ID.CNcomment: 资源ID
* param[out] pRes  the handle of resouce surface. CNcomment: 资源surface句柄
* retval ::HI_SUCCESS  Success.CNcomment: 成功
* retval ::HI_ERR_RES_UNKNOWRES Coun't find the resouce id. CNcomment: 资源ID不存在
* retval ::HI_ERR_RES_LOAD   Load failed.CNcomment: 加载失败，可能是字体创建或图片解码失败
*/
HI_S32 HI_GV_Res_GetResInfo(HI_RESID ResID, HIGV_HANDLE *pRes);

/**
* brief Free the information of the surface which is for picture reource ID.CNcomment: 释放图片资源ID对应的surface信息
* param[in]  ResID Resouce ID.CNcomment: 资源ID
* retval ::HI_SUCCESS  Success.CNcomment: 成功
* retval ::HI_ERR_RES_UNKNOWRES Coun't find the resouce id. CNcomment: 资源ID不存在
* retval ::HI_ERR_RES_NOLOAD   No Load.CNcomment: 资源未加载
*/
HI_S32 HI_GV_Res_ReleaseResInfo(HI_RESID ResID);

/**
* brief Create font set.CNcomment: 创建字体集
* attention
* CNcomment: 字体集是具有相同风格的在不同语言下可替换的字体集合
* param[out] pFont   CNcomment:  字体句柄
* retval ::HI_SUCCESS Success.CNcomment:  成功
* retval ::HI_ERR_COMM_LOST Invalid handle.CNcomment: 句柄无效
* retval ::HI_ERR_RES_UNKNOWRES Coun't find the font. CNcomment: 资源ID在系统中找不到
* see::HI_GV_FontSet_Destroy
*/
HI_S32 HI_GV_FontSet_Create(HIGV_HANDLE *pFontSet);

/**
* brief Add font to font set. CNcomment: 添加字体到字体集中
* attention
* pSupportLan CNcomment: 为空，表示支持所有语言字体；字体不允许重复添加
* param[in] hFont Font handle.CNcomment:  字体句柄
* param[in]  pSupportLan The language which font support. CNcomment:  字体支持的语言，多种语言使用分号隔开，如"cn;en;" ，
* 镅源虢ㄒ槭褂肐SO-639 标准 \param[in]  hFontSet  The handle of font set.CNcomment: 字体集句柄
* retval ::HI_SUCCESS Success.CNcomment: 成功
* retval ::HI_ERR_COMM_LOST Invalid handle.CNcomment: 句柄无效
* retval ::HI_ERR_COMM_NORES No memory.CNcomment: 系统中没有内存
* retval ::HI_ERR_RES_UNKNOWRES Coun't find the reouce id. CNcomment: 资源ID在系统中找不到
* see ::HI_GV_FontSet_Destroy
*/
HI_S32 HI_GV_FontSet_AddFont(HIGV_HANDLE hFont, const HI_CHAR *pSupportLan, HIGV_HANDLE hFontSet);

/**
* brief  Query the font style which is the same as hFont from font set.CNcomment: 查询字体集中风格与hFont相同的字体
* param[in]  pLan CNcomment:  字体支持的语言，语言代码建议使用ISO-639 标准
* param[in] hFont     Font handle. CNcomment: 字体句柄
* param[in]  phSupprotFont  CNcomment: 字体集中风格与hFont相同的字体
* retval ::HI_SUCCESS Success.CNcomment: 成功
* retval ::HI_ERR_COMM_LOST Invalid handle.CNcomment: 句柄无效
* retval ::HI_ERR_COMM_NORES No memory. CNcomment: 系统中没有内存
* retval ::HI_ERR_RES_UNKNOWRES Coun't find the resouce id. CNcomment: 资源ID在系统中找不到
* see ::HI_GV_FontSet_Destroy
*/
HI_S32 HI_GV_FontSet_Query(HI_CHAR *pLan, HIGV_HANDLE hFont, HIGV_HANDLE *phSupprotFont);

/**
* brief Destroy font set.CNcomment: 删除字体集
* retval ::HI_SUCCESS Success.CNcomment: 成功
* see :: HI_GV_Font_Create
*/
HI_S32 HI_GV_FontSet_Destroy(HI_VOID);

/**
* brief Get style file name.CNcomment: 获取Style文件名
* param[in]  hStyle     Style handle.CNcomment:  style句柄
* param[out] pStyleFileNameInfo  the file names of resouce.CNcomment: 获取组成style的资源文件名
* retval ::HI_SUCCESS
*/
HI_S32 HI_GV_Res_GetStyleFileName(HI_RESID hStyle, HIGV_STYLEFILENAME_S *pStyleFileNameInfo);

/**
* brief print resource information.CNcomment: 打印资源信息
* param N/A.CNcomment:无
* retval ::HI_SUCCESS
*/
HI_S32 HI_GV_Res_PrintCurLoadResInfo(HI_VOID);

/**
* brief Create animtion information. CNcomment: 创建动画信息
* param[in]  pAnimInfo the point of animtion information. CNcomment: 创建动画信息
* param[out] phAnim  animtion information handle.CNcomment: 动画信息句柄
* retval ::HI_SUCCESS Success. CNcomment: 成功
* retval ::HI_ERR_COMM_NORES No memory. CNcomment: 系统中没有内存
* see ::HI_GV_AnimInfo_Destroy
*/
HI_S32 HI_GV_AnimInfo_Create(const HIGV_ANIM_INFO_S *pAnimInfo, HIGV_HANDLE *phAnim);

/**
* brief Create animtion information by the handle. CNcomment: 根据指定的句柄创建动画信息，为XML2Bin提供
* param[in]  pAnimInfo the point of animation information. CNcomment: 创建动画信息
* param[in]  hAnim  Anim handle.CNcomment: 动画信息句柄
* retval ::HI_SUCCESS Success. CNcomment: 成功
* retval ::HI_ERR_COMM_LOST Invalid handle.CNcomment: 句柄无效
* reval ::HI_ERR_COMM_NORES No memory. CNcomment: 系统中没有内存
*/
HI_S32 HI_GV_AnimInfo_CreateByHandle(const HIGV_ANIM_INFO_S *pAnimInfo, HIGV_HANDLE hAnim);

/**
* brief Destroy font.CNcomment: 删除动画信息
* param[in]  hAnim  Anim handle.CNcomment: 动画信息句柄
* retval ::HI_SUCCESS Success.CNcomment: 成功
* retval ::HI_ERR_RES_UNKNOWRES Coun't find the anim.CNcomment: 系统中没有找到对应的动画
* see ::HI_GV_AnimInfo_Create
*/
HI_S32 HI_GV_AnimInfo_Destroy(HIGV_HANDLE hAnim);

/**
* brief Get system default font.CNcomment: 获取动画信息
* param[in]  hAnim  Anim handle.CNcomment: 动画信息句柄
* param[out] pAnimInfo   Anim info. CNcomment: 动画信息
* retval ::HI_SUCCESS Success.CNcomment: 成功
* retval ::HI_ERR_COMM_INVAL Invalid handle.CNcomment: 无效参数
* see ::HI_GV_AnimInfo_Create
*/
HI_S32 HI_GV_AnimInfo_Get(HIGV_HANDLE hAnim, HIGV_ANIM_INFO_S *pAnimInfo);

/* *==== API declaration end ==== */
#ifdef __cplusplus
}
#endif
#endif /* __HI_GV_RESM_H__ */
