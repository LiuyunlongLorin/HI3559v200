/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2009-2019. All rights reserved.
 * Description: hi_gv_errno.h
 * Author: NULL
 * Create: 2009-2-9
 */

#ifndef __HI_GV_ERRNO_H__
#define __HI_GV_ERRNO_H__

/* add include here */
#include "hi_type.h"

#ifdef __cplusplus
extern "C" {
#endif

/* **************************** Macro Definition ***************************** */
/* * addtogroup      CoreError   */
/* *  【Error】 CNcomment: 【错误码】 */
typedef enum {
    /* *unknown error ; CNcomment:错误原因未知，确定已经出错但是不能判定错误的原因 */
    HI_ERR_COMM_UNKNOWN = 1,
    /* *other error ; CNcomment:其他错误，知道错误原因但是难以归类 */
    HI_ERR_COMM_OTHER,
    /* *inner error */
    /* * CNcomment:内部错误，例如内部发生断言错误、某些内存崩溃、
    CPU自测错、I/O错、数学运算错、死锁等 */
    HI_ERR_COMM_INTER,
    /* *version error ; CNcomment:版本错误 */
    HI_ERR_COMM_VERSION,
    /* *unsupport function, operation, feature */
    /* *CNcomment:不支持的操作/功能/特性，可能其他版本或者
    未来的版本或者其他产品会有支持*/
    HI_ERR_COMM_PAERM = 5,
    /* *invalid paramater */
    /* *CNcomment:参数错误，包括函数参数错误、配置参数错误、配置参数不一致或有冲突，
    不适当的编号、通道号、设备号、空指针、地址、操作码等等*/
    HI_ERR_COMM_INVAL,
    /* *not init */
    /* * CNcomment:没有初始化，某些操作必须在先进行初始化后才能进行，例如未加锁 */
    HI_ERR_COMM_NOTINIT,
    /* *not ready */
    /* *CNcomment:没有准备好，某些操作必须在具备一定条件后才能进行，或者需要的环境不正确
     ，或者缺乏必需的资源*/
    HI_ERR_COMM_NOTREADY,
    /* *no resource ; CNcomment:没有资源，例如申请内存失败、没有空闲缓冲区、没有空闲端口、没有空闲通道等 */
    HI_ERR_COMM_NORES,
    /* *resouce is  exist ; CNcomment:资源已存在，欲申请或新建的资源已经存在，容器中有容量 */
    HI_ERR_COMM_EXIST = 10,
    /* *resouce is not exist ; CNcomment:资源不存在，依赖的某种资源、地址、会话不存在 */
    HI_ERR_COMM_LOST,
    /* *no operation ; CNcomment:资源不可操作，可能已损坏不可使用、校验错误、未发挥预期的作用、设备不兼容等等 */
    HI_ERR_COMM_NOOP,
    /* *busy ; CNcomment:资源正忙碌，例如被加锁 */
    HI_ERR_COMM_BUSY,
    /* *idle ; CNcomment:资源正空闲 */
    HI_ERR_COMM_IDLE,
    /* *full ; CNcomment:满，某种容器中已经被充满 */
    HI_ERR_COMM_FULL = 15,
    /* *empty ; CNcomment:空，某种容器中是空的 */
    HI_ERR_COMM_EMPTY,
    /* * underflow ; CNcomment:下溢，某种容器中的容量已经下降到下溢水线之下 */
    HI_ERR_COMM_UNDERFLOW,
    /* * Overflow ; CNcomment:上溢，某种容器中的容量上升到上溢水线之上 */
    HI_ERR_COMM_OVERFLOW,
    /* * Couldn't access ; CNcomment:权限错误，例如没有权限、密码错误等等 */
    HI_ERR_COMM_ACCES,
    /* * operation is not finish when get interupt ; CNcomment:操作未完成，已经中断 */
    HI_ERR_COMM_INTR = 20,
    /* * Operation is not finish, it will continue ; CNcomment:操作未完成，仍在继续 */
    HI_ERR_COMM_CONTINUE,
    /* * Operation is over, there is no any object to opreate ; CNcomment:操作完成，已没有后续的操作对象 */
    HI_ERR_COMM_OVER,
    /* * Operation is not support ; CNcomment:不支持的操作 */
    HI_ERR_COMM_UNSUPPORT,
    /* * out of bound ; CNcomment:超出边界 */
    HI_ERR_COMM_OUTOFBOUND,
    /* * File opreation error ; CNcomment:文件操作错误 */
    HI_ERR_COMM_FILEOP,
    /* * Secure function error ; CNcomment:安全函数返回值错误 */
    HI_ERR_COMM_SECURE,
    /* * No effect error ; CNcomment:对业务功能无影响的返回值 ，有别于HI_SUCCESS */
    HI_ERR_COMM_NOEFFECT,

    HI_ERR_COMM_END = 28,

    /* *Messsage module */
    /* * Send message failuer; CNcomment:发送同步VTOP消息错误 */
    HI_ERR_MSGM_VTOPMSGSEND = 500,
    /* * Create message queue failure ; CNcomment:创建VTOP消息队列失败 */
    HI_ERR_MSGM_VTOPMSGCREATE,
    /* * Create message server failure ; CNcomment:创建VTOP消息服务器失败 */
    HI_ERR_MSGM_VTOPMSGSERVER,

    /* *APP */
    /* * The base of task manager ; CNcomment:任务管理模块错误码的基准值 */
    /* *Tase not stop ; CNcomment:任务未停止 */
    HI_ERR_TASK_TASKNOTSTOP,

    /* *resouce manager */
    /* * no memory ; CNcomment:内存不足 */
    HI_ERR_RES_NOMEN,
    /* * unknown resource type ; CNcomment:未知的资源类型 */
    HI_ERR_RES_UNKNOWRES = 505,
    /* * resource is using ; CNcomment:资源正被使用 */
    HI_ERR_RES_USED,
    /* * invalid resouce type ; CNcomment:无效的资源类型 */
    HI_ERR_RES_INVRESTYPE,
    /* * The pointer is null when save resouce infoamtion ; CNcomment:保存资源信息的指针为空 */
    HI_ERR_RES_NULL,
    /* * Load resouce is failure ; CNcomment:资源加载失败 */
    HI_ERR_RES_LOAD,
    /* * The rource is not loaded ; CNcomment:资源未加载 */
    HI_ERR_RES_NOLOAD = 510,
    /* * Create resource failure ; CNcomment:资源创建失败，一般只调用HIGO接口失败 */
    HI_ERR_RES_CREATE,

    /* *Window manager ; CNcomment: 窗口管理 */
    /* *The base of window manager ; CNcomment: 窗口管理模块错误码的基准值 */
    HI_ERR_WM_CREATE,

    /* *widget framework ; CNcomment: 控件框架 */
    /* * Widget type is no exisit ; CNcomment:指定widget类型不存在 */
    HI_ERR_TYPE_NOEXISIT,
    /* * Unsupport multi thread to call ; CNcomment:不支持在其它线程中调用 */
    HI_ERR_MULTIPLE_THREAD_CALL,
    /* *binded db is not used ; CNcomment:绑定的DB不可用 */
    HI_ERR_WIDGET_INVALDB = 515,
    /* * The widget is shown ; CNcomment:Widget已显示 */
    HI_ERR_WIDGET_SHOW,
    /* * the widget is hided ; CNcomment:Widget已hide */
    HI_ERR_WIDGET_HIDE,
    /* * the rect is null ; CNcomment:剪切矩形为空，导致不能画 */
    HI_ERR_INVSIBLE,

    /* DDB */
    /* * no field ; CNcomment:没有字段属性 */
    HI_ERR_DDB_ZEROFIELD,
    /* * out of range ; CNcomment: 超出边界 */
    HI_ERR_DDB_OUTOFRANGE = 520,
    /* * null pointer ; CNcomment:空指针 */
    HI_ERR_DDB_NULLPTR,
    /* * invalid paramater ; CNcomment:非法参数 */
    HI_ERR_DDB_INVAIDPARA,
    /* *Bufffer is too small ; CNcomment:BUFFER太小 */
    HI_ERR_DDB_BUFFSMALL,

    /* * ADM */
    /* *no filed ; CNcomment:没有字段属性 */
    HI_ERR_ADM_ZEROFIELD,
    /* *Out of range ; CNcomment:超出边界 */
    HI_ERR_ADM_OUTOFRANGE = 525,
    /* *Null pointer ; CNcomment:空指针 */
    HI_ERR_ADM_NULLPTR,
    /* *Invalid paramater ; CNcomment:非法参数 */
    HI_ERR_ADM_INVAIDPARA,
    /* *The ADM buffer is too small ; CNcomment:BUFFER太小 */
    HI_ERR_ADM_BUFFSMALL,
    /* *Get data failure ; CNcomment:获取数据失败 */
    HI_ERR_ADM_GETDATA,
    /* *Unknown operation ; CNcomment:数据库未知操作 */
    HI_ERR_ADM_UNKNOWNOPT = 530,

    /* *LANGUAGE */
    /* *Not set locale ; CNcomment:没有设置多语言的LOCALE */
    HI_ERR_LANG_INVALID_LOCALE,

    /* 输入设备 */
    /* * 打开IR设备失败 */
    HI_ERR_IM_OPENIRDEVICE,
    /* * 获取IR值失败 */
    HI_ERR_IM_GETIRVALUE,
    /* * 没有对应的虚拟键 */
    HI_ERR_IM_NOVIRKEY,
    /* 字符集 */
    /* * 不支持的字符集 */
    HI_ERR_CHARSET_UNSUPPORT = 535,
    /* * 无效的字符集转换 */
    HI_ERR_CHARSET_CONVERT,

    /* *CNcomment: Parser模块错误码基准值  ; CNcomment: Parser模块错误码基准值  */
    /* * Not init ; CNcomment:未初始化 */
    HI_ERR_PARSER_NOINIT,
    /* *it is inited ; CNcomment: 已初始化 */
    HI_ERR_PARSER_INITED,
    /* *Check code is error ; CNcomment: 校验码错误 */
    HI_ERR_PARSER_VERIFY = 540,
    /* *The identifier is error ; CNcomment: 文件标识错误 */
    HI_ERR_PARSER_MARK,
    /* *The file type is error ; CNcomment: 文件类型错误 */
    HI_ERR_PARSER_TYPE,
    /* *The length is error ; CNcomment: 数据长度错误 */
    HI_ERR_PARSER_DATALEN,
    /* * Unsupport ;  CNcomment:不支持 */
    HI_ERR_PARSER_UNSUPPORT,
    /* * The data of higv bin is error ;  CNcomment:HIGVBin文件数据错误 */
    HI_ERR_PARSER_DATAERR = 545,
    /* * The view is not loaded fullly ;  CNcomment:视图未全部加载 */
    HI_ERR_PARSER_VIEWNOLOAD,
    /* * File is loaded ;  CNcomment:文件已加载 */
    HI_ERR_PARSER_FILELOADED,
    /* *File is not loaded ; CNcomment: 文件未加载 */
    HI_ERR_PARSER_FILENOTLOAD,
} VResult;

/* *  Macro Definition end */
#ifdef __cplusplus
}
#endif
#endif /* __HI_GV_ERRNO_H__ */
