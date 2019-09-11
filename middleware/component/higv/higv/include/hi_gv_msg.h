/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2009-2019. All rights reserved.
 * Description: HIGV message module API
 * Author: NULL
 * Create: 2009-2-9
 */

#ifndef __HI_GV_MSG_H__
#define __HI_GV_MSG_H__

/* add include here */
#include "hi_type.h"
#include "hi_gv_conf.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ************************** Structure Definition *************************** */
/*  addtogroup      Message */
/*   【message】    CNcomment:【消息】   */
/*   The structure of message   CNcomment:消息信息结构 */
typedef struct hiHIGV_MSGINFO_S {
    HI_U32 HighLevelMsgNum; /* <High level message number , CNcomment: 高优先级队列中消息数 */
    HI_U32 MiddleLevelMsgNum; /* <Middle level message number , CNcomment:中优先级队列中消息数 */
    HI_U32 LowLevelMsgNum; /* <Low level message number , CNcomment: 低优先级中消息数 */
    HI_U32 CanLoseMsgNum; /* <The message number which can lose, CNcomment: 可丢失消息队列中消息数 */
    HI_U32 LoseMsgNum; /* <The message number which  losed, CNcomment: 可丢失消息队列中实际已发生的丢消息数 */
    HI_U32 NoLoseMsgNum; /* <The message number which cann't lose, CNcomment: 不可丢失消息队列中的消息数 */
} HIGV_MSGINFO_S;

/* * Key type ;  CNcomment:按键类型 */
typedef enum hiHIGV_KEYTYPE_E {
    HIGV_KEYTYPE_UNICODE = 0x0,
    /* < Ascii code  CNcomment: ascii码表中定义的键值 */
    HIGV_KEYTYPE_FUNCTION = 0x10000,
    /*  Function key  CNcomment: 功能键 */
} HIGV_KEYTYPE_E;

#define HIGV_KEY_CAT(type, index) ((HIGV_KEYTYPE_##type) | (index))

/* * Vitual key defined(include keyboard, softkey board.   CNcomment:虚拟按键值定义 (遥控器,软键盘) */
typedef enum hiHIGV_KEYCODE_E {
    HIGV_KEY_0 = HIGV_KEY_CAT(UNICODE, 0x30),
    HIGV_KEY_1 = HIGV_KEY_CAT(UNICODE, 0x31),
    HIGV_KEY_2 = HIGV_KEY_CAT(UNICODE, 0x32),
    HIGV_KEY_3 = HIGV_KEY_CAT(UNICODE, 0x33),
    HIGV_KEY_4 = HIGV_KEY_CAT(UNICODE, 0x34),
    HIGV_KEY_5 = HIGV_KEY_CAT(UNICODE, 0x35),
    HIGV_KEY_6 = HIGV_KEY_CAT(UNICODE, 0x36),
    HIGV_KEY_7 = HIGV_KEY_CAT(UNICODE, 0x37),
    HIGV_KEY_8 = HIGV_KEY_CAT(UNICODE, 0x38),
    HIGV_KEY_9 = HIGV_KEY_CAT(UNICODE, 0x39),
    /* * The follow is digit key  ; CNcomment:在遥控器中以下开始按键复用数字按键 */
    HIGV_KEY_A = HIGV_KEY_CAT(UNICODE, 0x41),
    HIGV_KEY_B = HIGV_KEY_CAT(UNICODE, 0x42),
    HIGV_KEY_C = HIGV_KEY_CAT(UNICODE, 0x43),
    HIGV_KEY_D = HIGV_KEY_CAT(UNICODE, 0x44),
    HIGV_KEY_E = HIGV_KEY_CAT(UNICODE, 0x45),
    HIGV_KEY_F = HIGV_KEY_CAT(UNICODE, 0x46),
    HIGV_KEY_G = HIGV_KEY_CAT(UNICODE, 0x47),
    HIGV_KEY_H = HIGV_KEY_CAT(UNICODE, 0x48),
    HIGV_KEY_I = HIGV_KEY_CAT(UNICODE, 0x49),
    HIGV_KEY_J = HIGV_KEY_CAT(UNICODE, 0x4A),
    HIGV_KEY_K = HIGV_KEY_CAT(UNICODE, 0x4B),
    HIGV_KEY_L = HIGV_KEY_CAT(UNICODE, 0x4C),
    HIGV_KEY_M = HIGV_KEY_CAT(UNICODE, 0x4D),
    HIGV_KEY_N = HIGV_KEY_CAT(UNICODE, 0x4E),
    HIGV_KEY_O = HIGV_KEY_CAT(UNICODE, 0x4F),
    HIGV_KEY_P = HIGV_KEY_CAT(UNICODE, 0x50),
    HIGV_KEY_Q = HIGV_KEY_CAT(UNICODE, 0x51),
    HIGV_KEY_R = HIGV_KEY_CAT(UNICODE, 0x52),
    HIGV_KEY_S = HIGV_KEY_CAT(UNICODE, 0x53),
    HIGV_KEY_T = HIGV_KEY_CAT(UNICODE, 0x54),
    HIGV_KEY_U = HIGV_KEY_CAT(UNICODE, 0x55),
    HIGV_KEY_V = HIGV_KEY_CAT(UNICODE, 0x56),
    HIGV_KEY_W = HIGV_KEY_CAT(UNICODE, 0x57),
    HIGV_KEY_X = HIGV_KEY_CAT(UNICODE, 0x58),
    HIGV_KEY_Y = HIGV_KEY_CAT(UNICODE, 0x59),
    HIGV_KEY_Z = HIGV_KEY_CAT(UNICODE, 0x5A),

    HIGV_KEY_a = HIGV_KEY_CAT(UNICODE, 0x61),
    HIGV_KEY_b = HIGV_KEY_CAT(UNICODE, 0x62),
    HIGV_KEY_c = HIGV_KEY_CAT(UNICODE, 0x63),
    HIGV_KEY_d = HIGV_KEY_CAT(UNICODE, 0x64),
    HIGV_KEY_e = HIGV_KEY_CAT(UNICODE, 0x65),
    HIGV_KEY_f = HIGV_KEY_CAT(UNICODE, 0x66),
    HIGV_KEY_g = HIGV_KEY_CAT(UNICODE, 0x67),
    HIGV_KEY_h = HIGV_KEY_CAT(UNICODE, 0x68),
    HIGV_KEY_i = HIGV_KEY_CAT(UNICODE, 0x69),
    HIGV_KEY_j = HIGV_KEY_CAT(UNICODE, 0x6A),
    HIGV_KEY_k = HIGV_KEY_CAT(UNICODE, 0x6B),
    HIGV_KEY_l = HIGV_KEY_CAT(UNICODE, 0x6C),
    HIGV_KEY_m = HIGV_KEY_CAT(UNICODE, 0x6D),
    HIGV_KEY_n = HIGV_KEY_CAT(UNICODE, 0x6E),
    HIGV_KEY_o = HIGV_KEY_CAT(UNICODE, 0x6F),
    HIGV_KEY_p = HIGV_KEY_CAT(UNICODE, 0x70),
    HIGV_KEY_q = HIGV_KEY_CAT(UNICODE, 0x71),
    HIGV_KEY_r = HIGV_KEY_CAT(UNICODE, 0x72),
    HIGV_KEY_s = HIGV_KEY_CAT(UNICODE, 0x73),
    HIGV_KEY_t = HIGV_KEY_CAT(UNICODE, 0x74),
    HIGV_KEY_u = HIGV_KEY_CAT(UNICODE, 0x75),
    HIGV_KEY_v = HIGV_KEY_CAT(UNICODE, 0x76),
    HIGV_KEY_w = HIGV_KEY_CAT(UNICODE, 0x77),
    HIGV_KEY_x = HIGV_KEY_CAT(UNICODE, 0x78),
    HIGV_KEY_y = HIGV_KEY_CAT(UNICODE, 0x79),
    HIGV_KEY_z = HIGV_KEY_CAT(UNICODE, 0x7A),

    HIGV_KEY_BACKSPACE = HIGV_KEY_CAT(UNICODE, 0x08), /* for keyboard */
    HIGV_KEY_SPACE = HIGV_KEY_CAT(UNICODE, 0x20),
    HIGV_KEY_LF = HIGV_KEY_CAT(UNICODE, 0x0A),
    HIGV_KEY_CR = HIGV_KEY_CAT(UNICODE, 0x0D),  /* for keyboard */
    HIGV_KEY_TAB = HIGV_KEY_CAT(UNICODE, 0x09), /* for keyboard */
    HIGV_KEY_ESC = HIGV_KEY_CAT(UNICODE, 0x1B),
    HIGV_KEY_DELETE = HIGV_KEY_CAT(UNICODE, 0x7F), /* for keyboard */
    /* * key '`'/'-'/'=' and '~'/'_'/'+' */        /* *CNcomment:键盘上侧的10个数字键两侧的符号键及其对应的转换键 */
    HIGV_KEY_BACKQUOTE = HIGV_KEY_CAT(UNICODE, 0x60),
    HIGV_KEY_MINUS = HIGV_KEY_CAT(UNICODE, 0x2D),
    HIGV_KEY_EQUAL = HIGV_KEY_CAT(UNICODE, 0x3D),

    HIGV_KEY_TILDE = HIGV_KEY_CAT(UNICODE, 0x7E),
    HIGV_KEY_UNDERSCORE = HIGV_KEY_CAT(UNICODE, 0x5F),
    HIGV_KEY_PLUS = HIGV_KEY_CAT(UNICODE, 0x2B),
    /* * the shift key of number key */ /* *CNcomment:键盘上侧的10个数字键对应的转换键 */
    HIGV_KEY_EXCLAM = HIGV_KEY_CAT(UNICODE, 0x21),
    HIGV_KEY_AT = HIGV_KEY_CAT(UNICODE, 0x40),
    HIGV_KEY_NUMBERSIGN = HIGV_KEY_CAT(UNICODE, 0x23),
    HIGV_KEY_DOLLAR = HIGV_KEY_CAT(UNICODE, 0x24),
    HIGV_KEY_PERCENT = HIGV_KEY_CAT(UNICODE, 0x25),
    HIGV_KEY_CARET = HIGV_KEY_CAT(UNICODE, 0x5E),
    HIGV_KEY_AMPERSAND = HIGV_KEY_CAT(UNICODE, 0x26),
    HIGV_KEY_ASTERISK = HIGV_KEY_CAT(UNICODE, 0x2A),
    HIGV_KEY_PARENLEFT = HIGV_KEY_CAT(UNICODE, 0x28),
    HIGV_KEY_PARENRIGHT = HIGV_KEY_CAT(UNICODE, 0x29),
    /* * the punctuation key on keyboard */ /* *CNcomment:键盘右侧的8个标号符号键 */
    HIGV_KEY_BRACKETLEFT = HIGV_KEY_CAT(UNICODE, 0x5B),
    HIGV_KEY_BRACKETRIGHT = HIGV_KEY_CAT(UNICODE, 0x5D),
    HIGV_KEY_BACKSLASH = HIGV_KEY_CAT(UNICODE, 0x5C),
    HIGV_KEY_SEMICOLON = HIGV_KEY_CAT(UNICODE, 0x3B),
    HIGV_KEY_QUOTE = HIGV_KEY_CAT(UNICODE, 0x27),
    HIGV_KEY_COMMA = HIGV_KEY_CAT(UNICODE, 0x2C),
    HIGV_KEY_DOT = HIGV_KEY_CAT(UNICODE, 0x2E),
    HIGV_KEY_SLASH = HIGV_KEY_CAT(UNICODE, 0x2F),
    /* * the shift key of punctuation key on keyboard */ /* *CNcomment:键盘右侧的8个标号符号键对应的转换键 */
    HIGV_KEY_BRACELEFT = HIGV_KEY_CAT(UNICODE, 0x7B),
    HIGV_KEY_BRACERIGHT = HIGV_KEY_CAT(UNICODE, 0x7D),
    HIGV_KEY_BAR = HIGV_KEY_CAT(UNICODE, 0x7C),
    HIGV_KEY_COLON = HIGV_KEY_CAT(UNICODE, 0x3A),
    HIGV_KEY_DOUBLEQUOTE = HIGV_KEY_CAT(UNICODE, 0x22),
    HIGV_KEY_LESS = HIGV_KEY_CAT(UNICODE, 0x3C),
    HIGV_KEY_GREATER = HIGV_KEY_CAT(UNICODE, 0x3E),
    HIGV_KEY_QUESTION = HIGV_KEY_CAT(UNICODE, 0x3F),

    /* * Function Key */
    HIGV_KEY_ENTER = HIGV_KEY_CAT(FUNCTION, 0x00),
    HIGV_KEY_DEL = HIGV_KEY_CAT(FUNCTION, 0x02), /* advise to use HIGV_KEY_DELETE */
    HIGV_KEY_BACK = HIGV_KEY_CAT(FUNCTION, 0x03),
    HIGV_KEY_EXIT = HIGV_KEY_CAT(FUNCTION, 0x04),
    HIGV_KEY_IME = HIGV_KEY_CAT(FUNCTION, 0x05),

    HIGV_KEY_UP = HIGV_KEY_CAT(FUNCTION, 0x06),
    HIGV_KEY_DOWN = HIGV_KEY_CAT(FUNCTION, 0x07),
    HIGV_KEY_LEFT = HIGV_KEY_CAT(FUNCTION, 0x08),
    HIGV_KEY_RIGHT = HIGV_KEY_CAT(FUNCTION, 0x09),
    HIGV_KEY_PGUP = HIGV_KEY_CAT(FUNCTION, 0x0A),
    HIGV_KEY_PGDOWN = HIGV_KEY_CAT(FUNCTION, 0x0B),

    HIGV_KEY_FAV = HIGV_KEY_CAT(FUNCTION, 0x0C),
    HIGV_KEY_MORE = HIGV_KEY_CAT(FUNCTION, 0x0D),
    HIGV_KEY_PREV = HIGV_KEY_CAT(FUNCTION, 0x0E),
    HIGV_KEY_REC = HIGV_KEY_CAT(FUNCTION, 0x0F),
    HIGV_KEY_NEXT = HIGV_KEY_CAT(FUNCTION, 0x10),
    HIGV_KEY_SLOW = HIGV_KEY_CAT(FUNCTION, 0x11),
    HIGV_KEY_RADIO = HIGV_KEY_CAT(FUNCTION, 0x12),
    HIGV_KEY_LIVE = HIGV_KEY_CAT(FUNCTION, 0x13),

    HIGV_KEY_SEARCH = HIGV_KEY_CAT(FUNCTION, 0x14),
    HIGV_KEY_MENU = HIGV_KEY_CAT(FUNCTION, 0x15),
    HIGV_KEY_PLAY = HIGV_KEY_CAT(FUNCTION, 0x16),
    HIGV_KEY_STOP = HIGV_KEY_CAT(FUNCTION, 0x17),
    HIGV_KEY_REW = HIGV_KEY_CAT(FUNCTION, 0x18),
    HIGV_KEY_FF = HIGV_KEY_CAT(FUNCTION, 0x19),
    HIGV_KEY_EPG = HIGV_KEY_CAT(FUNCTION, 0x1A),
    HIGV_KEY_NVOD = HIGV_KEY_CAT(FUNCTION, 0x1B),

    HIGV_KEY_BTV = HIGV_KEY_CAT(FUNCTION, 0x1C),
    HIGV_KEY_VOD = HIGV_KEY_CAT(FUNCTION, 0x1D),
    HIGV_KEY_NPVR = HIGV_KEY_CAT(FUNCTION, 0x1E),
    HIGV_KEY_HELP = HIGV_KEY_CAT(FUNCTION, 0x1F),
    HIGV_KEY_STOCK = HIGV_KEY_CAT(FUNCTION, 0x20),
    HIGV_KEY_GUIDE = HIGV_KEY_CAT(FUNCTION, 0x21),
    HIGV_KEY_MAIL = HIGV_KEY_CAT(FUNCTION, 0x22),
    HIGV_KEY_NEWS = HIGV_KEY_CAT(FUNCTION, 0x23),

    HIGV_KEY_IPANEL = HIGV_KEY_CAT(FUNCTION, 0x24),
    HIGV_KEY_SET = HIGV_KEY_CAT(FUNCTION, 0x25),
    HIGV_KEY_SUB = HIGV_KEY_CAT(FUNCTION, 0x26),
    HIGV_KEY_SEEK = HIGV_KEY_CAT(FUNCTION, 0x27),
    HIGV_KEY_INFO = HIGV_KEY_CAT(FUNCTION, 0x28),
    HIGV_KEY_POWER = HIGV_KEY_CAT(FUNCTION, 0x29),
    HIGV_KEY_AUDIO = HIGV_KEY_CAT(FUNCTION, 0x2A),
    HIGV_KEY_MUTE = HIGV_KEY_CAT(FUNCTION, 0x2B),

    HIGV_KEY_CHNUP = HIGV_KEY_CAT(FUNCTION, 0x2C),
    HIGV_KEY_CHNDOWN = HIGV_KEY_CAT(FUNCTION, 0x2D),
    HIGV_KEY_VOLUP = HIGV_KEY_CAT(FUNCTION, 0x2E),
    HIGV_KEY_VOLDOWN = HIGV_KEY_CAT(FUNCTION, 0x2F),
    HIGV_KEY_CAPS = HIGV_KEY_CAT(FUNCTION, 0x30),
    HIGV_KEY_SHIFT = HIGV_KEY_CAT(FUNCTION, 0x31),
    HIGV_KEY_CTRL = HIGV_KEY_CAT(FUNCTION, 0x32),
    HIGV_KEY_ALT = HIGV_KEY_CAT(FUNCTION, 0x33),

    HIGV_KEY_RED = HIGV_KEY_CAT(FUNCTION, 0x34),
    HIGV_KEY_YELLOW = HIGV_KEY_CAT(FUNCTION, 0x35),
    HIGV_KEY_BLUE = HIGV_KEY_CAT(FUNCTION, 0x36),
    HIGV_KEY_GREEN = HIGV_KEY_CAT(FUNCTION, 0x37),

    HIGV_KEY_P1 = HIGV_KEY_CAT(FUNCTION, 0x38),
    HIGV_KEY_P2 = HIGV_KEY_CAT(FUNCTION, 0x39),
    HIGV_KEY_F1 = HIGV_KEY_CAT(FUNCTION, 0x3A),
    HIGV_KEY_F2 = HIGV_KEY_CAT(FUNCTION, 0x3B),
    HIGV_KEY_F3 = HIGV_KEY_CAT(FUNCTION, 0x3C),
    HIGV_KEY_F4 = HIGV_KEY_CAT(FUNCTION, 0x3D),
    HIGV_KEY_PLAYPAUSE = HIGV_KEY_CAT(FUNCTION, 0x3E),
    HIGV_KEY_HOME = HIGV_KEY_CAT(FUNCTION, 0x3F),
    HIGV_KEY_PICTURE_MODE = HIGV_KEY_CAT(FUNCTION, 0x40),
    HIGV_KEY_SOUND_MODE = HIGV_KEY_CAT(FUNCTION, 0x41),
    HIGV_KEY_ASPECT = HIGV_KEY_CAT(FUNCTION, 0x42),
    HIGV_KEY_3D = HIGV_KEY_CAT(FUNCTION, 0x43),
    HIGV_KEY_SOURCE = HIGV_KEY_CAT(FUNCTION, 0x44),
    HIGV_KEY_CC = HIGV_KEY_CAT(FUNCTION, 0x45),
    HIGV_KEY_SWAP = HIGV_KEY_CAT(FUNCTION, 0x46),
    HIGV_KEY_FREEZE = HIGV_KEY_CAT(FUNCTION, 0x47),
    HIGV_KEY_PIP = HIGV_KEY_CAT(FUNCTION, 0x48),
    HIGV_KEY_NICAM_MTS = HIGV_KEY_CAT(FUNCTION, 0x49),

    HIGV_KEY_LWIN = HIGV_KEY_CAT(FUNCTION, 0x5B),
} HIGV_KEYCODE_E;

/* *==== Structure Definition end ==== */
/* ****************************** API declaration **************************** */
/* * addtogroup      Message */
/* * 【message】    【消息】   */
/**
* brief Send synchroniztion message. CNcomment:发送同步消息
* param[in] hWidget Widget handle.CNcomment:窗口或控件
* param[in] MsgId  Messag ID. The msg id is defined in "hi_gv_widget.h"
* CNcomment:消息ID，具体消息定义参考头文件hi_gv_widget.h \param[in] Param1  Paramater 1.
* CNcomment:参数1，其含义由消息类型确定，对按键消息来说是按键值 \param[in] Param2  Paramater 2.
* CNcomment:参数2，其含义由消息类型确定，对按键消息来说是特殊按键(如shift、ctrl等)状态 \param[in] TimeOut Time
* out.CNcomment:超时时间
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_INVAL
* retval ::HI_ERR_COMM_PAERM Unsupport operation, such as, send key message.CNcomment:该操作不允许，比如发送按銉消息
* see ::
* HI_GV_Msg_SendAsync
* HI_GV_Msg_PostAsync
* HI_GV_Msg_SendAsyncWithData
*/
HI_S32 HI_GV_Msg_SendSync(HIGV_HANDLE hWidget, HI_U32 MsgId, HI_PARAM Param1, HI_PARAM Param2, HI_U32 TimeOut);

/**
* brief Send the asynchronism. CNcomment:发送不可丢失的异步消息
* param[in] hWidget Wiget handle.CNcomment:窗口或控件
* param[in] MsgId  Messag ID. The msg id is defined in "hi_gv_widget.h"
* .CNcomment:消息ID，具体消息定义参考头文件hi_gv_widget.h \param[in] Param1  Paramater 1, the meaning of message is
* decided by message type.CNcomment:参数1，其含义由消息类型确定，对按键消息来说是按键值 \param[in] Param2  Paramater 2,
* the meaning of message is decided by message
* type.CNcomment:参数2，其含义由消息类型确定，对按键消息来说是特殊按键(如shift、ctrl等)状态
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_INVAL
* retval ::HI_ERR_COMM_PAERM Unsupport operation, such as, send key message.CNcomment:该操作不允许，比如发送按銉消息
* see
* ::HI_GV_Msg_SendSync
* ::HI_GV_Msg_PostAsync
* ::HI_GV_Msg_SendAsyncWithData
*/
HI_S32 HI_GV_Msg_SendAsync(HIGV_HANDLE hWidget, HI_U32 MsgId, HI_PARAM Param1, HI_PARAM Param2);

/**
* brief Send the asynchronism which can loss.CNcomment:发送可丢失的异步消息
* param[in] hWidget Widget handle.CNcomment:窗口或控件
* param[in] MsgId  Messag ID. The msg id is defined in "hi_gv_widget.h"
* .CNcomment:消息ID，具体消息定义参考头文件hi_gv_widget.h \param[in] Param1  Paramater 1, the meaning of message is
* decided by message type.CNcomment:参数1，其含义由消息类型确定，对按键消息来说是按键值
* param[in] Param2  Paramater 2,the meaning of message is decided by message
* type.CNcomment:参数2，其含义由消息类型确定，对按键消息来说是特殊按键(如shift、ctrl等)状态
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_INVAL
* retval ::HI_ERR_COMM_PAERM Unsupport operation, such as, send key message.CNcomment:该操作不允许，比如发送按銉消息
* see
* ::HI_GV_Msg_SendSync
* ::HI_GV_Msg_SendAsync
* ::HI_GV_Msg_SendAsyncWithData
*/
HI_S32 HI_GV_Msg_PostAsync(HIGV_HANDLE hWidget, HI_U32 MsgId, HI_PARAM Param1, HI_PARAM Param2);

/**
* brief Send the data attached the message which is not allowed to loss.CNcomment:发送带附加数据不可丢失的异步消息
* param[in] hWidget Widget handle.CNcomment:窗口或控件
* param[in] MsgId  Messag ID. The msg id is defined in "hi_gv_widget.h"
* .CNcomment:消息ID，具体消息定义参考头文件hi_gv_widget.h \param[in] pBuf  Paramater 1,   the pointer of
* data.CNcomment:参数1，消息附带的数据指针 \param[in] BufLen  Paramater 2, The length of
* data.CNcomment:参数2，消息附带的数据长度
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_INVAL
* retval ::HI_ERR_COMM_PAERM Unsupport operation, such as, send key message.CNcomment:该操作不允许，比如发送按键消息
* see
* ::HI_GV_Msg_SendSync
* ::HI_GV_Msg_SendAsync
* ::HI_GV_Msg_PostAsync
*/
HI_S32 HI_GV_Msg_SendAsyncWithData(HIGV_HANDLE hWidget, HI_U32 MsgId, HI_VOID *pBuf, HI_PARAM BufLen);

/* * ==== API declaration end ==== */
#ifdef __cplusplus
}
#endif
#endif /* __HI_GV_MSG_H__ */
