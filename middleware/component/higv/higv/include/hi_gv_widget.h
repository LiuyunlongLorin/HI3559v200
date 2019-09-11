/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2009-2019. All rights reserved.
 * Description: Widget moudle API
 * Author: NULL
 * Create: 2009-2-9
 */

#ifndef __HI_GV_WIDGET_H__
#define __HI_GV_WIDGET_H__

#include "hi_gv_conf.h"
#include "hi_go.h"
#include "hi_gv_errno.h"
#include "hi_gv_resm.h"

#ifdef HIGV_USE_WIDGET_BUTTON
#include "hi_gv_button.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* ************************** Structure Definition *************************** */
/* * addtogroup      Widget   */
/* *  【Widget module】 */
/* * CNcomment:- 【控件框架】 */
/* *Widget type */
/* *CNcomment: 控件类型定义 */
typedef enum HI_WIDGET {
    HIGV_WIDGET_WINDOW = 0,
    /* <window CNcomment: 窗口控件 */
    HIGV_WIDGET_CONTAINER = 1,
    /* <containder  <CNcomment: 容器控件 */
    HIGV_WIDGET_BUTTON = 2,
    /* <button <CNcomment: Button控件 */
    HIGV_WIDGET_LABEL = 3,
    /* <label <CNcomment: Label控件 */
    HIGV_WIDGET_IMAGE = 4,
    /* <image CNcomment: Image控件 */
    HIGV_WIDGET_IMAGEEX = 5,
    /* <extend image <CNcomment: 扩展Image控件 */
    HIGV_WIDGET_GRID = 6,
    /* <grid <CNcomment: Grid控件 */
    HIGV_WIDGET_GROUPBOX = 7,
    /* <groupbox <CNcomment: Groupbox控件 */
    HIGV_WIDGET_SCROLLBAR = 8,
    /* <scrollbar <CNcomment: Scrollbar控件 */
    HIGV_WIDGET_EDIT = 9,
    /* <edit <CNcomment: Edit控件 */
    HIGV_WIDGET_SPINBOX = 10,
    /* <spin box *<CNcomment: Spinbox控件 */
    HIGV_WIDGET_PROGRESS = 11,
    /* <progressbar <CNcomment: Progressbar控件 */
    HIGV_WIDGET_SCROLLBOX = 12,
    /* <scrollbox <CNcomment: Scrollbox控件 */
    HIGV_WIDGET_SCROLLTEXT = 13,
    /* <scrolltext <CNcomment: Scrolltext控件 */
    HIGV_WIDGET_IMAGECIPHER = 14,
    /* <image cipher <CNcomment: Imagecipher控件 */
    HIGV_WIDGET_CLOCK = 15,
    /* <clock <CNcomment: Clock控件 */
    HIGV_WIDGET_LISTBOX = 16,
    /* <listbox <CNcomment: Listbox控件 */
    HIGV_WIDGET_TIMETRACK = 17,
    /* <timetrack <CNcomment: 时间轴控件 */
    HIGV_WIDGET_SCROLLGRID = 18,
    /* <scrollgrid <CNcomment: ScrollGrid控件 */
    HIGV_WIDGET_TRACKBAR = 19,
    /* <trackbar <CNcomment:TrackBar控件 */
    HIGV_WIDGET_IPEDIT = 20,
    /* <ipedit <CNcomment:IpEdit控件 */
    HIGV_WIDGET_CALENDAR = 21,
    /* <calendar <CNcomment: 日历控件 */
    HIGV_WIDGET_VIDEOSHELTER = 22,
    /* <videoshelter<CNcomment: 视频遮挡控件 */
    HIGV_WIDGET_MOVEMONITOR = 23,
    /* <movemonito <CNcomment: 移动监控控件 */
    HIGV_WIDGET_COMBOBOX = 24,
    /* <combobox <CNcomment:组合框控件 */
    HIGV_WIDGET_CPOPBOX = 25,
    /* <pop window of combobox <CNcomment: 多功能窗口 */
    HIGV_WIDGET_IMEWINDOW = 26,
    /* <input method window  <CNcomment: 输入法窗口 */
    HIGV_WIDGET_TIMEBAR = 27,
    /* <timebar  <CNcomment: 时间轴控件 */
    HIGV_WIDGET_MSGBOX = 28,
    /* <dialog box  CNcomment: 对话框控件 */
    HIGV_WIDGET_MULTIEDIT = 29,
    /* <multi edit <CNcomment: 多行编辑框控件 */
    HIGV_WIDGET_SCROLLVIEW = 30,
    /* <scroll view  <CNcomment: 滚动视图 */
    HIGV_WIDGET_CONTENTBOX = 31,
    /* <content box  <CNcomment: 内容盒子 */
    HIGV_WIDGET_SLIDEUNLOCK = 32,
    /* <slide unlock  CNcomment: 滑动解锁 */
    HIGV_WIDGET_WHEELVIEW = 33,
    /* <wheel view <CNcomment: 滚动选择器 */
    HIGV_WIDGET_IMAGEVIEW = 34,
    /* <image view <CNcomment: 大图片显示  */
    HIGV_WIDGET_BUTT
} HIGV_WIDGET_E;

/* *The sequence of user defined widget */ /* *CNcomment:用户自定义控件的ID开始 */
#define HIGV_EXTWIDGET_START HIGV_WIDGET_BUTT
/* *The callback of user defined widget  CNcomment:用户自定义控件创建回调函数 */
typedef HI_VOID *(*HIGV_WIDGET_INIT_FUNC)(HI_U32 style, const HI_RECT *pRect, HIGV_HANDLE hParent, HI_U32 level);

typedef HI_S32 (*HIGV_WIDGET_HDL2ADDR_PFN)(HI_U32 u32Idx, HI_U8 u8MemType, HI_PARAM *pAddr);

typedef struct hiHIGV_WIDGET_TYPEINFO_S {
    /* <Widget name <CNcomment:  控件名 */
    HI_CHAR Name[33];
    /* < The sequence of user defined widget ; CNcomment:  TypeID应从HIGV_EXTWIDGET_START开始编号 */
    HI_U32 TypeID;
    HIGV_WIDGET_INIT_FUNC WidgetInitFunc;
    /* <The callback of user defined widget <CNcomment:  用户自定义控件创建回调函数 */
    /* <The size of widget that pasered by parser mode, it is should seted
    if the widget is defined in xml, otherwise it can be set 0 */
    /* <CNcomment:  控件解析模块所定义结构的大小，如果该控件已在XML中解析创建则需要设置，否则可设为0 */
    HI_U32 WidgetPaserSize;
} HIGV_WIDGET_TYPEINFO_S;

/* *coordinate type  CNcomment:  HIGV 坐标类型 */
typedef HI_S32 HIGV_CORD;

/* *The button of mouse which pressed * CNcomment:  鼠标按钮按下状态定义 */
typedef enum {
    /* <CNcomment:  鼠标未按下 */
    HIGV_MOUSEBUTTON_NONE = 0x00,
    /* <CNcomment:  鼠标左键按下 */
    HIGV_MOUSEBUTTON_L = 0x01,
    /* <CNcomment:  鼠标中键按下 */
    HIGV_MOUSEBUTTON_M = 0x02,
    /* <CNcomment:  鼠标右键按下 */
    HIGV_MOUSEBUTTON_R = 0x04,
    HIGV_MOUSEBUTTON_BUTT
} HIGV_MOUSEBUTTON_E;

/* *The status of timer running  CNcomment: 控件定时器消息运行状态 */
typedef enum {
    HIGV_TIMERMSG_STATUS_UNEXIST = 0,
    /* <unexit status <CNcomment: 定时器不存在 */
    HIGV_TIMERMSG_STATUS_IDLE,
    /* <idle status <CNcomment: 定时器空闲状态 */
    HIGV_TIMERMSG_STATUS_FRYING,
    /* <frying status <CNcomment: 定时器正在处理 */
    HIGV_TIMERMSG_STATUS_RESET,
    /* <reset status <CNcomment: 定时器复位状态 */
    HIGV_TIMERMSG_STATUS_BUTT
} HIGV_TIMERMSG_STATUS_E;
/* *The language direction of widget  CNcomment: 强制控件语言方向 */
typedef enum {
    HIGV_TEXTDIR_NEUTRAL = 0,
    HIGV_TEXTDIR_LTR,
    HIGV_TEXTDIR_RTL,
    HIGV_TEXTDIR_BUTT
} HIGV_TEXTDIR_E;

/* *Set widget foucs switch mode  CNcomment: 焦点切换模式 */
typedef enum {
    HIGV_FOCUS_SWITCH_AUTO = 0,
    /* <auto CNcomment:焦点自动切换模式，不需要按ENTER键就可以切换焦点，一般按上下方向键切焦点 */
    HIGV_FOCUS_SWITCH_MANUAL,
    /* <manual CNcomment:焦点手动切换模式，需要按ENTER进入或退出控件的操作编辑状态 */
    HIGV_FOCUS_SWITCH_BUTT
} HIGV_FOCUS_SWITCH_E;

/* Focus switch state at manual focus mode  CNcomment: 手动焦点切换状态 */
typedef enum {
    HIGV_FOCUS_STATE_SWITCH = 0,
    /* <Focus switch state CNcomment:焦点切换状态 */
    HIGV_FOCUS_STATE_EDIT,
    /* <Widget can operate CNcomment:控件操作或编辑状态 */
    HIGV_FOCUS_STATE_BUTT
} HIGV_FOCUS_STATE_E;

/* The type of widget paint  CNcomment: 隐藏或显示 */
typedef enum {
    HIGV_PAINT_TYPE_HIDE = 0,
    /* hide widget  CNcomment:隐藏控件 */
    HIGV_PAINT_TYPE_SHOW,
    /* show widget  CNcomment:显示控件 */
    HIGV_PAINT_TYPE_BUTT
} HIGV_PAINT_TYPE_E;

/* * Window style   CNcomment: 窗口风格定义 */
/* * Default style, it is used 2bytes  CNcomment: 默认风格,占用风格最低两个字节 */
#define HIGV_STYLE_DEFAULT 0x00
/* *Don't draw the backgroud  CNcomment: 对控件有效，该控件不绘制背景 */
#define HIGV_STYLE_TRANSPARENT 0x02
/* *Couldn't get foucs，such Label  CNcomment:  不能获取焦点，例如Label */
#define HIGV_STYLE_NOFOCUS 0x04
/* *Release the resouce when hided  CNcomment: 当隐藏时，释放该控件或窗口占的资源 */
#define HIGV_STYLE_HIDE_UNLODRES 0x10
/* *It is indicated that the widget can change language, it can triger the langugage msg to user */
/* *CNcomment:表示该控件可以改变语言,主要是控制发送语言切换消息给用户 */
#define HIGV_STYLE_LANCHNG 0x20
/* *Modal windwo */
/* *CNcomment: 表示该窗口是模态的，模态窗口不不能被夺去焦点，除非该窗口关闭或隐藏 */
#define HIGV_STYLE_MODAL 0x40
/* *Foce draw backgroud of parent */
/* *CNcomment: 强制绘制父背景 */
#define HIGV_STYLE_FORCE_PAINT_PARENT 0x80
/* *if container is seted this flag, it's child also hightlighted,
and lost the focus, the child will not be hightlight */
/* *CNcomment: 对于容器类控件当置上这个标志时，当激活它时，它的子控件也同时被高亮，当
  * 失去焦点时，孩子失去高亮状态 */
#define HIGV_STYLE_HILIGHT_CHILD 0x100

/* *The extend style of widget which only support 8 */
/* *CNcomment: 对于控件扩展style 风格最多支持8种,占用风格字段最高2个byte */
#define HIGV_STYLE_DEFINE(WidgetType, style) ((((HI_U32)WidgetType) << 24) | (1 << (16 + (HI_U32)((style)&0x7))))

/* *Common style mask of widget */
/* *CNcomment: 控件通用风格掩码 */
#define HIGV_STYLE_MASK 0xffffff

/* *Skin index, it is only used to skin */
/* *CNcomment:  皮肤IDX定义，主要用于皮肤 */
/* *No focus */ /* *CNcomment:  无焦点 */
#define HIGV_SKIN_NORMAL 0x01
/* *Disable state */
/* *CNcomment:  未使能状态 */
#define HIGV_SKIN_DISABLE 0x02
/* *Hightlight state */
/* *CNcomment:  高亮状态 */
#define HIGV_SKIN_HIGHLIGHT 0x03
/* *Active state */
/* *CNcomment:  有焦点，激活 */
#define HIGV_SKIN_ACITVE 0x04
/* *Mouse down state */
/* *CNcomment: 鼠标按下，控件显示皮肤 */
#define HIGV_SKIN_MOUSEDOWN 0x05
/* *Checked state */
/* *CNcomment:  处于check状态的button在正常状态下的皮肤 */
#define HIGV_SKIN_BUTTON_CHECKED_NORMAL 0x06
/* *Disable at check state */
/* *CNcomment:  处于check状态的button在未使能状态下的皮肤 */
#define HIGV_SKIN_BUTTON_CHECKED_DISABLE 0x07
/* *Hightlight at check state */
/* *CNcomment:  处于check状态的button在高亮状态下的皮肤 */
#define HIGV_SKIN_BUTTON_CHECKED_HIGHLIGHT 0x08
/* *Focus at check state */
/* *CNcomment:  处于check状态的button在焦点状态下的皮肤 */
#define HIGV_SKIN_BUTTON_CHECKED_ACITVE 0x09
/* *Pressed at check state */
/* *CNcomment:  处于check状态的button在鼠标按下状态下的皮肤 */
#define HIGV_SKIN_BUTTON_CHECKED_MOUSEDOWN 0xa

/* *Window manager handle */
/* *CNcomment:  窗口管理句柄，由事件管理模块收到消息都发给窗口管理模块 */
#define HIGV_WND_MANAGER_HANDLE ((HI_U32)-1)

/* *It is used to HIGV_MSG_STATE_CHANGE message */
/* *CNcomment:  这个用于HIGV_MSG_STATE_CHANGE消息, wParam参数 */
typedef enum {
    HIGV_STATENAME_ENABLE,
    /* *< enable <CNcomment:  使能 */
    HIGV_STATENAME_HIGHLIGHT,
    /* *<highlight <CNcomment:  高亮 */
    HIGV_STATENAME_BUTT
} HIGV_STATENAME_E;
/* *The order of the message which user register to higv */
/* *CNcomment:标识同一个事件消息用户注册的事件函数与
* 控件原始的事件函数的处理先后顺序
*/
typedef enum {
    HIGV_PROCORDER_BEFORE,
    /* <It is first order to call user function */
    /* <CNcomment:  用户注册的事件函数先调用 */
    HIGV_PROCORDER_AFTER,
    /* <It is after order to call user function */
    /* <CNcomment:  用户注册的事件函数后调用 */
    HIGV_PROCORDER_BUTT
} HIGV_PROCORDER_E;

/* * 矩形与参数转换规则 */
#define HIGV_RECT_TOU32PARAM(rc, param1, param2)                                  \
    do {                                                                          \
        param1 = ((((HI_U32)(rc).x & 0xFFFF) << 16) | ((HI_U32)(rc).y & 0xFFFF)); \
        param2 = ((((HI_U32)(rc).w & 0xFFFF) << 16) | ((HI_U32)(rc).h & 0xFFFF)); \
    } while (0)

#define HIGV_U32PARAM_TORECT(param1, param2, rc)      \
    do {                                              \
       (rc).x = (HI_S32)(((param1) >> 16) & 0xFFFF); \
       (rc).y = (HI_S32)((param1)&0xFFFF);           \
       (rc).w = (HI_S32)(((param2) >> 16) & 0xFFFF); \
       (rc).h = (HI_S32)((param2)&0xFFFF);           \
    } while (0)

/* *Message of window CNcomment: 窗口消息定义 */
/* *Paint messsage [wparam, lparam] = [PaintRect<x,y>, PaintRect<w,h>] =
[((((rc).x & 0xFFFF)<<16) | ((rc).y & 0xFFFF)),
 *  ((((rc).w & 0xFFFF)<<16) | ((rc).h & 0xFFFF))] */
/* CNcomment:CNcomment:  绘制消息[wparam, lparam] = [PaintRect<x,y>, PaintRect<w,h>] =
[((((rc).x & 0xFFFF)<<16) | ((rc).y & 0xFFFF)), ((((rc).w & 0xFFFF)<<16) | ((rc).h & 0xFFFF))] */
#define HIGV_MSG_PAINT 0x03
/* *Key down message [wparam, lparam]= [HIGV_KEYCODE_E, nil] */
/* *CNcomment:  键按下消息 [wparam, lparam]=   [HIGV_KEYCODE_E, nil] */
#define HIGV_MSG_KEYDOWN 0x04
/* *Key up message [wparam, lparam]  = [HIGV_KEYCODE_E, nil]* */
/* *CNcomment:  键弹起消息 [wparam, lparam]  = [HIGV_KEYCODE_E, nil] */
#define HIGV_MSG_KEYUP 0x05
/* *Get focus message [wparam, lparam]  = [nil, nil] */
/* *CNcomment:  获得焦点消息 [wparam, lparam]  = [nil, nil] */
#define HIGV_MSG_GET_FOCUS 0x06
/* *Lost focus message [wparam, lparam]  = [nil, nil] */
/* *CNcomment:  失去焦点消息 [wparam, lparam]  = [nil, nil] */
#define HIGV_MSG_LOST_FOCUS 0x07
/* *Show message [wparam, lparam]  = [nil, nil] */
/* *CNcomment:  显示消息 [wparam, lparam]  = [nil, nil] */
#define HIGV_MSG_SHOW 0x08
/* *Hide message [wparam, lparam]  = [nil, nil] */
/* *CNcomment:  隐藏消息[wparam, lparam]  = [nil, nil] */
#define HIGV_MSG_HIDE 0x09
/* *Data change message [wparam, lparam]  = [nil, nil] */
/* *CNcomment:  数据改变消息[wparam, lparam]  = [nil, nil]  */
#define HIGV_MSG_DATA_CHANGE 0x10
/* *Timer message [wparam, lparam]  = [TimerID, nil]  */
/* *CNcomment:  定时消息 [wparam, lparam]  = [TimerID, nil]  */
#define HIGV_MSG_TIMER 0x11
/* *button down message  [wparam, lparam]  = [nil, nil] */
/* *CNcomment:  button按下消息 [wparam, lparam]  = [nil, nil]
                                                           */
#define HIGV_MSG_CLICK 0x12
/* *State change message */
/* *CNcomment:  状态改变消息 [wparam, lparam]  = [HIGV_STATENAME_E, HI_TRUE | HI_FALSE]  */
#define HIGV_MSG_STATE_CHANGE 0x13
/* *Item selected message */
/* *CNcomment:  条目选中消息 [wparam, lparam]  = [SelCell.Item, SelCell.Col]  */
#define HIGV_MSG_ITEM_SELECT 0x14
/* *Scroll text message */ /* *CNcomment:  滚动字幕 [wparam, lparam]  = [nil, nil]  */
#define HIGV_MSG_ST_UPDATE 0x15
/* *Update time message [wparam, lparam]  = [nil, nil] */
/* *CNcomment:  时钟控件需要更新时间 [wparam, lparam]  = [nil,nil]  */
#define HIGV_MSG_CLOCK_UPDATE 0x16
/* *Input over for imagecipher [wparam, lparam]  = [nil, nil] */
/* *CNcomment:  图片密码框输入结束[wparam, lparam]  =  [nil, nil]  */
#define HIGV_MSG_IC_INPUTEND 0x17
/* *Input over for edit [wparam, lparam]  = [nil, nil] */
/* *CNcomment:  密码风格输入结束 [wparam, lparam]  = [nil, nil]  */
#define HIGV_MSG_EDIT_INPUTEND 0x18
/* *Scroll over in one cycle for scrolltext  [wparam, lparam]  = [nil, nil] */
/* *CNcomment:  滚动字幕滚动一轮结束[wparam, lparam]  = [nil, nil] */
#define HIGV_MSG_ST_SCROLLONETIME 0x19
/* *The value changed when mouse opreate [wparam, lparam]  = [SlidePos, ContentLength] */
/* *CNcomment:        滚动条值因为鼠标操作发生改变[wparam,  lparam]  = [SlidePos,ContentLength] */
#define HIGV_MSG_SCROLLBAR_CHANGE 0x1a
/* *State change of checkbox or radio button [wparam, lparam]  = [HIGV_BUTTON_STATUS_E, nil] */
/* *CNcomment:  CheckBox或者Radio Button的状态改变
* [wparam, lparam]  =[HIGV_BUTTON_STATUS_E,nil] */
#define HIGV_MSG_BUTTON_STATUSCHANGE 0x1b
/* *Cell is selected [m_SelCell.Item,m_SelCell.Col] */
/* *CNcomment:  某一个cell被选中 [wparam, lparam]  =[m_SelCell.Item,m_SelCell.Col] */
#define HIGV_MSG_LISTBOX_CELLSEL 0x1c
/* *Item is selected [wparam, lparam]  =  [Item, nil] */
/* *CNcomment:  条目选中消息 [wparam, lparam]  =  [Item, nil] */
#define HIGV_MSG_COMBOBOX_ITEMSEL 0x1d
/* *Container widget change active child [wparam, lparam] = [Last active child handle, new active child handle] */
/* *CNcomment: 容器控件改变焦点孩子事件 [wparam, lparam] = [上一个焦点孩子句柄, 新焦点孩子句柄] */
#define HIGV_MSG_CONTAINER_CHANGE_FOCUSE 0x1e

/* *The event of scrollgrid */ /* *CNcomment:  ScrollGrid事件定义 */
/* *The cell selected event  [wparam, lparam]  = [SelCell.Item, SelCell.Col] */
/* *CNcomment:  ScrollGrid某一个cell被选中 [wparam, lparam]  = [SelCell.Item, SelCell.Col] */
#define HIGV_MSG_SCROLLGRID_CELL_COLSEL 0x23
/* *The focus move the top margin will genterate the event [wparam, lparam]  = [SelCell, nil] */
/* *CNcomment:  ScrollGrid的焦点移动到最上边缘后的事件 [wparam, lparam]  =SelCell, nil] */
#define HIGV_MSG_SCROLLGRID_REACH_TOP 0x24
/* *The focus move the bottom margin will genterate the event  [wparam, lparam]  = [SelCell, nil] */
/* *CNcomment:ScrollGrid 的焦点移动到最下边缘后的事件 [wparam, lparam] = [SelCell, nil] */
#define HIGV_MSG_SCROLLGRID_REACH_BTM 0x25
/* *The focus move the left margin will genterate the event  [wparam, lparam]  = [SelCell, nil]  */
/* *CNcomment:ScrollGrid 的焦点移动到最左边缘后的事件 [wparam, lparam]= [SelCell, nil] */
#define HIGV_MSG_SCROLLGRID_REACH_LEFT 0x26
/* *The focus move the right margin will genterate the event  [wparam, lparam]  = [SelCell, nil] */
/* *CNcomment:ScrollGrid    的焦点移动到最右边缘后的事件    [wparam, lparam]   */
#define HIGV_MSG_SCROLLGRID_REACH_RIGHT 0x27
/* *The update event of item for scrollgrid [wparam, lparam]  = [nil, nil] */
/* *CNcomment:  ScrollGrid 绘制条目更新事件 [wparam, lparam]  = [nil, nil]  */
#define HIGV_MSG_SCROLLGRID_ITEM_UPDATE 0x28
/* *The uddate event of scrollgrid [wparam, lparam]  = [nil, nil] */
/* *CNcomment:  ScrollGrid 绘制焦点框更新事件 [wparam, lparam]  = [nil, nil]  */
#define HIGV_MSG_SCROLLGRID_FOCUS_UPDATE 0x29
/* *The begin event of decode image for scrollgrid [wparam, lparam]  = [nil, nil] */
/* *CNcomment: ScrollGrid解码开始事件[wparam, lparam]  = [nil, nil]   */
#define HIGV_MSG_SCROLLGRID_DECIMG_BEGIN 0x2a
/* *The finish event of decode image for scrollgrid [wparam, lparam]  = [nil, nil] */
/* *CNcomment:ScrollGrid解码结束事件 [wparam,lparam]  = [nil, nil]  */
#define HIGV_MSG_SCROLLGRID_DECIMG_FINISH 0x2b
/* *The finish event of fling for scrollgrid [wparam, lparam]  = [nil, nil] */
/* *CNcomment:  ScrollGrid轻扫手势结束事件[wparam, lparam]  = [nil, nil]  */
#define HIGV_MSG_SCROLLGRID_FLING_FINISH 0x2c

/* *The event of timetrack */ /* *CNcomment:  时间轴事件定义 */
/* * The drag event of mouse for timetrack [wparam, lparam]  = [m_CurGraduation, nil]  */
/* *CNcomment:  TimeTrack通过鼠标开始拖动滑块事件[wparam, lparam]  =  [m_CurGraduation, nil]  */
#define HIGV_MSG_TIMETRACK_DRAGBEGIN 0x30
/* *The position change of timetrack  [wparam, lparam]  = [m_CurGraduation, nil]   */
/* *CNcomment:  TimeTrack滑块位置改变事件[wparam, lparam]  =[m_CurGraduation, nil]   */
#define HIGV_MSG_TIMETRACK_CHANGE 0x31
/* *The end event when mouse move through timetrack [wparam, lparam]  = [m_CurGraduation, nil] */
/* *CNcomment:  TimeTrack通过鼠标拖动滑块结束事件 [wparam, lparam]  = [m_CurGraduation,  nil]   */
#define HIGV_MSG_TIMETRACK_DRAGEND 0x32
/* *The graduation value change event when drag trackbar [wparam, lparam]  = [m_CurValue, nil]   */
/* *CNcomment: TrackBar拖动改变刻度产生消息 [wparam, lparam] = [m_CurValue,nil]   */
#define HIGV_MSG_VALUEONCHANGE 0x33

/* * The update event of scrollbox  [wparam, lparam]  = [nil, nil] */
/* *CNcomment:  SCROLLBOX绘制更新事件 [wparam,lparam]  = [nil, nil]  */
#define HIGV_MSG_SCROLLBOX_UPDATE 0x34

/* *The switch event of input method[wparam, lparam]  = [nil, nil] */
/* *CNcomment:  输入法切换事件 [wparam, lparam]  = [nil, nil]  */
#define HIGV_MSG_IME_SWITCH 0x35
/* *The show event of input method [wparam, lparam]  = [nil, nil] */
/* *CNcomment:  输入法相关窗口显示 [wparam, lparam]  = [nil, nil]  */
#define HIGV_MSG_IME_SHOW 0x36
/* *The hide event of input method [wparam, lparam]  = [nil, nil] */
/* *CNcomment:  输入法相关窗口隐藏[wparam, lparam]  = [nil, nil]   */
#define HIGV_MSG_IME_HIDE 0x37

/* *The event of timebar */
/* *CNcomment:  时间轴事件定义 */
/* *The drag start event of mouse for timebar  [wparam, lparam]  = [nil, nil] */
/* *CNcomment: Timebar通过鼠标开始拖动滑块事件 [wparam, lparam]  = [nil, nil]  */
#define HIGV_MSG_TIMEBAR_DRAGBEGIN 0x40
/* *The change event of timebar  [wparam, lparam]  = [GraduationIndex, nil] */
/* *CNcomment:  timebar滑块位置改变事件 [wparam, lparam]  = [GraduationIndex, nil]  */
#define HIGV_MSG_TIMEBAR_CHANGE 0x41
/* * The drag end event of mouse for timebar  [wparam, lparam]  = [nil, nil] */
/* *CNcomment: timebar通过鼠标拖动滑块结束事件 [wparam, lparam]  = [nil, nil] */
#define HIGV_MSG_TIMEBAR_DRAGEND 0x42

/* * Change the foucs of clock [wparam, lparam]  =[m_FocusSeg, 0] */
/* *CNcomment:  clock焦点修改 [wparam, lparam]  = [m_FocusSeg, 0] */
#define HIGV_MSG_CLOCK_FOCUS_MOVE 0x50
/* *Set clock. [wparam, lparam]  =  [nil, nil] */
/* *CNcomment:  修改 clock时间[wparam, lparam]  =  [Item, nil] */
#define HIGV_MSG_CLOCK_TIME_ADJUST 0x51
/* *Set widget sate. [wparam, lparam]  =  [nil, nil] */
/* *CNcomment:  控件手动焦点模式状态改变[wparam, lparam]  = [HIGV_FOCUS_STATE_E, nil] */
#define HIGV_MSG_MANUAL_FOCUS_STATE_CHANGE 0x52

/* *The event of message box */ /* *CNcomment:  消息框事件定义 */
/* *The timeout event of MessageBox show  [wparam, lparam]  = [ButtonID, show times] */
/* *CNcomment:   MessageBox显示时间结束自动隐藏事件 [wparam, lparam]  = [ButtonID, show times]  */
#define HIGV_MSG_MSGBOX_TIMEOUT 0x60
/* *The message box click button message [wparam, lparam]  = [ButtonID, nil] */
/* *CNcomment:  MessageBox选定按钮消息 [wparam, lparam]  = [ButtonID, nil] */
#define HIGV_MSG_MSGBOX_SELECT 0x61
/* *The message box hide message. attention:Don't use it */
/* *CNcomment:  MessageBox选定隐藏消息。注意请勿使用 */
#define HIGV_MSG_MSGBOX_HIDE 0x62

/* *The event of multi edit */
/* *CNcomment:  多行编辑框事件定义 */
/* *Input over for multi edit [wparam, lparam]  = [nil, nil] */
/* *CNcomment:  多行编辑框输入事件 [wparam, lparam]  = [nil, nil] */
#define HIGV_MSG_MULTIEDIT_INPUT 0x70
/* *Begin select string from text [wparam, lparam]  = [nil, nil] */
/* *CNcomment:  鼠标在控件外选取字串事件 [wparam,  lparam]  = [nil, nil] */
#define HIGV_MSG_MULTIEDIT_SELECT_OUT 0x71
/* *End select string from text [wparam, lparam]  = [nil, nil] */
/* *CNcomment:  从文中选取字串结束事件 [wparam, lparam] = [nil, nil] */
#define HIGV_MSG_MULTIEDIT_SELECT_END 0x72
/* *Move reach side [wparam, lparam]  = [HIGV_MULTIEDIT_SIDE_E, nil] */
/* *CNcomment:  移动到达边界 [wparam, lparam]  = [HIGV_MULTIEDIT_SIDE_E, nil] */
#define HIGV_MSG_MULTIEDIT_REACH_SIDE 0x73
/* *Draw cursor [wparam, lparam]  = [nil, nil] */
/* *CNcomment:  绘制光标消息 [wparam, lparam]  = [nil, nil] */
#define HIGV_MSG_MULTIEDIT_DRAW_CURSOR 0x74

/* *The event of scroll view */ /* *CNcomment: 滚动视图事件定义 */
/* *Scroll view scroll event [wparam, lparam] =
[(((content rect).x & 0xFFFF)<<16) | (y & 0xFFFF), ((w & 0xFFFF)<<16) |
 *  (h & 0xFFFF)] */
/* CNcomment: 视口移动事件 [wparam, lparam]  =
[(((content rect).x & 0xFFFF)<<16) | (y & 0xFFFF), ((w & 0xFFFF)<<16) | (h & 0xFFFF)] */
#define HIGV_MSG_SCROLLVIEW_SCROLL 0x80
/* *The event of finish fling event */ /* *CNcomment: 结束轻扫事件定义 */
/* *finish fling event [wparam, lparam] = [(((content rect).x & 0xFFFF)<<16) | (y & 0xFFFF), ((w & 0xFFFF)<<16) | (h &
 * 0xFFFF)] */
/* CNcomment: 轻扫结束事件 [wparam, lparam]  =
[(((content rect).x & 0xFFFF)<<16) | (y & 0xFFFF), ((w & 0xFFFF)<<16) | (h & 0xFFFF)] */
#define HIGV_MSG_SCROLLVIEW_FINISHFLING 0x81

/* *Language change event */
/* *CNcomment:  语言改变事件 [wparam, lparam]  = [LanguageID, LanguageDirection] */
#define HIGV_MSG_LAN_CHANGE 0x100
/* *Exit edit status at auto manual focus mode [wparam, lparam]  = [nil, nil] */
/* *CNcomment: 手动切换焦点模式下退出编辑状态[wparam, lparam]  = [nil, nil] */
#define HIGV_MSG_FOCUS_EDIT_EXIT 0x101
/* *Mouse in event [wparam, lparam]  = [nil, nil] */
/* *CNcomment: 鼠标移入事件[wparam, lparam]  = [nil, nil] */
#define HIGV_MSG_MOUSEIN 0x200
/* *The first event of mouse */ /* *CNcomment: 第一个鼠标事件 */
#define HIGV_MSG_FIRSTMOUSEMSG HIGV_MSG_MOUSEIN
/* *Mouse down event [wparam, lparam]  = [HIGV_MOUSEBUTTON_L, (ScreenX << 16)|(ScreenY & 0xffff)] */
/* *CNcomment: 鼠标按下事件[wparam,   lparam]  =  [HIGV_MOUSEBUTTON_L, (ScreenX << 16)|(ScreenY & 0xffff)] */
#define HIGV_MSG_MOUSEDOWN 0x201
/* *Mouse out event [wparam, lparam]  = [nil, nil] */
/* *CNcomment: 鼠标移出事件[wparam, lparam]  = [nil, nil] */
#define HIGV_MSG_MOUSEOUT 0x202
/* *Mouse up event [wparam, lparam]  = [HigvMouseState, (ScreenX << 16)|(ScreenY & 0xffff)] */
/* *CNcomment:  鼠标抬起事件[wparam, lparam]  =  [HigvMouseState,(ScreenX << 16)|(ScreenY & 0xffff)] */
#define HIGV_MSG_MOUSEUP 0x203
/* *Mouse move event [wparam, lparam]  = [nil, (ScreenX << 16)|(ScreenY & 0xffff)] */
/* *CNcomment: 鼠标移动事件[wparam, lparam]  = [nil, (ScreenX <<  16)|(ScreenY & 0xffff)] */
#define HIGV_MSG_MOUSEMOVE 0x204
/* *Mouse wheel event [wparam, lparam]  = [nil, nil] */
/* *CNcomment: 鼠标滚轮事件[wparam, lparam]  = [nil, nil] */
#define HIGV_MSG_MOUSEWHEEL 0x205
/* *The press down of left mouse [wparam, lparam]  = [nil, nil] */
/* * 鼠标左键按下事件 [wparam, lparam]  = [nil, nil] */
#define HIGV_MSG_MOUSELONGDOWN 0x206
/* *The double click of  mouse [wparam, lparam]  = [nil, nil] */
/* *鼠标双击事件[wparam, lparam]  = [nil, nil] */
#define HIGV_MSG_MOUSEDBCLICK 0x207
/* *The last message of  mouse */
/* *CNcomment: 最后一个鼠标事件 */
#define HIGV_MSG_LASTMOUSEMSG HIGV_MSG_MOUSEDBCLICK
/* *The virtual mouse up event [wparam, lparam]  = [nil, nil] */
/* *CNcomment: 虚拟鼠标弹起事件[wparam, lparam]  = [nil,  nil] */
#define HIGV_MSG_VIRTUAL_MOUSEUP 0x208
/* *The virtual mouse down event [wparam, lparam]  = [nil, nil] */
/* *CNcomment: 虚拟鼠标按下事件[wparam, lparam]  = [nil,  nil] */
#define HIGV_MSG_VIRTUAL_MOUSEDOWN 0x209

/* *Refesh the window to screen [wparam, lparam]  =
[(((rc).x & 0xFFFF)<<16) | ((rc).y & 0xFFFF), (((rc).w & 0xFFFF)<<16)
 * | ((rc).h & 0xFFFF)] */
/* CNcomment:  将窗口刷新到屏幕上[wparam, lparam]  =
[(((rc).x & 0xFFFF)<<16) | ((rc).y & 0xFFFF), (((rc).w & 0xFFFF)<<16) | ((rc).h & 0xFFFF)] */
#define HIGV_MSG_REFRESH_WINDOW 0x500
/* *After refesh the window to screen [wparam, lparam]  = [nil, nil] */
/* *CNcomment:  将窗口刷新到屏幕上[wparam, lparam] = [nil, nil] */
#define HIGV_MSG_AFTERREFRESH 0x501
/* *The data change messag for adm */
/* *CNcomment:  ADM模块特有的消息 [wparam, lparam]  = [HIGV_DC_E, nil] */
#define HIGV_MSG_ADM_DATACHANGE 0x502
/* *Force to update the window to the screen */
/* *CNcomment:  强制更新并将窗口刷新到屏幕上[wparam, lparam]  = [(((rc).x &
*  0xFFFF)<<16) | ((rc).y & 0xFFFF), (((rc).w & 0xFFFF)<<16) | ((rc).h &
* 　0xFFFF)] */
#define HIGV_MSG_FORCE_REFRESH_WINDOW 0x503

/* *Refresh message which is used to avoid loss combined message [wparam, lparam]  = [nil, nil] */
/* *CNcomment:刷新消息为避免合并丢失刷新事件而产生的消息[wparam,    nil] */
#define HIGV_MSG_FAKE_REFRESH_WINDOW 0x504

/* *Bussiness event [wparam, lparam]  = [nil, nil] */
/* *CNcomment: 业务事件[wparam, lparam]  = [nil, nil] */
#define HIGV_MSG_EVENT 0x900
/* *The base value of user defined [wparam, lparam]  = [nil, nil] */
/* *CNcomment: 用户自定义事件起始值[wparam, lparam]  =  [nil, nil] */
#define HIGV_MSG_USER 0x1000
/* *The mouse in for viedo shelter [wparam, lparam]  = [RectIndex, HIGV_VIDEOSHELTER_CURSOR_UNDER_E] */
/* *CNcomment:  鼠标移入事件[wparam, lparam]  = [RectIndex,  HIGV_VIDEOSHELTER_CURSOR_UNDER_E] */
#define HIGV_MSG_VIDEOSHELTER_MOUSEIN 0x1001
/* *The down event of mouse for video shelter  [wparam, lparam]  = [RectIndex, HIGV_VIDEOSHELTER_CURSOR_UNDER_E] */
/* *CNcomment:  鼠标左键键单击事件 [wparam, lparam] = [RectIndex, HIGV_VIDEOSHELTER_CURSOR_UNDER_E]   */
#define HIGV_MSG_VIDEOSHELTER_MOUSEDOWN_L 0x1002
/* *The up event of mouse for video shelter [wparam, lparam]  = [RectIndex, HIGV_VIDEOSHELTER_CURSOR_UNDER_E] */
/* *CNcomment:  鼠标弹起事件[wparam,  lparam] = [RectIndex, HIGV_VIDEOSHELTER_CURSOR_UNDER_E] */
#define HIGV_MSG_VIDEOSHELTER_MOUSEUP 0x1003
/* *The move event of mouse for viedo shelter  [wparam, lparam]  = [RectIndex, HIGV_VIDEOSHELTER_CURSOR_UNDER_E] */
/* *CNcomment:  鼠标移动事件 [wparam,  lparam]  =  [RectIndex,  HIGV_VIDEOSHELTER_CURSOR_UNDER_E]  */
#define HIGV_MSG_VIDEOSHELTER_MOUSEMOVE 0x1004
/* *The  click event of right mouse for viedo shelter [wparam, lparam]  = [RectIndex, nil] */
/* *CNcomment:  鼠标右键单击事件[wparam,lparam]  = [RectIndex,nil]  */
#define HIGV_MSG_VIDEOSHELTER_MOUSEDOWN_R 0x1005
/* *The up event of mouse for move monitor [wparam, lparam]  = [HIGV_VIDEOSHELTER_CURSOR_UNDER_E, nil]  */
/* *CNcomment:  移动侦测鼠标弹起事件[wparam, lparam]  = [HIGV_VIDEOSHELTER_CURSOR_UNDER_E, nil]  */
#define HIGV_MSG_MOVEMONITOR_MOUSEUP 0x1006
/* *touch screen event [wparam, lparam]  = [nil, nil] */
/* *CNcomment:  触摸事件[wparam, lparam]  = [nil, nil]   */
#define HIGV_MSG_TOUCH 0x2001
/* *tap gesture event[wparam, lparam]  = [nil, nil] */
/* *CNcomment:  轻敲触摸屏手势事件[wparam, lparam]  = [nil, nil] */
#define HIGV_MSG_GESTURE_TAP 0x2002
/* *long press gesture event [wparam, lparam]  = [nil, nil] */
/* *CNcomment:  长按手势事件[wparam, lparam]  = [nil, nil]  */
#define HIGV_MSG_GESTURE_LONGTAP 0x2003
/* *fling gesture event [wparam, lparam]  = [nil, nil] */
/* *CNcomment:  轻抚触摸屏手势事件[wparam, lparam]  = [nil, nil]
 */
#define HIGV_MSG_GESTURE_FLING 0x2004
/* *scroll gesture event [wparam, lparam]  = [nil, nil] */
/* *CNcomment:  滑动手势事件[wparam, lparam]  = [nil, nil]   */
#define HIGV_MSG_GESTURE_SCROLL 0x2005
/* *pinch gesture event [wparam, lparam]  = [nil, nil] */
/* *CNcomment:  捏合手势事件[wparam, lparam]  = [nil, nil]   */
#define HIGV_MSG_GESTURE_PINCH 0x2006
/* *unlock done event [wparam, lparam]  = [nil, nil] */
/* *CNcomment:  解锁完成事件[wparam, lparam]  = [nil, nil]   */
#define HIGV_MSG_UNLOCK 0x2007
/* *finish seek trackbar [wparam, lparam]  = [m_CurValue, nil]   */
/* *CNcomment:  TrackBar拖动结束上报事件 [wparam,  lparam]  = [m_CurValue, nil]   */
#define HIGV_MSG_FINISHSEEK 0x2008
/* *move event [wparam, lparam]  = [nil, nil] */
/* *CNcomment:  滑动解锁控件移动事件[wparam, lparam]  = [nil, nil]   */
#define HIGV_MSG_MOVE 0x2009
/* *kick back event [wparam, lparam]  = [nil, nil] */
/* *CNcomment:  滑动解锁回弹完成事件[wparam, lparam]  = [nil, nil] */
#define HIGV_MSG_KICKBACK 0x2010
/* *clock position event [wparam, lparam]  = [nil, nil] */
/* *CNcomment:  时钟位置上报事件[wparam, lparam]  = [nil, nil] */
#define HIGV_MSG_POSITION 0x2011
/* *wheelview position event [wparam, lparam]  = [nil, nil] */
/* *CNcomment:  滚动选择器焦点位置上报事件[wparam, lparam]  = [nil, nil]   */
#define HIGV_MSG_WHEELVIEW_ITEM 0x3001
/* *imageview git finish repeat event [wparam, lparam]  = [nil, nil] */
/* *CNcomment: gif动画循环播放结束上报事件[wparam,   lparam]  = [nil, nil]   */
#define HIGV_MSG_IMAGEEX_ENDOF_REPEAT 0x3101

/* *animation start event [wparam, lparam]   = [nil, nil] */
/* *CNcomment:  动画开始事件[wparam, lparam]  = [nil, nil]wparam 组件句柄, lparam 动画句柄 */
#define HIGV_MSG_ANIM_START 0x5001
/* *animation running event [wparam, lparam] = [nil, nil] */
/* *CNcomment:  动画进行事件[wparam, lparam]  = [nil, nil]  wparam 组件句柄, lparam 动画句柄 */
#define HIGV_MSG_ANIM_UPDATE 0x5002
/* *animation stop event [wparam, lparam]    = [nil, nil] */
/* *CNcomment:  动画结束事件[wparam, lparam]  = [nil, nil]　wparam 组件句柄, lparam 动画句柄 */
#define HIGV_MSG_ANIM_STOP 0x5003

/* *animation start event [wparam, lparam]   = [nil, nil] */
/* *CNcomment:  回弹开始事件[wparam, lparam]  = [nil, nil] wparam 组件句柄, lparam 为空 */
#define HIGV_MSG_REBOUND_START 0x5101
/* *animation running event [wparam, lparam] = [nil, nil] */
/* *CNcomment:  回弹进行事件[wparam, lparam]  = [nil, nil]   wparam 组件句柄, lparam 移动距离 */
#define HIGV_MSG_REBOUND_UPDATE 0x5102
/* *animation stop event [wparam, lparam]    = [nil, nil] */
/* *CNcomment:  回弹结束事件[wparam, lparam]  = [nil, nil]  wparam 组件句柄, lparam 为空  */
#define HIGV_MSG_REBOUND_STOP 0x5103

/* *it is defined the message whether process continue */
/* *CNcomment:定义消息是否继继续向下走 */
/* *The message will go on */ /* *CNcomment: 消息继续往下传递 */
#define HIGV_PROC_GOON 100
/* *Stop process message, it is not continue */
/* *CNcomment: 消息停止，不再往下传递 */
#define HIGV_PROC_STOP 200
/* *Get the x coordiate of mouse which is relative the x coordiate of widget */
/* *CNcomment:  取得鼠标的坐标，该坐标系统相对于控件的X坐标  */
#define HIGV_GET_MOUSE_X(lParam) ((HIGV_CORD)((HI_S16)(((HI_U32)lParam) >> 16)))
/* *Get the y coordiate of mouse which is relative the y coordiate of widget */
/* *CNcomment:    取得鼠标的坐标，该坐标系统相对于控件的Y坐标 */
#define HIGV_GET_MOUSE_Y(lParam) ((HIGV_CORD)((HI_S16)(((HI_U32)lParam) & 0xFFFF)))
/* *Get step of mouse moving */ /* *CNcomment: 获取鼠标步进值 */
#define HIGV_GET_MOUSE_STEP(wParam) ((HI_S32)((HI_S16)(((HI_U32)wParam) >> 16)))
/* *Get state of mouse button */ /* *CNcomment: 获取鼠标按键状态 */
#define HIGV_GET_MOUSE_BUTTON(wParam) ((HI_U32)((HI_S16)(((HI_U32)wParam) & 0xFFFF)))

/* *Mouse wheel direction */ /* *CNcomment: 鼠标滚轮滚动方向 */
typedef enum hiHIGV_WHEEL_DIRECTION {
    HIGV_WHEEL_DIRECTION_DOWN = 0,
    HIGV_WHEEL_DIRECTION_UP = 1,
    HIGV_WHEEL_DIRECTION_BUTT
} HIGV_WHEEL_DIRECTION;

/* * Get step and diection of mouse wheel */
/* *CNcomment: 获取鼠标滚轮滚动的方向和步长 */
#define HIGV_GET_MOUSE_WHEEL(wParam, Step, Direction) \
    do {                                              \
        Step = HIGV_GET_MOUSE_STEP(wParam);           \
        if (Step >> 7) {                              \
            Step = 0xFF - Step + 1;                   \
            Direction = HIGV_WHEEL_DIRECTION_UP;      \
        } else {                                      \
            Direction = HIGV_WHEEL_DIRECTION_DOWN;    \
        }                                             \
    } while (0)

/* *Message callback */ /* *CNcomment:  消息处理回调函数 */
typedef HI_S32 (*HIGV_MSG_PROC)(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam);

/* * The attribute of widget to use create 　 CNcomment:控件创建属性 */
typedef struct hiHIGV_WCREATE_S {
    HI_U32 type; /* <Control class  CNcomment:  控件类型 */
    HI_RECT rect;                    /* <The position and size of widget　<CNcomment:  控件位置及大小 */
    HIGV_HANDLE hParent;             /* <Parent widget handle　<CNcomment:  父控件句柄 */
    HI_U32 style;                    /* <Widget style　<CNcomment:  控件风格 */
    HI_U32 level;
    /* <Window level, it is only effect to window, the range is [0,15]
    　<CNcomment:  窗口层次，只针对窗口控件有效，范围为[0,15] */
    HI_VOID *pPrivate; /* <The private data of widget　<CNcomment:  各Widget创建需要的私有数据 */
} HIGV_WCREATE_S;

/* *The private data of widnow *　CNcomment:  窗口创建私有数据结构 */
typedef struct hiHIGV_WINCREATE_S {
    HIGV_HANDLE hLayer;    /* <Layer of window　<CNcomment:  窗口所属图层 */
    HIGO_PF_E PixelFormat; /* <Pixel format of window　CNcomment:  窗口的像素格式 */
    HI_BOOL bVideoHole;    /* <For video hole　<CNcomment: 是否作为视频窗口 */
} HIGV_WINCREATE_S;

/* *The build tree of widget　CNcomment:  控件构造树(这里使用二叉树表示)的节点 */
typedef struct hiHIGV_WIDGETNODE_S {
    struct hiHIGV_WIDGETNODE_S *pChildHead;  /* <first child node　 CNcomment: 构造节点孩子的节点 */
    struct hiHIGV_WIDGETNODE_S *pNextSibing; /* <sibling node　 CNcomment: 构造节点兄弟节点 */
    HI_VOID *pWidgetInfo;
    /* <The build parameter when create the widget
    　CNcomment: 创建控件控件需要的构造参数,具体控件结构体信息参考higv_widgetstruct.h */
    HI_BOOL bCreateHide;   /* <Wether hide the widget when create　CNcomment: 控件创建时是否隐藏 */
    HI_VOID *pPrivateData; /* <User private data　CNcomment: 用户自定义私有数据 */
} HIGV_WIDGETNODE_S;

/* * The type of text alignment 　CNcomment: 文字对齐方式 */
/* *Horizontal left of text alignment 　CNcomment:  水平居左 */
#define HIGV_ALIGN_LEFT LAYOUT_LEFT
/* *Horizontal right of text alignment 　CNcomment:  水平居右 */
#define HIGV_ALIGN_RIGHT LAYOUT_RIGHT
/* *Horizontal center of text alignment 　CNcomment:  水平居中 */
#define HIGV_ALIGN_HCENTER LAYOUT_HCENTER
/* *Top style of text alignment 　CNcomment:  垂直居顶 */
#define HIGV_ALIGN_TOP LAYOUT_TOP
/* *Vertical bottom style  of text alignment 　CNcomment:  垂直居底 */
#define HIGV_ALIGN_BOTTOM LAYOUT_BOTTOM
/* *Vertical center style  of text alignment 　CNcomment:  垂直居中 */
#define HIGV_ALIGN_VCENTER LAYOUT_VCENTER
/* * Wap style  of text alignment　CNcomment:  自动换行 */
#define HIGV_ALIGN_WRAP LAYOUT_WRAP

/* *Ellipsis style of text alignment  CNcomment:  文本超出长度后加声省略号 */
#define HIGV_ALIGN_WORDELLIPSIS LAYOUT_WORDELLIPSIS
/* *Force to left of Horizontal  CNcomment:  强制水平居左 */
#define HIGV_ALIGN_FORCE_LEFT LAYOUT_FORCE_LEFT
/* *Force to right of Horizontal  CNcomment:  强制水平居右 */
#define HIGV_ALIGN_FORCE_RIGHT LAYOUT_FORCE_RIGHT
/* *Support the type of input method  CNcomment:  支持的输入法类型 */
/* *Support all input method by default CNcomment:  默认支持所有 */
#define HIGV_IMETYPE_ALLTYPE 0
/* *Lowcase english input method  CNcomment:  小写英文 */
#define HIGV_IMETYPE_ENGLISH (1 << 0)
/* * Caps english input method  CNcomment:   大写英文 */
#define HIGV_IMETYPE_CAPENGLISH (1 << 1)
/* *Number input method  CNcomment:  数字 */
#define HIGV_IMETYPE_NUMBER (1 << 2)
/* *Pingyin input method  CNcomment:  拼音 */
#define HIGV_IMETYPE_PINYIN (1 << 3)
/* * Symbol method   CNcomment: 符号 */
#define HIGV_IMETYPE_SYMBOL (1 << 4)
/* * Standard input metod A  CNcomment: 标准输入法A */
#define HIGV_IMETYPE_STANDARD_A (1 << 5)
/* * Standard input method B CNcomment: 标准输入法B */
#define HIGV_IMETYPE_STANDARD_B (1 << 6)

/* *  ==== Structure Definition end ==== */
/* ****************************** API declaration **************************** */
/* * addtogroup      Widget   */
/* * 【Widget moudle】 */
/* * 【控件框架】 */
/**
* brief Active widget.CNcomment: 设置控件焦点为活动的
* param[in] hWidget Widget handle.CNcomment: 控件句柄
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_INVAL
* retval ::HI_FAILURE CNcomment: 控件父可能不是活动的，导致失败
*/
extern HI_S32 HI_GV_Widget_Active(HIGV_HANDLE hWidget);

/**
* brief Create widget.CNcomment: 创建窗口和控件
* param[in] pCreatInfo The information of widget.CNcomment:  控件信息
* param[out] pWidget   Widget handle.CNcomment:  控件句柄指针
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_INVAL
* retval ::HI_ERR_COMM_NORES
* see :HI_GV_Widget_Destroy
*/
HI_S32 HI_GV_Widget_Create(const HIGV_WCREATE_S *pCreatInfo, HIGV_HANDLE *pWidget);

/*
* brief Destroy widget.CNcomment: 销毁窗口
* param[in] hWidget Widget handle.CNcomment: 控件句柄
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_NORES
* see:HI_GV_Widget_Create
*/
HI_S32 HI_GV_Widget_Destroy(HIGV_HANDLE hWidget);

/*
* brief Enable widget, but it is not effect to window.CNcomment: 使能控件，对于窗口无效
* param[in] hWidget Widget handle.CNcomment: 控件句柄
* param[in] bEnable  Enable flag.CNcomment: 使能标志，HI_TRUE表示使能，HI_FALSE表示去使能
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_NOOP Unsupport operation, such as the operation is to window.CNcomment:
* 不支持该窗口，比如对窗口进行该操作 \retval ::HI_ERR_COMM_PAERM Not allow, such as,
* it is not allowed when the parent
* window is disabled. CNcomment: 不允许，例如父窗口是禁止的，执行enable
*
*/
HI_S32 HI_GV_Widget_Enable(HIGV_HANDLE hWidget, HI_BOOL bEnable);

/*
* brief Get the active child widget.CNcomment: 获取活动子窗口
* attention :You can get the active window if hParent = HIGV_WND_MANAGER_HANDLE.
* CNcomment:当hParent = HIGV_WND_MANAGER_HANDLE时可以获取active窗口
* param[in] hParent  Parent widget handle.CNcomment: 父控件句柄
* param[out] pActive The pointer of active childe widget.CNcomment: 活动子控件指针
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST Invalid parater, such as the parent
* doesn't has active child or widget is invalid.CNcomment:
* 控件句柄无效或者父窗口没有活动孩子 \retval ::HI_ERR_COMM_PAERM No support operation, parent window is not a
* container.CNcomment: 不支持该操作，父窗口并不是个容器
* see:HI_GV_Widget_SetActiveWidget
*/
HI_S32 HI_GV_Widget_GetActiveWidget(HIGV_HANDLE hParent, HIGV_HANDLE *pActive);

/*
* brief Get font.CNcomment: 获取字体
* param[in] hWidget Widget handle.CNcomment: 控件句柄
* param[out] phFont Pointer of font.CNcomment: 字体对象指针
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_INVAL
*/
HI_S32 HI_GV_Widget_GetFont(HIGV_HANDLE hWidget, HIGV_HANDLE *phFont);

/*
* brief Get parent widget, if it is window ,it will return NULL.CNcomment: 获取控件父控件，如果是主窗口，则父为NULL
* param[in] hWidget Widget handle.CNcomment: 控件句柄
* param[out] phParent Ponter of parent widget handle.CNcomment: 父控件句柄指针
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_INVAL
* retval ::HI_FAILURE Failure, such as, no parent. CNcomment: 没有父
*/
HI_S32 HI_GV_Widget_GetParent(HIGV_HANDLE hWidget, HIGV_HANDLE *phParent);

/*
* brief Get sibings of widget, it is mainly used to change focus.CNcomment: 获取该控件左右上下兄弟，主要用于焦点切换
* param[in] hWidget Widget handle.CNcomment: 控件句柄
* param[out] pLeft  Left sibing widget. CNcomment: 左邻居控件句柄指针
* param[out] pRight Right sibing widget.CNcomment: 右邻居控件句柄指针
* param[out] pUp    Up sibing widget.CNcomment: 上邻居控件句柄指针
* param[out] pDown  Down sibing widget.CNcomment: 下邻居控件句柄指针
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_INVAL
*/
HI_S32 HI_GV_Widget_GetSiblings(HIGV_HANDLE hWidget, HIGV_HANDLE *pLeft, HIGV_HANDLE *pRight, HIGV_HANDLE *pUp,
                                HIGV_HANDLE *pDown);

/*
* brief Hide widget.CNcomment: 隐藏控件
* param[in] hWidget Widget handle.CNcomment: 控件句柄
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_INVAL
*/
HI_S32 HI_GV_Widget_Hide(HIGV_HANDLE hWidget);

/*
* brief Get the widget active state.CNcomment: 判断控件是否活动
* param[in] Widget handle.CNcomment: hWidget 控件句柄
* retval ::HI_TRUE  The widget is active.CNcomment: 控件是活动控件
* retval ::HI_FALSE The widget is inactive.CNcomment: 控件不是活动控件
*/
HI_BOOL HI_GV_Widget_IsActive(HIGV_HANDLE hWidget);

/*
* brief Get the widget enabel state.CNcomment: 控件使能状态，对于窗口无效
* param[in] hWidget Widget handle.CNcomment: 控件句柄
* retval ::HI_TRUE  Enable state.CNcomment: 使能状态
* retval ::HI_FALSE Disable state. CNcomment: 禁用状态
*/
HI_BOOL HI_GV_Widget_IsEnable(HIGV_HANDLE hWidget);

/*
* brief The widget whether show.CNcomment: 判断控件是否显示
* param[in] hWidget  Widget handle.CNcomment: 控件句柄
* retval ::HI_TRUE  Show state.CNcomment: 显示状态
* retval ::HI_FALSE  Hide state.CNcomment: 隐藏状态
*/
HI_BOOL HI_GV_Widget_IsShow(HIGV_HANDLE hWidget);

/*
* brief Get widget surface.CNcomment: 取得控件的surface
* param[in] hWidget Widget handle.CNcomment: 控件句柄
* param[out] phSurface Surface pointerCNcomment: 句柄指针
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
*/
HI_S32 HI_GV_Widget_GetSurface(HIGV_HANDLE hWidget, HIGV_HANDLE *phSurface);

/*
* brief Paint the widget, it will emmit a asynchronism message.CNcomment: 异步绘制该控件，产生HIGV_MSG_PAINT消息
* param[in] hWidget Widget handle.CNcomment: 控件句柄
* param[in] pRect The area of update . CNcomment: 绘制更新的区域
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_FAILURE Failure such as the widget is not show.CNcomment: 可能控件并不是show的状态
*/
HI_S32 HI_GV_Widget_Paint(HIGV_HANDLE hWidget, const HI_RECT *pRect);

/*
* brief Set font.CNcomment: 设置字体
* param[in] hWidget Widget handle.CNcomment: 控件句柄
* param[in] hFont The font of widget. CNcomment:  控件字体对象
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
*/
HI_S32 HI_GV_Widget_SetFont(HIGV_HANDLE hWidget, HIGV_HANDLE hFont);

/*
* brief Register message processor.CNcomment: 注册消息处理函数
* param[in] hWidget   Widget handle. CNcomment: 控件句柄
* param[in] Msg        Message. CNcomment: 消息
* param[in] CustomProc Message processor.CNcomment: 消息处理函数
* param[in] ProcOrder  Message prioprity.CNcomment: 消息处理优先级
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_NORES
*/
HI_S32 HI_GV_Widget_SetMsgProc(HIGV_HANDLE hWidget, HI_U32 Msg, HIGV_MSG_PROC CustomProc,
                               HIGV_PROCORDER_E ProcOrder);

/*
* brief Set skin of widget.CNcomment: 设置控件皮肤
* param[in] hWidget   Widget handle. CNcomment: 控件句柄
* param[in] hSkinIndex Skin index. CNcomment: 皮肤状态索引
* param[in] hSkin     Skin handle. CNcomment: 皮肤资源句柄
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_INVAL
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_NORES CNcomment: 内存不够
* see:HI_GV_Widget_SetCharset
*/
HI_S32 HI_GV_Widget_SetSkin(HIGV_HANDLE hWidget, HI_U32 hSkinIndex, HI_RESID hSkin);

/*
* brief Set siblings of widget.CNcomment: 设置控件的兄弟
* param[in] hWidget  Widget handle.  CNcomment: 控件句柄
* param[in] hLeft      Left siblings widet. CNcomment: 左邻居控件句柄
* param[in] hRight    Right siblings widet.CNcomment:  右邻居控件句柄
* param[in] hUp        Up siblings widet.CNcomment: 上邻居控件句柄
* param[in] hDown    Down siblings widet. CNcomment:  下邻居控件句柄
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_INVAL
* retval ::HI_ERR_COMM_LOST
* see : HI_GV_Widget_GetSiblings
*/
HI_S32 HI_GV_Widget_SetSiblings(HIGV_HANDLE hWidget, HIGV_HANDLE hLeft, HIGV_HANDLE hRight, HIGV_HANDLE hUp,
                                HIGV_HANDLE hDown);

/*
* brief Show widget. CNcomment: 显示控件
* param[in] hWidget   Widget handle. CNcomment: 控件句柄
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* see: HI_GV_Widget_Hide
*/
HI_S32 HI_GV_Widget_Show(HIGV_HANDLE hWidgt);

/*
* brief Move widget.CNcomment: 移动控件坐标
* param[in] hWidget Widget handle.  CNcomment:  控件句柄
* param[in] x       the x coordinate that moved after.   CNcomment: 移动后的x坐标
* param[in] y       the y coordinate  that moved after.  CNcomment: 移动后的y坐标
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
*/
HI_S32 HI_GV_Widget_Move(HIGV_HANDLE hWidgt, HIGV_CORD x, HIGV_CORD y);

/*
* brief Move window to top.CNcomment: 把窗口置顶，对控件无效
* param[in] hWidget  Widget handle.  CNcomment: 窗口句柄
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_NOOP Unsupport operation, such as the widget except window not support the api.CNcomment:
* 不支持该操作，例如对控件调用该接口
*/
HI_S32 HI_GV_Widget_MoveToTop(HIGV_HANDLE hWidget);

/*
* brief Hightlight widget.CNcomment: 高亮控件
* param[in] hWidget   Widget handle. CNcomment: 控件句柄
* param[in] highlight  CNcomment: 高亮标志
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_NOOP Unsupport operation, such as the widget is disabed.CNcomment: 不支持该操作，例如控件禁用
*/
HI_S32 HI_GV_Widget_Highlight(HIGV_HANDLE hWidget, HI_BOOL highlight);

/*
* brief Whether highligt the widget.CNcomment: 控件是否高亮
* param[in] hWidget   Widget handle. CNcomment: 控件句柄
* retval ::HI_TRUE Highlight. CNcomment:  高亮
* retval ::HI_FALSE it is not hightlight.CNcomment: 非高亮
*/
HI_BOOL HI_GV_Widget_IsHighlight(HIGV_HANDLE hWidget);

/*
* brief Set transparent of widget.CNcomment: 设置控件是否透明
* param[in] hWidget  Widget handle.  CNcomment: 控件句柄
* param[in] IsTrans  Transparent flag.  CNcomment: 透明标志
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
*/
HI_S32 HI_GV_Widget_SetTransparent(HIGV_HANDLE hWidget, HI_BOOL IsTrans);

/*
* brief Set alignment of text.CNcomment: 设置文本对齐方式
* param[in] hWidget Widget handle.   CNcomment: 控件句柄
* param[in] align  Alignment of text. CNcomment:  文本对齐方式
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
*/
HI_S32 HI_GV_Widget_SetAlign(HIGV_HANDLE hWidget, HI_U32 align);

/*
* brief Set text.CNcomment: 设置文本
* param[in] hWidget  Widget handle.  CNcomment: 控件句柄
* param[in] pText   Text of wiget.  CNcomment:  控件文本内容
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_INVAL
* retval ::HI_ERR_COMM_NORES No enought memory.CNcomment: 没有内存
* see
* ::HI_GV_Widget_GetText
* ::HI_GV_Widget_SetTextByID
*/
HI_S32 HI_GV_Widget_SetText(HIGV_HANDLE hWidget, const HI_CHAR *pText);

/*
* brief Set text by ID.CNcomment: 通过字符串ID设置文本
* param[in] hWidget  Widget handle.  CNcomment: 控件句柄
* param[in] StrID      String ID.CNcomment: 控件文本ID
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* see:
* HI_GV_Widget_GetText
* HI_GV_Widget_GetTextID
* HI_GV_Widget_SetText
*/
HI_S32 HI_GV_Widget_SetTextByID(HIGV_HANDLE hWidget, const HI_U32 StrID);

/*
* brief Get text.CNcomment: 获取控件文本
* param[in] hWidget Widget handle.   CNcomment: 控件句柄
* param[out] pBuf     Text content. CNcomment: 控件文本内容
* param[in] BufLen     Buffer length. CNcomment: 输出pBuf的长度
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_INVAL Null pointer and the buffer length is 0.CNcomment: 空指针或buffer长度为0
* see
* HI_GV_Widget_SetText
*/
HI_S32 HI_GV_Widget_GetText(HIGV_HANDLE hWidget, HI_CHAR *pBuf, HI_U32 BufLen);

/*
* brief Get text by ID.CNcomment: 获取控件文本ID
* attention:
* CNcomment: 获取的文本ID为language.xml中字段的索引，索引从1开始计数
* param[in] hWidget Widget handle. CNcomment:  控件句柄
* param[out] pStrID    String ID.CNcomment:  输出文本ID
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_INVAL
* see :HI_GV_Widget_SetTextByID
*/
HI_S32 HI_GV_Widget_GetTextID(HIGV_HANDLE hWidget, HI_U32 *pStrID);

/*
* brief Bind database to widget.CNcomment: 绑定数据库
* param[in] hWidget Widget handle. CNcomment: 控件句柄
* param[in] hDataSource Data source object.CNcomment: 数据库对象
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* see:HI_GV_Widget_UnbindDataSource
*/
HI_S32 HI_GV_Widget_BindDataSource(HIGV_HANDLE hWidget, HIGV_HANDLE hDataSource);

/*
* brief Unbind database to widget.CNcomment: 解绑定数据库
* param[in] hWidget  Widget handle. CNcomment: 控件句柄
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_INVAL
* see :HI_GV_Widget_BindDataSource
*/
HI_S32 HI_GV_Widget_UnbindDataSource(HIGV_HANDLE hWidget);

/*
* brief Get datasource handle.CNcomment: 获取数据源句柄
* param[in] hWidget  Widget handle. CNcomment:  控件句柄
* param[out] pDataSource The pointer of datasource.CNcomment: 数据源句柄指针
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_INVAL
*/
HI_S32 HI_GV_Widget_GetDataSource(HIGV_HANDLE hWidget, HIGV_HANDLE *pDataSource);

/*
* brief Get window which the widget attach. CNcomment: 获取控件所在主窗口句柄
* param[in] hWidget Widget handle. CNcomment: 控件句柄
* param[out] pWindow  the pointer of parent widget handle. CNcomment:  父窗口句柄指针
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_INVAL
*/
HI_S32 HI_GV_Widget_GetWindow(HIGV_HANDLE hWidget, HIGV_HANDLE *pWindow);

/*
* brief Immediate repaint control.CNcomment: 即时重绘控件，不产生HIGV_MSG_PAINT消息
* param[in] hWidget  Widget handle.  CNcomment: 控件句柄
* param[in] pRect    The area of update CNcomment: 刷新区域
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
*/
HI_S32 HI_GV_Widget_Update(HIGV_HANDLE hWidget, const HI_RECT *pRect);

/*
* brief Immediate update the area of screen.CNcomment: 将控件的指定区域立即显示到屏幕上
* param[in] hWidget Widget handle.  CNcomment:  控件句柄
* param[in] pRect  The area of update. CNcomment:  刷新区域
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
*/
HI_S32 HI_GV_Widget_Refresh(HIGV_HANDLE hWidget, const HI_RECT *pRect);

/*
* brief Get data from db and notify the widget to upate data.CNcomment: 从数据源获取数据，通知控件显示最新的数据
* param[in] hWidget  Widget handle.CNcomment: 控件句柄
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_INVAL
*/
HI_S32 HI_GV_Widget_SyncDB(HIGV_HANDLE hWidget);

/*
* brief Get area of widget, the widget coordate is refre to the parent, and window is refer to the screen.CNcomment:
* 获取控件或窗口的区域。坐标相对于其父；窗口则相对于屏幕 \attention \n N/A.CNcomment:无 \param[in] hWidget Widget handle.
* CNcomment: 控件句柄 \param[out] pRect    The size of widget.CNcomment: 控件区域大小
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_INVAL
*/
HI_S32 HI_GV_Widget_GetRect(HIGV_HANDLE hWidget, HI_RECT *pRect);

/*
* brief Set colorkey of window.CNcomment: 设置窗口colorkey
* param[in] hWindow Window handle. CNcomment: 控件句柄
* param[in] ColorKey colorekey value.CNcomment: colorkey值
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
*/
HI_S32 HI_GV_Widget_SetWinColorKey(HIGV_HANDLE hWindow, HI_COLOR ColorKey);

/*
* brief is it a window.CNcomment: 控件是否为窗口
* param[in] hWidget   Widget handle.CNcomment: 控件句柄
* retval ::HI_TRUE
* retval ::HI_FALSE
*/
HI_BOOL HI_GV_Widget_IsWindow(HIGV_HANDLE hWidget);

/*
* brief Cancel paint.CNcomment: 取消绘制
* param[in] hWidget Widet handle.CNcomment: 控件句柄
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
*/
HI_S32 HI_GV_Widget_Depaint(HIGV_HANDLE hWidget);

/*
* brief Convert the screen coordaite to widget coordiate.CNcomment: 将屏幕坐标转换成控件坐标
* param[in] hWidget Widget handle.CNcomment: 控件句柄
* param[in] ScreenX The x coordinate of screen.CNcomment: 屏幕坐标X
* param[in] ScreenY The y coordinate of screen.CNcomment: 屏幕坐标Y
* param[out] pWidgetX The x coordinate of widget.CNcomment: 存放控件坐标X
* param[out] pWidgetY The y coordinate of widget.CNcomment: 存放控件坐标Y
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
*/
HI_S32 HI_GV_Widget_Screen2Widget(HIGV_HANDLE hWidget, HIGV_CORD ScreenX, HIGV_CORD ScreenY, HIGV_CORD *pWidgetX,
                                  HIGV_CORD *pWidgetY);

/*
* brief Convert thewidget coordaite to  screen coordiate.CNcomment: 将控件坐标转换成屏幕坐标
* param[in] hWidget Widget handle.CNcomment: 控件句柄
* param[in] WidgetX The x coordinate of widget.CNcomment: 控件坐标X
* param[in] WidgetY The y coordinate of widget.CNcomment: 控件坐标Y
* param[out] pScreenX The x coordinate of screen.CNcomment: 存放屏幕坐标X
* param[out] pScreenY The y coordinate of screen.CNcomment: 存放屏幕坐标Y
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
*/
HI_S32 HI_GV_Widget_Widget2Screen(HIGV_HANDLE hWidget, HIGV_CORD WidgetX, HIGV_CORD WidgetY, HIGV_CORD *pScreenX,
                                  HIGV_CORD *pScreenY);

/*
* brief Get widget correspoding coordate.CNcomment: 获取坐标所在顶层控件
* param[in]  ScreenX The x coordinate of widget.CNcomment: 控件坐标X
* param[in]  ScreenY The x coordinate of widget.CNcomment: 控件坐标Y
* param[out] phWidget The x coordinate of screen.CNcomment: 获取的屏幕顶层控件
* retval ::HI_SUCCESS
* retval ::HI_EINVAL
* retval ::HI_ENOTINIT
* retval ::HI_FAILURE
* retval ::HI_ERR_COMM_LOST
*/
HI_S32 HI_GV_Widget_GetWidgetByPos(HIGV_CORD ScreenX, HIGV_CORD ScreenY, HIGV_HANDLE *phWidget);

/*
* brief Get widget correspoding coordate.CNcomment: 获取坐标所在顶层控件,适用于触摸屏设备
* param[in]  ScreenX The x coordinate of widget.CNcomment: 控件坐标X
* param[in]  ScreenY The x coordinate of widget.CNcomment: 控件坐标Y
* param[out] phWidget The x coordinate of screen.CNcomment: 获取的屏幕顶层控件,包含容器类控件
* retval ::HI_SUCCESS
* retval ::HI_EINVAL
* retval ::HI_ENOTINIT
* retval ::HI_FAILURE
* retval ::HI_ERR_COMM_LOST
*/
HI_S32 HI_GV_Widget_GetWidgetByPos_TouchDevice(HIGV_CORD ScreenX, HIGV_CORD ScreenY, HIGV_HANDLE *phWidget);

/*
* brief Bind the scrollbar to widget.CNcomment: 控件绑定滚动条
* param[in] hWidget  Widget handle. CNcomment: 控件句柄
* param[in] hScrollBar Scrollbar handle.CNcomment: 滚动条控件句柄
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_INVAL
*/
HI_S32 HI_GV_Widget_BindScrollBar(HIGV_HANDLE hWidget, HIGV_HANDLE hScrollBar);

/*
* brief Get the scrollbar of widget.CNcomment: 获取该控件绑定的scrollbar
* param[in] hWidget  widget handle. CNcomment: 控件句柄
* param[in] hScrollBar  Binding scrollbar CNcomment: 绑定的scrollbar
* retval N/A.CNcomment:无
*/
HI_S32 HI_GV_Widget_GetScrollBar(HIGV_HANDLE hWidget, HIGV_HANDLE *hScrollBar);

#ifdef HIGV_USE_WIDGET_BUTTON
/*
* brief Get the sate of all checkbox.CNcomment: 将该控件下的所有Check Box设置为选中状态或未选中状态
* param[in] hParent Parent widget handle. CNcomment: 父控件句柄
* param[in] eStatus  Status. CNcomment: 状态
* retval N/A.CNcomment:无
*/
HI_S32 HI_GV_Widget_SetAllCheckStatus(HIGV_HANDLE hParent, HIGV_BUTTON_STATUS_E eStatus);
#endif

/*
* brief Change window size, it is not effect widget. CNcomment: 改变窗口大小，对于控件无效
* param[in] hWidget  Widget handle.  CNcomment: 窗口句柄
* param[in] Width   Window width.   CNcomment: 改变后窗口宽度
* param[in] Height    widnow height. CNcomment:  改变后窗口高度
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_NOOP
*/
HI_S32 HI_GV_Widget_Resize(HIGV_HANDLE hWidgt, HI_S32 Width, HI_S32 Height);

/*
* brief Query the widget whether support the input method window.CNcomment: 查寻目标控件是否需要弹出输入法窗口
* param[in] hWidgt      Widget handle.  CNcomment: 窗口句柄
* param[in] SupportIMEType  Input method type.CNcomment:  支持的输入法类型
*          *    取值为以下类型位或：
*         *    支持所有：  HIGV_IMETYPE_ALLTYPE
*             英文：      HIGV_IMETYPE_ENGLISH
*             大写英文：  HIGV_IMETYPE_CAPENGLISH
*             数字：      HIGV_IMETYPE_NUMBER
*             符号：      HIGV_IMETYPE_SYMBOL
*             拼音：      HIGV_IMETYPE_PINYIN
* retval HI_TRUE          Is need input method window.   CNcomment: 需要输入法窗口
* return HI_FALSE          it is not need. CNcomment:  不需要
*/
HI_BOOL HI_GV_Widget_IsNeedIMEWindow(HIGV_HANDLE hWidgt, HI_U32 *SupportIMEType);

/*
* brief Get the layer correspoding widget.CNcomment: 获取Widget所属图层
* param[in]  hWidgt       Widget handle.CNcomment: Widget句柄
* param[out] pLayer       Layer.   CNcomment: 图层
* retval HI_SUCCESS
* return HI_ERR_COMM_LOST
* return HI_ERR_COMM_EMPTY
*/
HI_S32 HI_GV_Widget_GetLayer(HIGV_HANDLE hWidget, HIGV_HANDLE *pLayer);

/*
* brief Set the private data of widget.CNcomment: 设置Widget的私有数据
* attention :The memory of private data should be released by user.
* CNcomment: 私有数据用户负责释放内存
* param[in]  hWidgt      Widget handle.CNcomment: Widget句柄
* param[in] pPrivateData    The pointer of private data.CNcomment: 私有数据指针
* retval ::HI_SUCCESS
* return ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_INVAL
*/
HI_S32 HI_GV_Widget_SetPrivate(HIGV_HANDLE hWidget, HI_VOID *pPrivateData);

/*
* brief CNcomment: 获取Widget的私有数据
* The memory of private data should be released by user.
* CNcomment: 私有数据用户负责释放内存
* aram[in]  hWidgt    Widget handle.CNcomment:  Widget句柄
* param[out] ppPrivateData    The pointer of private data. CNcomment: 私有数据指针
* retval ::HI_SUCCESS
* return ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_INVAL
*/
HI_S32 HI_GV_Widget_GetPrivate(HIGV_HANDLE hWidget, HI_VOID **ppPrivateData);

/*
* brief Get the clone handle correspoding layer.CNcomment: 获取指定图层控件的克隆句柄
* attention \n CNcomment: 该函数查询的克隆句柄未必有效，需要做有效性检查
* param[in]  hWidget  Widget handle.CNcomment: 控件句柄
* param[in]  hLayer     Layer handle.CNcomment: 图层句柄
* param[out] phClone   Clone handle. CNcomment: 克隆的句柄
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_INVAL
* retval ::HI_ERR_COMM_INTER
* retval ::HI_ERR_COMM_OVERFLOW The layer number is over.CNcomment:  图层数超出范围
*/
HI_S32 HI_GV_Widget_GetCloneHandle(HIGV_HANDLE hWidget, HIGV_HANDLE hLayer, HIGV_HANDLE *phClone);

/*
* brief Whether errasing the bakground.CNcomment: 设置控件是否擦除背景
* attention  CNcomment: 窗口共享模式默认是不擦除背景\n
* param[in]  hWidget  Widget handle.CNcomment: 控件句柄
* param[in] isErasebg CNcomment: 是否擦除背景
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
*/
HI_S32 HI_GV_Widget_EraseBackground(HIGV_HANDLE hWidget, HI_BOOL isErasebg);

/*
* brief Register the infomation of widget type, user can apply this api to register their own widget.CNcomment:
* 注册控件类型信息，用户可以通过该接口 注册自定义的控件类型信息 \attention \n N/A.CNcomment:无 \param[in] TypeInfo The
* info of widget typeCNcomment: 控件类型信息
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_EXIST
* retval ::HI_ERR_COMM_INVAL
* retval ::HI_ERR_COMM_NORES
*/
HI_S32 HI_GV_Widget_RgisterWidget(HIGV_WIDGET_TYPEINFO_S TypeInfo);

/*
* brief Unregister the widget.CNcomment: 反注册控件信息
* param[in] TypeID The type ID of widget.CNcomment: 自定义控件类型ID
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
*/
HI_S32 HI_GV_Widget_UnRgisterWidget(HI_U32 TypeID);

/*
* brief Whether active the widget, This api is added for pop window of combobox.CNcomment:
* 为弹出框增加的接口，即使有模态窗口，模态窗口上的弹出框 窗口也可以激活 \attention \n it is should attention to us this
* api, otherwise the modal window will loss the function. CNcomment: 慎重使用此接口，否则模态窗口将失去意义 \param[in]
* hWidget   Widget handle. CNcomment: 控件句柄
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
*/
HI_S32 HI_GV_Widget_CanActive(HIGV_HANDLE hWidget);

/*
* brief Enable/disable mirror the widget when change language.CNcomment:使能是否允许镜像控件
* param[in] hWidget Widget handle.CNcomment: 控件句柄
* param[in] bPosMirror   Whether mirror widget pos.CNcomment:是否允许切换语言时在父容器中镜像
* param[in] bInteriorMirror  Whether mirror widget interior.CNcomment:是否允许切换语言时进行控件内部镜像
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* see : HI_GV_Widget_GetMirrorAttr
*/
HI_S32 HI_GV_Widget_EnableMirror(HIGV_HANDLE hWidget, HI_BOOL bPosMirror, HI_BOOL bInteriorMirror);

/*
* brief Get the mirror attribute.CNcommment:获取镜像控件属性
* param[in] hWidget  Widget handle.CNcomment: 控件句柄
* param[out] pPosMirror   Whether mirror widget pos.CNcomment:是否允许切换语言时在父容器中镜像
* param[out] pInteriorMirror  Whether mirror widget interior.CNcomment:是否允许切换语言时进行控件内部镜像
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_EMPTY
* see : HI_GV_Widget_EnableMirror
*/
HI_S32 HI_GV_Widget_GetMirrorAttr(HIGV_HANDLE hWidget, HI_BOOL *pPosMirror, HI_BOOL *pInteriorMirror);

/*
* brief Set text direction of the widget.CNcommment:强制控件文本的方向
* param[in] hWidget  Widget handle.CNcomment: 控件句柄
* param[in] Direction  Text direction.CNcomment:文本的方向
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
*/
HI_S32 HI_GV_Widget_SetTextDir(HIGV_HANDLE hWidget, HIGV_TEXTDIR_E Direction);

/*
* brief Set fouces switch mode of the widget.CNcommment:设置控件焦点切换模式
* param[in] hWidget  Widget handle.CNcomment: 控件句柄
* param[in] Direction  FousMode.CNcomment:焦点切换模式
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
*/
HI_S32 HI_GV_Widget_SetFoucsMode(HIGV_HANDLE hWidget, HIGV_FOCUS_SWITCH_E FousMode);

/*
* brief Set text or image margin. CNcomment:设置文字或图片显示的边缘间隔
* attention:
* For button,label,trackbar,listbox,msgbox,timerbar.
* CNcomment:作用于button label trackbar listbox msgbox timerbar控件
* param[in] hWidget  Widget handle.CNcomment: 控件句柄
* param[in] LeftMargin  Left margin.CNcomment:左边缘间隔
* param[in] RightMargin  Right margin.CNcomment:右边缘间隔
* param[in] TopMargin  Top margin.CNcomment:上边缘间隔
* param[in] BottomMargin  Bottom margin.CNcomment:下边缘间隔
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_INVAL
*/
HI_S32 HI_GV_Widget_SetMargin(HIGV_HANDLE hWidget, HI_U32 LeftMargin, HI_U32 RightMargin, HI_U32 TopMargin,
                              HI_U32 BottomMargin);

/*
* brief Get the margin.CNcomment:获取边缘间隔
* param[in] hWidget  Widget handle.CNcomment: 控件句柄
* param[out] pLeftMargin Left margin.CNcomment: 左边缘间隔
* param[out] pRightMargin  Right margin.CNcomment:右边缘间隔
* param[out] pTopMargin  Top margin.CNcomment:上边缘间隔
* param[out] pBottomMargin Bottom margin.CNcomment:下边缘间隔
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
* retval ::HI_ERR_COMM_INVAL
*/
HI_S32 HI_GV_Widget_GetMargin(HIGV_HANDLE hWidget, HI_U32 *pLeftMargin, HI_U32 *pRightMargin, HI_U32 *pTopMargin,
                              HI_U32 *pBottomMargin);

/*
* brief Enable config image res path.CNcommment:使能配置图片资源的相对路径
* param[in] hWidget  Widget handle.CNcomment: 控件句柄
* param[in] Enable Enable or disable image res path.CNcomment:使能开关
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
*/
HI_S32 HI_GV_Widget_EnableResPach(HIGV_HANDLE hWidget, HI_BOOL Enable);

/*
* brief Enable widget foget focus.CNcommment: show & active控件
* param[in] hWidget  Widget handle.CNcomment: 控件句柄
* param[in] bActiveShow. CNcomment:使能或非使能
* retval ::HI_SUCCESS
*/
HI_S32 HI_GV_Widget_ActiveShow(HIGV_HANDLE hWidget, HI_BOOL bActiveShow);

/*
* brief Set bit block transfer operation of picture type skin.CNcommment: 设置控件图片类皮肤的搬移混合操作运算
* param[in] hWidget  Widget handle.CNcomment: 控件句柄
* param[in] pBlitOpt Bit block transfer operation.CNcomment:搬移混合操作运算属性
* retval ::HI_SUCCESS
* retval ::HI_ERR_COMM_LOST
*/
HI_S32 HI_GV_Widget_SetSkinBlitOpt(HIGV_HANDLE hWidget, HIGO_BLTOPT_S *pBlitOpt);

/* * @} */ /* * <!-- ==== API declaration end ==== */
#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __HI_GV_WIDGET_H__ */
