/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2009-2019. All rights reserved.
 * Description: Muti-language API
 * Author: NULL
 * Create: 2009-2-9
 */

#ifndef __HI_GV_LAN_H__
#define __HI_GV_LAN_H__

/* add include here */
#ifdef __cplusplus
extern "C" {
#endif

/* ****************************** Structure Definition  **************************** */
/* * addtogroup      Language */
/* *  【language】 CNcomment:【多语言】 */
/* language country abbreviation */
/* *he format of languge is <language>_<territory>. The <language> should satisfy iso639-2 and <territory> should
 * satisfy iso3166-1
*/
/**  CNcomment: 语言格式为:    语言_地区。如:zh_CN表示中文大陆地区
* 其中语言满足iso639-2,国家地区编码简写满足iso3166-1。
* zh_TW表示中文台湾地区。HIGV内部根据下划线左边部 *
* 分来判定文字方向，预留右边地区为方便用户设定
* 同种语言的不同地区字符串 。用户也可以自定义下列
* 已存在的语言中，不同地区的语言(按照zh_CN的格式)
*/

#define LAN_CN    "cn"    /* china <CNcomment:简体中文，为匹配以前版本等同于LAN_ZN */
#define LAN_ZH    "zh"    /* china <CNcomment::简体中文，等同于LAN_ZN_CN */
#define LAN_ZH_CN "zh_CN" /* china <CNcomment::简体中文，等同于LAN_ZN */
#define LAN_ZH_TW "zh_TW" /* taiwan <CNcomment::繁体中文(台湾) */
#define LAN_ZH_HK "zh_HK" /* hongkong <CNcomment::繁体中文(香港) */
#define LAN_AR    "ar"    /* arabic <CNcomment::阿拉伯文 */
#define LAN_CS    "cs"    /* arabic <CNcomment::捷克文 */
#define LAN_DA    "da"    /* arabic <CNcomment::丹麦文 */
#define LAN_DE    "de"    /* germary <CNcomment::德文(德国) */
#define LAN_DE_AT "de_AT" /* germary(austria)  <CNcomment::德文(奥地利) */
#define LAN_DE_CH "de_CH" /* germary(swiss) <CNcomment::德文(瑞士) */
#define LAN_EL    "el"    /* greece <CNcomment::希腊文 */
#define LAN_EN    "en"    /* english <CNcomment::英文，等同于en_US */
#define LAN_EN_US "en_US" /* english(america)  <CNcomment::英文(美国)，等同于en */
#define LAN_EN_CA "en_CA" /* english(canada)   <CNcomment::英文(加拿大) */
#define LAN_EN_GB "en_GB" /* english(england)  <CNcomment::英文(英国) */
#define LAN_ES    "es"    /* espana  <CNcomment::西班牙文 */
#define LAN_FA    "fa"    /* persia  <CNcomment::波斯文(伊朗) */
#define LAN_FI    "fi"    /* finnish <CNcomment::芬兰文 */
#define LAN_FR    "fr"    /* french(france)  <CNcomment::法文(法国) */
#define LAN_FR_CA "fr_CA" /* french(canada)    <CNcomment: 法文(加拿大) */
#define LAN_FR_CH "fr_CH" /* french(swizerland)    <CNcomment: 法文(瑞士) */
#define LAN_IT    "it"    /* italian <CNcomment: 意大利文(意大利) */
#define LAN_IT_CH "it_CH" /* italian   <CNcomment: 意大利文(瑞士) */
#define LAN_HE    "he"    /* hebrew  <CNcomment: 希伯来文 */
#define LAN_JA    "ja"    /* japan   <CNcomment: 日文 */
#define LAN_KO    "ko"    /* korea   <CNcomment: 朝鲜文 */
#define LAN_KO_KR "ko_KR" /* corean    <CNcomment: 韩文 */
#define LAN_NL    "nl"    /* holand  <CNcomment: 荷兰文(荷兰) */
#define LAN_NL_BE "nl_BE" /* holand    <CNcomment: 荷兰文(比利时) */
#define LAN_PT    "pt"    /* portuguese(portugal)    <CNcomment: 葡萄牙文(葡萄牙) */
#define LAN_PT_BR "pt_BR" /* portuguese(brazil)    <CNcomment: 葡萄牙文(巴西) */
#define LAN_RU    "ru"    /* russain <CNcomment: 俄文 */
#define LAN_SV    "sv"    /* swedish <CNcomment: 瑞典 */
#define LAN_TH    "th"    /* thai    <CNcomment: 泰文 */
#define LAN_TR    "tr"    /* Turkish <CNcomment: 土耳其文 */
#define LAN_PO    "po"    /* Polski  <CNcomment: 波兰文 */

/* Direction of language   *CNcomment: 语言的文本书写方向 */
typedef enum {
    HI_LAN_DIR_LTR = 0, /* Left to right   *<CNcomment:从左向右书写的语言 */
    HI_LAN_DIR_RTL = 1,  /* Right to left   *<CNcomment:从右向左书写的语言 */
    HI_LAN_DIR_BUTT
} HI_LAN_DIR;

/*-- Structure Definition end */
/* ****************************** API declaration **************************** */
/* *  addtogroup      Language */
/* *  【language】 CNcomment: 【多语言】 */
/**
* brief Register the multi-language.CNcomment:注册需要切换的语言信息
* attention:This api should call after the BIN loaded.
* CNcomment:语言注册函数放到加载BIN文件之后调用，在加载视图前调用
* param[in] pResFile CNcomment:界面中需要变换指定语言字符串
* param[in] hFont 对CNcomment:应该的字体句柄
* param[in] pLangID Language ID.CNcomment:语言标识
* retval ::HI_SUCCESS Success.CNcomment:成功
* retval ::HI_ERR_COMM_INVAL Invalid paramater.CNcomment:参数不合法，比如句柄无效，资源文件无效，有空行等
* retval ::HI_ERR_COMM_EXIST This language has registered.CNcomment:该语言已经注册
* retval ::HI_ERR_LANG_INVALID_LOCALE  Locale is not setting.CNcomment:locale语言未设置
* retval ::HI_ERR_COMM_FULL  the max language is 16 or The total string count is over
* 128*1024.CNcomment:注册的语言超过系统个数最大16个，或者资源文件行数超过128*1024行 \retval ::HI_ERR_COMM_NORES
* CNcomment:没有资源，例如系统内存不够
* see: HI_GV_Lan_UnRegister
*/
HI_S32 HI_GV_Lan_Register(const HI_CHAR *pResFile, HIGV_HANDLE hFont, const HI_CHAR *pLangID);

/**
* brief Unregister the language.CNcomment:反注册已注册的语言信息
* param[in] pLangID Language ID.CNcomment:pLangID 语言标识
* retval ::HI_SUCCESS Success.CNcomment:成功
* retval ::HI_ERR_COMM_INVAL Invalid paramater.CNcomment:参数不合法，比如语言还未注册等
* etval ::HI_ERR_COMM_PAERM Unsupport operation.CNcomment:不支持的操作，如注销本地语言或注销当前语言
* see :HI_GV_Lan_Register
*/
HI_S32 HI_GV_Lan_UnRegister(const HI_CHAR *pLangID);

/**
* brief Register the multi-language.CNcomment:设置本地语言
* param[in] pLangID Language ID.CNcomment:pLangID 语言标识
* retval ::HI_SUCCESS Success.CNcomment:成功
* retval ::HI_ERR_COMM_INVAL Invalid paramater.CNcomment:参数不合法，比如语言还未注册等
* retval ::HI_ERR_COMM_PAERM Unsupport operation.CNcomment:不支持的操作，如注销本地语言或注销当前语言
*/
HI_S32 HI_GV_Lan_SetLocale(const HI_CHAR *pLocale);

/**
* brief Reigster font info of the language. CNcomment:注册多语言的字体信息
* param[in]  pLangID  Language ID.CNcomment:语言标识
* param[in]  hFont Font handle.CNcomment:对应该的字体句柄
* retval ::HI_SUCCESS Success.CNcomment:成功
* retval ::HI_ERR_LOSTInvalid handle. CNcomment:句柄无效
* retval ::HI_ERR_COMM_INVAL Invalid paramater.CNcomment:无效参数
*/
HI_S32 HI_GV_Lan_FontRegister(const HI_CHAR *pLangID, HIGV_HANDLE hFont);

/**
* brief Change the language.CNcomment:切换到指定语言，可改变语言的风格控件将收到HIGV_MSG_LANCHNGE消息
* param[in] pLangID Language ID.CNcomment:语言标识，应用程序自己指定和定义，但必须是在注册的语言集中
* retval ::HI_SUCCESS Success.CNcomment:成功
* retval ::HI_ERR_COMM_EMPTY Invalid paramater.CNcomment:该语言ID在系统中找不到，可能没有注册
* see:::HI_GV_Lan_Register
*/
HI_S32 HI_GV_Lan_Change(const HI_CHAR *pLangID);

/**
* brief Find current language id.CNcomment:找到当前语言ID
* param[in] pLangID Language ID.CNcomment:存入当前语言ID
* retval ::HI_SUCCESS Success.CNcomment:成功
* retval ::HI_ERR_COMM_EMPTY The language is not registered.CNcomment:系统没有注册过语言
* retval ::HI_ERR_COMM_INVAL Invalid paramater.CNcomment:参数不合法
* see : ::HI_GV_Lan_Register
*/
HI_S32 HI_GV_Lan_GetCurLangID(HI_CHAR **ppLangID);

/**
* brief Get the text by string id.CNcomment:根据字符串ID，取得当前语言文本
* param[in]  StrID  String ID.CNcomment:字符串ID
* param[in]  pLangID  Language ID.CNcomment:语言ID
* param[out] ppStr The pointer of string.CNcomment:字符串指针
* retval ::HI_SUCCESS Success.CNcomment:成功
* retval ::HI_ERR_COMM_LOST Invalid handle.CNcomment:句柄无效
* retval ::HI_ERR_COMM_INVAL Invalid paramater.CNcomment:无效参数
*/
HI_S32 HI_GV_Lan_GetLangString(const HI_CHAR *pLangID, const HI_U32 StrID, HI_CHAR **ppStr);

/**
* brief Add string.CNcomment:添加非xml定义的字符串
* You should register langugae before adding strings.CNcomment:添加外部字符串需要调用HI_GV_Lan_Register接口注册语言
* param[in]  pLangID  Language ID.CNcomment:语言ID
* param[in]  StrID  String ID.CNcomment:字符串ID
* param[out] pStrID The pointer of string id.CNcomment:字符串ID指针
* retval ::HI_SUCCESS Success.CNcomment:成功
* retval ::HI_EFULL String item is full.CNcomment:字符串已满(只支持1024个)
* retval ::HI_EINVAL Invalid paramater.CNcomment:无效参数
* see :HI_GV_Lan_Register(...)
*/
HI_S32 HI_GV_AddLanString(const HI_CHAR *pLangID, HI_CHAR *pStr, HI_U32 *pStrID);

/**
* brief Get the dirction of the language.CNcomment:获取指定语言方向
* param[in] pDirection Direction.CNcomment:存入当前语言方向
* param[in] pLangID   Language ID.CNcomment:获取方向的语言
* retval ::HI_SUCCESS Success.CNcomment:成功
* see : HI_GV_Lan_Register
*/
HI_S32 HI_GV_Lan_GetLangIDDirection(const HI_CHAR *pLangID, HI_LAN_DIR *pDirection);

/* * ==== API declaration end ==== */
#ifdef __cplusplus
}
#endif
#endif /* __HI_GV_LAN_H__ */
