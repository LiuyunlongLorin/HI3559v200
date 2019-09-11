#ifndef __SAMPLE_UTILS_H__
#define __SAMPLE_UTILS_H__
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <hi_gv.h>

#define RES_PATH  "./res/"

#define SBFONT_FILE  RES_PATH"font/ttf/simhei.ttf"
#define MBFONT_FILE  RES_PATH"font/ttf/simhei.ttf"

#if defined(__HI3559AV100__)
#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 480
#else if (defined(__HI3556__) || defined(__HI3559__) || defined(__HI3556AV100__) || defined(__HI3559V200__))
#define SCREEN_WIDTH  240
#define SCREEN_HEIGHT 320
#endif


#define INVALID_HANDLE 0x0
#define WIDGET_CHECK_RET(p, val)   if(!(p)) { printf("Error==%s,%d==\n", __FUNCTION__, __LINE__);return (val);}

#define HI_GV_Widget_SetMsgProcX(x1,x2,x3)  HI_GV_Widget_SetMsgProc(x1,x2,x3,HIGV_PROCORDER_BEFORE)

#define HIGV_CONTROL_PAGENUM   11

#define HIGV_CHECK(Function, Ret)  \
    if (Ret != HI_SUCCESS) {printf("[Func: %s, Line: %d] %s fail Ret(x):%x, Ret(d): %d\n", __FUNCTION__, __LINE__, Function, Ret, Ret);}

#endif
