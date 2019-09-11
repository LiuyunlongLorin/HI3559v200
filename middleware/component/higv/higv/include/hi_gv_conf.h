#ifndef __HI_GV_CONF_H__
#define __HI_GV_CONF_H__

#define HIGOV200 200
#define HIGOV300 300
#define HIGOV400 400
#define HIGOV410 410//增加HI_GO_BlitEx接口为支持负坐标
#define HIGOV500 500//解决窗口共享模式下窗口图片皮肤有圆角，higo增加了HIGO_WNDMEM_SHARED_EX

typedef unsigned int HIGV_HANDLE;

#ifdef __LP64__
typedef unsigned long long  HI_PARAM;
typedef struct
{
    long long  x;
    long long  y;
    long long  w;
    long long  h;
} HIGV_RECT_WRAPPER;
#else
typedef unsigned int HI_PARAM;
typedef struct
{
    int x;
    int y;
    int w;
    int h;
} HIGV_RECT_WRAPPER;
#endif

typedef HI_PARAM HIGV_COLOR_WRAPPER;

#define HIGV_COMMON_MODID_START 50
#define HIGV_PERFORMACE 1

#define HIGV_USE_TOUCHSCREEN_INPUT  1
#define HIGV_NOT_REGISTER_ALLWIDGET
#define HIGV_USE_WIDGET_BUTTON 1
#define HIGV_USE_WIDGET_SCROLLBAR 1
#define HIGV_USE_WIDGET_LABEL 1
#define HIGV_USE_WIDGET_IMAGE 1
#define HIGV_USE_WIDGET_IMAGEEX 1
#define HIGV_USE_WIDGET_PROGRESSBAR 1
#define HIGV_USE_WIDGET_SCROLLTEXT 1
#define HIGV_USE_WIDGET_SCROLLGRID 1
#define HIGV_USE_WIDGET_CLOCK 1
#define HIGV_USE_WIDGET_LISTBOX 1
#define HIGV_USE_WIDGET_TRACKBAR 1
#define HIGV_USE_WIDGET_SLIDEUNLOCK 1
#define HIGV_USE_WIDGET_WHEELVIEW 1
#define HIGV_USE_MODULE_TOUCH 1
#define HIGV_USE_WIDGET_SCROLLVIEW 1

#endif  // __HI_GV_CONF_H__
