/**
* Copyright (C), 2016-2030, Hisilicon Tech. Co., Ltd.
* All rights reserved.
*
* @file      hi_rtsp_server_err.h
* @brief     rtsp module header file
* @author    HiMobileCam middleware develop team
* @date      2016.06.29
*/

#ifndef __HI_RTSP_SERVER_ERR_H__
#define __HI_RTSP_SERVER_ERR_H__
#include "hi_error_def.h"
#include "hi_defs.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/* __cplusplus */
/** \addtogroup     RTSPSVR */
/** @{ */  /** <!-- [RTSPSVR] */


typedef enum hiAPP_RTSPSERVER_ERR_CODE_E
{
    /*general error code*/

    APP_RTSPSERVER_ERR_HANDLE_INVALID = 0x40,                     /**<rtspserver handle invalid*/
    APP_RTSPSERVER_ERR_INVALID_ARG = 0x41,                             /**<param is null or invalid*/
    APP_RTSPSERVER_ERR_MALLOC_FAIL = 0x42,                             /**<malloc memory fail*/
    APP_RTSPSERVER_ERR_CREATE_FAIL = 0x43,                               /**<create rtspserverfail*/
    APP_RTSPSERVER_ERR_DESTORY_FAIL = 0x44,                            /**<destory rtspserver  fail*/
    APP_RTSPSERVER_ERR_START_FAIL = 0x45,                                 /**<start rtspserverfail*/
    APP_RTSPSERVER_ERR_STOP_FAIL  = 0x46,                                  /**<stop rtspserver fail*/
    APP_RTSPSERVER_ERR_REACH_MAX_CONNECT  = 0x47,        /**<rtspserver reach the max clients connect*/
    APP_RTSPSERVER_ERR_CREATE_AGAIN  = 0x48,                        /**<rtspserver have been created*/
    APP_RTSPSERVER_ERR_NOT_CREATE = 0x49,                              /**<rtspserver have been created*/



    /*stream related error code*/
    APP_RTSPSERVER_ERR_STREAM_REMOVE_FAIL = 0x50,               /**<remove stream fail*/
    APP_RTSPSERVER_ERR_STREAM_ADD_FAIL = 0x51,                        /**<add stream fail*/
    APP_RTSPSERVER_ERR_STREAM_EXISTED = 0x52,                            /**<add stream existed*/
    APP_RTSPSERVER_ERR_STREAM_NOT_EXIST = 0x53,                      /**<remove or find stream not existed*/
    APP_RTSPSERVER_ERR_STREAM_REMOVE_SESS_FAIL = 0x54,    /**<remove stream related session fail*/
    APP_RTSPSERVER_ERR_STREAM_ADD_SESS_FAIL = 0x55,            /**<add a session for stream fail*/


    /*mbuffer related error code */
    APP_RTSPSERVER_ERR_INIT_MBUF_FAIL   = 0x56,                          /**<init mbuffer fail*/
    APP_RTSPSERVER_ERR_DEL_MBUF_FAIL   = 0x57,                           /**<deinit mbuffer fail*/


    /*listener related error code*/
    APP_RTSPSERVER_ERR_CREATE_LISTENER_FAIL  = 0x60,                   /**<create listener fail*/
    APP_RTSPSERVER_ERR_DESTROY_LISTENER_FAIL   = 0x61,               /**<destroy listener fail*/
    APP_RTSPSERVER_ERR_START_LISTENER_FAIL  = 0x62,                     /**<start listener fail*/
    APP_RTSPSERVER_ERR_STOP_LISTENER_FAIL  = 0x63,                        /**<stop listener fail*/
    APP_RTSPSERVER_ERR_START_LISTENER_AGAIN  = 0x64,                 /**<start the listener again*/
    APP_RTSPSERVER_ERR_STOP_LISTENER_NOT_STARTED = 0x65,     /**<stop the listener not started*/

    /*sess related error code */
    APP_RTSPSERVER_ERR_SESS_HANDLE_INVALID = 0x70,                      /**<session handle invalidt*/
    APP_RTSPSERVER_ERR_SESS_CONNECT_FAIL = 0x71,                            /**<session connect fail*/
    APP_RTSPSERVER_ERR_SESS_CREATE_FAIL   = 0x72,                              /**<create sess fail*/
    APP_RTSPSERVER_ERR_SESS_DESTROY_FAIL   = 0x73,                           /**<destroy sess fail*/
    APP_RTSPSERVER_ERR_SESS_START_FAIL   = 0x74,                                /**<start sess fail*/
    APP_RTSPSERVER_ERR_SESS_STOP_FAIL   = 0x75,                                   /**<stop sess fail*/
    APP_RTSPSERVER_ERR_SESS_SET_FAIL   = 0x76,                                      /**<set sess fail*/
    APP_RTSPSERVER_ERR_SESS_EXISTED   = 0x77,                                       /**<sess already existed*/
    APP_RTSPSERVER_ERR_SESS_NOT_EXISTED = 0x78,                              /**<sess not existed*/
    APP_RTSPSERVER_ERR_SESS_BAD_REQUEST   = 0x79,                          /**<bad request*/
    APP_RTSPSERVER_ERR_SESS_STREAM_NOT_FOUND   = 0x7a,          /**<stream not found*/
    APP_RTSPSERVER_ERR_SESS_UNSUPPORT_TRANSPORT  = 0x7b,    /**<unsuport transtype*/
    APP_RTSPSERVER_ERR_SESS_SEND_FAIL  = 0x7c,                                   /**<send fail*/
    APP_RTSPSERVER_ERR_SESS_UNSUPPORT_MEDIATYPE = 0x7d,      /**<unsuport mediatype*/
    APP_RTSPSERVER_ERR_SESS_NO_DATA = 0x7e,                                     /**<read no data*/


    /*rtp  error code*/
    APP_RTSPSERVER_ERR_RTP_DESTROY_FAIL   = 0x80,                                 /**<create rtp fail*/
    APP_RTSPSERVER_ERR_RTP_CREATE_FAIL   = 0x81,                                    /**<destory rtp fail*/
    APP_RTSPSERVER_ERR_RTP_TRANS_MODE = 0x82,                                    /**<not support trans mode*/
    APP_RTSPSERVER_ERR_RTP_PACKET_TYPE = 0x83,                                    /**<not support pack type*/
    APP_RTSPSERVER_ERR_RTP_TRANS_SEND = 0x84,                                    /**<rtp trans send error*/

    /*rtcp  error code*/
    APP_RTSPSERVER_ERR_RTCP_DESTROY_FAIL   = 0x85,                              /**<create rtcp fail*/
    APP_RTSPSERVER_ERR_RTCP_CREATE_FAIL   = 0x86,                                 /**<destory rtcp fail*/


    /*write frame error code*/
    APP_RTSPSERVER_ERR_WRITE_FRAME_FAIL   = 0x87,                           /**<write frame fail*/
    APP_RTSPSERVER_ERR_MBUF_FULL   = 0x88,                                 /**<buff full fail*/


    APP_RTSPSERVER_BUTT = 0xFF
} HI_APP_RTSPSERVER_ERR_CODE_E;


/*general error code*/
#define HI_ERR_RTSPSERVER_NULL_PTR                                       HI_APP_DEF_ERR(HI_APPID_RTSPSERVER,APP_ERR_LEVEL_ERROR,APP_ERR_NULL_PTR)
#define HI_ERR_RTSPSERVER_HANDLE_INVALID               HI_APP_DEF_ERR(HI_APPID_RTSPSERVER,APP_ERR_LEVEL_ERROR,APP_RTSPSERVER_ERR_HANDLE_INVALID)
#define HI_ERR_RTSPSERVER_ILLEGAL_PARAM                          HI_APP_DEF_ERR(HI_APPID_RTSPSERVER,APP_ERR_LEVEL_ERROR,APP_RTSPSERVER_ERR_INVALID_ARG)
#define HI_ERR_RTSPSERVER_ERR_MALLOC_FAIL                       HI_APP_DEF_ERR(HI_APPID_RTSPSERVER,APP_ERR_LEVEL_ERROR,APP_RTSPSERVER_ERR_MALLOC_FAIL)
#define HI_ERR_RTSPSERVER_ERR_CREATE_FAIL                        HI_APP_DEF_ERR(HI_APPID_RTSPSERVER,APP_ERR_LEVEL_ERROR,APP_RTSPSERVER_ERR_CREATE_FAIL)
#define HI_ERR_RTSPSERVER_ERR_DESTROY_FAIL                     HI_APP_DEF_ERR(HI_APPID_RTSPSERVER,APP_ERR_LEVEL_ERROR,APP_RTSPSERVER_ERR_DESTORY_FAIL)
#define HI_ERR_RTSPSERVER_ERR_START_FAIL                           HI_APP_DEF_ERR(HI_APPID_RTSPSERVER,APP_ERR_LEVEL_ERROR,APP_RTSPSERVER_ERR_START_FAIL)
#define HI_ERR_RTSPSERVER_ERR_STOP_FAIL                              HI_APP_DEF_ERR(HI_APPID_RTSPSERVER,APP_ERR_LEVEL_ERROR,APP_RTSPSERVER_ERR_STOP_FAIL)
#define HI_ERR_RTSPSERVER_ERR_REACH_MAX_CONNECT    HI_APP_DEF_ERR(HI_APPID_RTSPSERVER,APP_ERR_LEVEL_ERROR,APP_RTSPSERVER_ERR_REACH_MAX_CONNECT)
#define HI_ERR_RTSPSERVER_ERR_CREATE_AGAIN                    HI_APP_DEF_ERR(HI_APPID_RTSPSERVER,APP_ERR_LEVEL_ERROR,APP_RTSPSERVER_ERR_CREATE_AGAIN)
#define HI_ERR_RTSPSERVER_ERR_NOT_CREATE                         HI_APP_DEF_ERR(HI_APPID_RTSPSERVER,APP_ERR_LEVEL_ERROR,APP_RTSPSERVER_ERR_NOT_CREATE)




/*stream related error code*/
#define HI_ERR_RTSPSERVER_ERR_STREAM_REMOVE_FAIL            HI_APP_DEF_ERR(HI_APPID_RTSPSERVER,APP_ERR_LEVEL_ERROR,APP_RTSPSERVER_ERR_STREAM_REMOVE_FAIL)
#define HI_ERR_RTSPSERVER_ERR_STREAM_ADD_FAIL                     HI_APP_DEF_ERR(HI_APPID_RTSPSERVER,APP_ERR_LEVEL_ERROR,APP_RTSPSERVER_ERR_STREAM_ADD_FAIL)
#define HI_ERR_RTSPSERVER_ERR_STREAM_EXISTED                         HI_APP_DEF_ERR(HI_APPID_RTSPSERVER,APP_ERR_LEVEL_ERROR,APP_RTSPSERVER_ERR_STREAM_EXISTED)
#define HI_ERR_RTSPSERVER_ERR_STREAM_NOT_EXIST                   HI_APP_DEF_ERR(HI_APPID_RTSPSERVER,APP_ERR_LEVEL_ERROR,APP_RTSPSERVER_ERR_STREAM_NOT_EXIST)
#define HI_ERR_RTSPSERVER_ERR_STREAM_REMOVE_SESS_FAIL  HI_APP_DEF_ERR(HI_APPID_RTSPSERVER,APP_ERR_LEVEL_ERROR,APP_RTSPSERVER_ERR_STREAM_REMOVE_SESS_FAIL)
#define HI_ERR_RTSPSERVER_ERR_STREAM_ADD_SESS_FAIL           HI_APP_DEF_ERR(HI_APPID_RTSPSERVER,APP_ERR_LEVEL_ERROR,APP_RTSPSERVER_ERR_STREAM_ADD_SESS_FAIL)

/*mbuffer related error code*/

#define HI_ERR_RTSPSERVER_ERR_INIT_MBUF_FAIL              HI_APP_DEF_ERR(HI_APPID_RTSPSERVER,APP_ERR_LEVEL_ERROR,APP_RTSPSERVER_ERR_INIT_MBUF_FAIL)
#define HI_ERR_RTSPSERVER_ERR_DEL_MBUF_FAIL              HI_APP_DEF_ERR(HI_APPID_RTSPSERVER,APP_ERR_LEVEL_ERROR,APP_RTSPSERVER_ERR_DEL_MBUF_FAIL)


/*listener related error code*/
#define HI_ERR_RTSPSERVER_ERR_CREATE_LISTENER_FAIL                   HI_APP_DEF_ERR(HI_APPID_RTSPSERVER,APP_ERR_LEVEL_ERROR,APP_RTSPSERVER_ERR_CREATE_LISTENER_FAIL)
#define HI_ERR_RTSPSERVER_ERR_DESTROY_LISTENER_FAIL                HI_APP_DEF_ERR(HI_APPID_RTSPSERVER,APP_ERR_LEVEL_ERROR,APP_RTSPSERVER_ERR_DESTROY_LISTENER_FAIL)
#define HI_ERR_RTSPSERVER_ERR_START_LISTENER_FAIL                      HI_APP_DEF_ERR(HI_APPID_RTSPSERVER,APP_ERR_LEVEL_ERROR,APP_RTSPSERVER_ERR_START_LISTENER_FAIL)
#define HI_ERR_RTSPSERVER_ERR_STOP_LISTENER_FAIL                        HI_APP_DEF_ERR(HI_APPID_RTSPSERVER,APP_ERR_LEVEL_ERROR,APP_RTSPSERVER_ERR_STOP_LISTENER_FAIL)
#define HI_ERR_RTSPSERVER_ERR_START_LISTENER_AGAIN                 HI_APP_DEF_ERR(HI_APPID_RTSPSERVER,APP_ERR_LEVEL_ERROR,APP_RTSPSERVER_ERR_START_LISTENER_AGAIN)
#define HI_ERR_RTSPSERVER_ERR_STOP_LISTENER_NOT_STARTED    HI_APP_DEF_ERR(HI_APPID_RTSPSERVER,APP_ERR_LEVEL_ERROR,APP_RTSPSERVER_ERR_STOP_LISTENER_NOT_STARTED)

/*sess related error code */
#define HI_ERR_RTSPSERVER_ERR_SESS_HANDLE_INVALID                   HI_APP_DEF_ERR(HI_APPID_RTSPSERVER,APP_ERR_LEVEL_ERROR,APP_RTSPSERVER_ERR_SESS_HANDLE_INVALID)
#define HI_ERR_RTSPSERVER_ERR_SESS_CONNECT_FAIL                          HI_APP_DEF_ERR(HI_APPID_RTSPSERVER,APP_ERR_LEVEL_ERROR,APP_RTSPSERVER_ERR_SESS_CONNECT_FAIL)
#define HI_ERR_RTSPSERVER_ERR_SESS_CREATE_FAIL                             HI_APP_DEF_ERR(HI_APPID_RTSPSERVER,APP_ERR_LEVEL_ERROR,APP_RTSPSERVER_ERR_SESS_CREATE_FAIL)
#define HI_ERR_RTSPSERVER_ERR_SESS_DESTROY_FAIL                          HI_APP_DEF_ERR(HI_APPID_RTSPSERVER,APP_ERR_LEVEL_ERROR,APP_RTSPSERVER_ERR_SESS_DESTROY_FAIL)
#define HI_ERR_RTSPSERVER_ERR_SESS_START_FAIL                                HI_APP_DEF_ERR(HI_APPID_RTSPSERVER,APP_ERR_LEVEL_ERROR,APP_RTSPSERVER_ERR_SESS_START_FAIL)
#define HI_ERR_RTSPSERVER_ERR_SESS_STOP_FAIL                                  HI_APP_DEF_ERR(HI_APPID_RTSPSERVER,APP_ERR_LEVEL_ERROR,APP_RTSPSERVER_ERR_SESS_STOP_FAIL)
#define HI_ERR_RTSPSERVER_ERR_SESS_SET_FAIL                                       HI_APP_DEF_ERR(HI_APPID_RTSPSERVER,APP_ERR_LEVEL_ERROR,APP_RTSPSERVER_ERR_SESS_SET_FAIL)
#define HI_ERR_RTSPSERVER_ERR_SESS_EXISTED                                         HI_APP_DEF_ERR(HI_APPID_RTSPSERVER,APP_ERR_LEVEL_ERROR,APP_RTSPSERVER_ERR_SESS_EXISTED)
#define HI_ERR_RTSPSERVER_ERR_SESS_NOT_EXISTED                              HI_APP_DEF_ERR(HI_APPID_RTSPSERVER,APP_ERR_LEVEL_ERROR,APP_RTSPSERVER_ERR_SESS_NOT_EXISTED)
#define HI_ERR_RTSPSERVER_ERR_SESS_BAD_REQUEST                          HI_APP_DEF_ERR(HI_APPID_RTSPSERVER,APP_ERR_LEVEL_ERROR,APP_RTSPSERVER_ERR_SESS_BAD_REQUEST)
#define HI_ERR_RTSPSERVER_ERR_SESS_STREAM_NOT_FOUND           HI_APP_DEF_ERR(HI_APPID_RTSPSERVER,APP_ERR_LEVEL_ERROR,APP_RTSPSERVER_ERR_SESS_STREAM_NOT_FOUND)
#define HI_ERR_RTSPSERVER_ERR_SESS_UNSUPPORT_TRANSPORT     HI_APP_DEF_ERR(HI_APPID_RTSPSERVER,APP_ERR_LEVEL_ERROR,APP_RTSPSERVER_ERR_SESS_UNSUPPORT_TRANSPORT)
#define HI_ERR_RTSPSERVER_ERR_SESS_SEND_FAIL                                   HI_APP_DEF_ERR(HI_APPID_HTTPSERVER,APP_ERR_LEVEL_ERROR,APP_RTSPSERVER_ERR_SESS_SEND_FAIL)
#define HI_ERR_RTSPSERVER_ERR_SESS_UNSUPPORT_MEDIATYPE      HI_APP_DEF_ERR(HI_APPID_HTTPSERVER,APP_ERR_LEVEL_ERROR,APP_RTSPSERVER_ERR_SESS_UNSUPPORT_MEDIATYPE )
#define HI_ERR_RTSPSERVER_ERR_SESS_NO_DATA                                    HI_APP_DEF_ERR(HI_APPID_HTTPSERVER,APP_ERR_LEVEL_ERROR,APP_RTSPSERVER_ERR_SESS_NO_DATA)



/*rtp  error code*/
#define HI_ERR_RTSPSERVER_ERR_RTP_DESTROY_FAIL                        HI_APP_DEF_ERR(HI_APPID_RTSPSERVER,APP_ERR_LEVEL_ERROR,APP_RTSPSERVER_ERR_RTP_DESTROY_FAIL)
#define HI_ERR_RTSPSERVER_ERR_RTP_CREATE_FAIL                           HI_APP_DEF_ERR(HI_APPID_RTSPSERVER,APP_ERR_LEVEL_ERROR,APP_RTSPSERVER_ERR_RTP_CREATE_FAIL)
#define HI_ERR_RTSPSERVER_ERR_RTP_TRANS_MODE                        HI_APP_DEF_ERR(HI_APPID_HTTPSERVER,APP_ERR_LEVEL_ERROR,APP_RTSPSERVER_ERR_RTP_TRANS_MODE)
#define HI_ERR_RTSPSERVER_ERR_RTP_PACKET_TYPE                       HI_APP_DEF_ERR(HI_APPID_HTTPSERVER,APP_ERR_LEVEL_ERROR,APP_RTSPSERVER_ERR_RTP_PACKET_TYPE)
#define HI_ERR_RTSPSERVER_ERR_RTP_TRANS_SEND                       HI_APP_DEF_ERR(HI_APPID_HTTPSERVER,APP_ERR_LEVEL_ERROR,APP_RTSPSERVER_ERR_RTP_TRANS_SEND)

/*rtcp  error code*/
#define HI_ERR_RTSPSERVER_ERR_RTCP_DESTROY_FAIL                        HI_APP_DEF_ERR(HI_APPID_RTSPSERVER,APP_ERR_LEVEL_ERROR,APP_RTSPSERVER_ERR_RTCP_DESTROY_FAIL)
#define HI_ERR_RTSPSERVER_ERR_RTCP_CREATE_FAIL                           HI_APP_DEF_ERR(HI_APPID_RTSPSERVER,APP_ERR_LEVEL_ERROR,APP_RTSPSERVER_ERR_RTCP_CREATE_FAIL)

/*write frame  error code*/
#define HI_ERR_RTSPSERVER_ERR_WRITE_FRAME                       HI_APP_DEF_ERR(HI_APPID_RTSPSERVER,APP_ERR_LEVEL_ERROR,APP_RTSPSERVER_ERR_WRITE_FRAME_FAIL)
#define HI_ERR_RTSPSERVER_ERR_MBUF_FULL                          HI_APP_DEF_ERR(HI_APPID_RTSPSERVER,APP_ERR_LEVEL_ERROR,APP_RTSPSERVER_ERR_MBUF_FULL)

/** @}*/  /** <!-- ==== RTSPSVR End ====*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __HI_RTSP_SERVER_ERR_H__ */
