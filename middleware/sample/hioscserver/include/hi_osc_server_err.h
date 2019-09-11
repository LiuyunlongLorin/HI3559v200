/**
* Copyright (C), 2016-2030, Hisilicon Tech. Co., Ltd.
* All rights reserved.
*
* @file      hi_osc_server_err.h
* @brief     osc module header file
* @author    HiMobileCam middleware develop team
* @date      2017.09.01
*/

#ifndef __HI_OSC_SERVER_ERR_H__
#define __HI_OSC_SERVER_ERR_H__

#include "hi_error_def.h"
#include "hi_defs.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
typedef enum hiAPP_OSCSERVER_ERR_CODE_E
{
    /*general error code*/
    APP_OSCSERVER_ERR_HANDLE_INVALID = 0x40,                     /**<oscserver handle invalid*/
    APP_OSCSERVER_ERR_INVALID_ARG = 0x41,                        /**<param is null or invalid*/
    APP_OSCSERVER_ERR_MALLOC_FAIL = 0x42,                        /**<malloc memory fail*/
    APP_OSCSERVER_ERR_API_FAIL = 0x43,                           /**<start oscserver*/
    APP_OSCSERVER_ERR_CREATE_AGAIN  = 0x44,                      /**<oscserver have been created*/
    APP_OSCSERVER_ERR_SOCKETFD_INVAILD = 0x45,                   /**<oscserver socket fd invaild*/
    APP_OSCSERVER_ERR_NOT_CREATE = 0x46,                         /**<oscserver not created*/
    APP_OSCSERVER_ERR_NEED_EXIT = 0x47,                          /**<oscserver nedd stop*/

    /*command status related error code*/
    APP_OSCSERVER_ERR_INVAILD_COMMANDID = 0x50,                  /**<invaild commnad id*/
    APP_OSCSERVER_ERR_INVAILD_COMMAND_STATUS = 0x51,             /**<invaild commnad status*/

    APP_OSCSERVER_ERR_SESS_SEND_FAIL   = 0x60,                   /**<send response fail*/
    APP_OSCSERVER_ERR_SESS_BAD_REQUEST   = 0x61,                 /**<bad request*/

    /*state change  related error code*/
    APP_OSCSERVER_API_CHANGE_STATE_FAIL   = 0x70,                /**<change state fail*/

    /*Reg command code*/
    APP_OSCSERVER_REG_COMMAND_INPUT_ERR  = 0x80,                 /**<Reg command input err*/
    APP_OSCSERVER_REG_COMMAND_MALLOC_FAIL  = 0x81,               /**<Reg command malloc fail*/

    /*livepreview*/
    APP_OSCSERVER_FRAME_MALLOC_FAIL = 0x91,                      /**<pack frame input null fail*/
    APP_OSCSERVER_FRAME_SEND_FAIL = 0x92,                        /**<pack frame send fail*/
    APP_OSCSERVER_FRAME_INPUT_LENGTH_INVAILD = 0x93,             /**<pack frame input length invaild*/

    APP_OSCSERVER_BUTT = 0xFF
} HI_APP_OSCSERVER_ERR_CODE_E;


/*general error code*/
#define HI_ERR_OSCSERVER_SOCKETFD_INVAILD                               HI_APP_DEF_ERR(HI_APPID_OSCSERVER,APP_ERR_LEVEL_ERROR,APP_OSCSERVER_ERR_SOCKETFD_INVAILD)
#define HI_ERR_OSCSERVER_NULL_PTR                                       HI_APP_DEF_ERR(HI_APPID_OSCSERVER,APP_ERR_LEVEL_ERROR,APP_ERR_NULL_PTR)
#define HI_ERR_OSCSERVER_HANDLE_INVALID                                 HI_APP_DEF_ERR(HI_APPID_OSCSERVER,APP_ERR_LEVEL_ERROR,APP_OSCSERVER_ERR_HANDLE_INVALID)
#define HI_ERR_OSCSERVER_ILLEGAL_PARAM                                  HI_APP_DEF_ERR(HI_APPID_OSCSERVER,APP_ERR_LEVEL_ERROR,APP_OSCSERVER_ERR_INVALID_ARG)
#define HI_ERR_OSCSERVER_ERR_MALLOC_FAIL                                HI_APP_DEF_ERR(HI_APPID_OSCSERVER,APP_ERR_LEVEL_ERROR,APP_OSCSERVER_ERR_MALLOC_FAIL)
#define HI_ERR_OSCSERVER_ERR_API_FAIL                                   HI_APP_DEF_ERR(HI_APPID_OSCSERVER,APP_ERR_LEVEL_ERROR,APP_OSCSERVER_ERR_API_FAIL)
#define HI_ERR_OSCSERVER_ERR_CREATE_AGAIN                               HI_APP_DEF_ERR(HI_APPID_OSCSERVER,APP_ERR_LEVEL_ERROR,APP_OSCSERVER_ERR_CREATE_AGAIN)
#define HI_ERR_OSCSERVER_ERR_NOT_CREATE                                 HI_APP_DEF_ERR(HI_APPID_OSCSERVER,APP_ERR_LEVEL_ERROR,APP_OSCSERVER_ERR_NOT_CREATE)
#define HI_ERR_OSCSERVER_ERR_NEED_EXIT                                  HI_APP_DEF_ERR(HI_APPID_OSCSERVER,APP_ERR_LEVEL_ERROR,APP_OSCSERVER_ERR_NEED_EXIT)

/*command id*/
#define HI_ERR_OSCSERVER_INVAILD_COMMANDID                              HI_APP_DEF_ERR(HI_APPID_OSCSERVER,APP_ERR_LEVEL_ERROR,APP_OSCSERVER_ERR_INVAILD_COMMANDID)
#define HI_ERR_OSCSERVER_INVAILD_COMMAND_STATUS                         HI_APP_DEF_ERR(HI_APPID_OSCSERVER,APP_ERR_LEVEL_ERROR,APP_OSCSERVER_ERR_INVAILD_COMMAND_STATUS)

/*http related*/
#define HI_ERR_OSCSERVER_ERR_SESS_SEND_FAIL                             HI_APP_DEF_ERR(HI_APPID_OSCSERVER,APP_ERR_LEVEL_ERROR,APP_OSCSERVER_ERR_SESS_SEND_FAIL)
#define HI_ERR_OSCSERVER_ERR_SESS_BAD_REQUEST                           HI_APP_DEF_ERR(HI_APPID_OSCSERVER,APP_ERR_LEVEL_ERROR,APP_OSCSERVER_ERR_SESS_BAD_REQUEST)
#define HI_ERR_OSCSERVER_ERR_CHANGE_STATE_FAIL                          HI_APP_DEF_ERR(HI_APPID_OSCSERVER,APP_ERR_LEVEL_ERROR,APP_OSCSERVER_API_CHANGE_STATE_FAIL)

/*Reg command related*/
#define HI_ERR_OSCSERVER_REGCOMMAND_INPUT_ERR                           HI_APP_DEF_ERR(HI_APPID_OSCSERVER,APP_ERR_LEVEL_ERROR,APP_OSCSERVER_REG_COMMAND_INPUT_ERR)
#define HI_ERR_OSCSERVER_REGCOMMAND_MALLOC_FAIL                         HI_APP_DEF_ERR(HI_APPID_OSCSERVER,APP_ERR_LEVEL_ERROR,APP_OSCSERVER_REG_COMMAND_MALLOC_FAIL)

/*osc getlivepreview package*/
#define HI_ERR_OSCSERVER_FRAME_INPUT_LENGTH_INVAILD                     HI_APP_DEF_ERR(HI_APPID_OSCSERVER,APP_ERR_LEVEL_ERROR,APP_OSCSERVER_FRAME_INPUT_LENGTH_INVAILD)
#define HI_ERR_OSCSERVER_FRAME_MALLOC_FAIL                              HI_APP_DEF_ERR(HI_APPID_OSCSERVER,APP_ERR_LEVEL_ERROR,APP_OSCSERVER_FRAME_MALLOC_FAIL)
#define HI_ERR_OSCSERVER_FRAME_FRAME_SEND_FAIL                          HI_APP_DEF_ERR(HI_APPID_OSCSERVER,APP_ERR_LEVEL_ERROR,APP_OSCSERVER_FRAME_SEND_FAIL)

/* __cplusplus */
/** \addtogroup     OSCSVR */
/** @{ */  /** <!-- [OSCSVR] */
/** @}*/  /** <!-- ==== OSCSVR End ====*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __HI_OSC_SERVER_ERR_H__ */
