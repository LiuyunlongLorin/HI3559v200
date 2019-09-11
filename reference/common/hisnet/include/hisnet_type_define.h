/****************************************************************************
*              Copyright 2004 - 2050, Hisilicon Tech. Co., Ltd.
*                           ALL RIGHTS RESERVED
* FileName: Hisnet_type_define.h
* Description: Describe parameter of HisnetAPI.
*
* History:
* Version   Date         Author     DefectNum    Description
* B010      2012-08-20   skf71347   NULL         Create this file.
* B020      2012-09-11   l00228447  NULL         Modify this file.
* B030
******************************************************************************/

#ifndef HISNET_TYPE_DEFINE_H
#define HISNET_TYPE_DEFINE_H

#include "hi_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */



/*************Event**********************/

#define HISNET_MAX_CAMERANAME       (40+1)      /*IPC Name Length*/
#define HISNET_MAX_CAMERATYPE       (40+1)      /*IPC Type Length*/
#define HISNET_MAX_SERIALNUM        (40+1)      /*IPC SerialNum*/
#define HISNET_MAX_SOFTVERSION      (64+1)      /*Software Verison Length */
#define HISNET_MAX_FIRMVERSION      (64+1)      /*Firmware Verison Length */
#define HISNET_MAX_TIMESIZE         (40+1)      /*Time Length*/
#define HISNET_MAX_WIFI_SSID_LEN    (32)      /*SSID*/
#define HISNET_MAX_USERNAME_LEN     (32+1)      /*UserName Length*/
#define HISNET_MAX_PASSWORD_LEN     (64)      /*Password Length*/
#define HISNET_MAX_ACT_LEN          10          /*Maximum Act Length in Commond*/

//#define   HISNET_SVR_RTN_SUCCESS                                  -1111
#define HISNET_SVR_RTN_ERROR                                    -2222
#define HISNET_SVR_REGFUNC_NULL                                 -3333
#define HISNET_SVR_CMD_INVALID                                  -4444
#define HISNET_SVR_ARGPARSER_ERROR                              -5555

typedef enum hisnet_DEVMODEL_E
{
    HISNET_DEVMODEL_PAL,
    HISNET_DEVMODEL_NTSC,
    HISNET_DEVMODEL_SECAM,
    HISNET_DEVMODEL_AUTO,
    HISNET_DEVMODEL_BUTT
} HISNET_DEVMODEL_E;

typedef struct hisnet_DEVICE_ATTR_S
{
    HI_CHAR aszDevModel[HISNET_MAX_CAMERANAME];
    HI_CHAR aszSoftwareVersion[HISNET_MAX_SOFTVERSION];
} HISNET_DEVICE_ATTR_S;

typedef enum hisnet_SDSTATUS_E
{
    HISNET_SDSTATUS_OUT,
    HISNET_SDSTATUS_MOUNT,
    HISNET_SDSTATUS_UMOUNT,
    HISNET_SDSTATUS_READONLY,
    HISNET_SDSTATUS_NOPARTITION,
    HISNET_SDSTATUS_BUTT
} HISNET_SDSTATUS_E;

typedef struct hisnet_SDSTATUS_S
{
    HISNET_SDSTATUS_E   enSdStatus;
    HI_S32 s32SDFreeSpace;
    HI_S32 s32TotalSpace;
    HI_S32 s32PartitionNum;
} HISNET_SDSTATUS_S;

typedef enum hisnet_TIMEFORMAT_E
{
    HISNET_TIMEFORMAT_DEFAULT,
    HISNET_TIMEFORMAT_WESTERN,
    HISNET_TIMEFORMAT_EASTERN,
    HISNET_TIMEFORMAT_BUTT
} HISNET_TIMEFORMAT_E;

typedef struct hisnet_TIME_ATTR_S
{
    HI_CHAR aszTime[HISNET_MAX_TIMESIZE];       /* eg. "20120608110426"*/
    HISNET_TIMEFORMAT_E enTimeFormat;
    HI_CHAR aszTimeZone[HISNET_MAX_TIMESIZE];
} HISNET_TIME_ATTR_S;

typedef struct hisnet_WIFI_ATTR_S
{
    HI_BOOL bEnable;
    HI_CHAR aszSSID[HISNET_MAX_WIFI_SSID_LEN];    /*WIFI SSID*/
    HI_U32 u32Channel;
    HI_U32 u32Links;
} HISNET_WIFI_ATTR_S;

typedef struct hisnet_REMOTEFILENAME_S
{
    HI_CHAR aszName[HI_APPCOMM_MAX_PATH_LEN];
} HISNET_REMOTEFILENAME_S;

typedef struct hisnet_REMOTEFILEINFO_S
{
    HI_U32  u32Size;/*Unit:Byte*/
    HI_U32  u32Duration;/*Unit: Second*/
    HI_CHAR aszPath[HI_APPCOMM_MAX_PATH_LEN];/* Filename with whole path*/
    HI_CHAR aszCreate[HISNET_MAX_TIMESIZE];/*Create Time, Format:2014/07/21 19:36:20*/
} FILEMNG_REMOTEFILEINFO_S;

typedef enum hisnet_SYS_SDCOMMAND_E
{
    HISNET_SDCOMMAND_MOUNT,
    HISNET_SDCOMMAND_UMOUNT,
    HISNET_SDCOMMAND_FORMAT,
    HISNET_SDCOMMAND_BUTT
} HISNET_SYS_SDCOMMAND_E;

typedef struct hisnet_SYS_SDCOMMAND_S
{
    HISNET_SYS_SDCOMMAND_E enSDCommand;
    HI_S32 s32Partition;
} HISNET_SYS_SDCOMMAND_S;

typedef enum hisnet_WORKMODE_E
{
    HISNET_WORKMODE_RECORD,
    HISNET_WORKMODE_RECORDLOOP,
    HISNET_WORKMODE_RECORDSLOW,
    HISNET_WORKMODE_RECORDLAPSE,
    HISNET_WORKMODE_RECORDSNAP,
    HISNET_WORKMODE_LAPSE,
    HISNET_WORKMODE_TIMER,
    HISNET_WORKMODE_IDLE,
    HISNET_WORKMODE_PHOTO,
    HISNET_WORKMODE_BUTT
} HISNET_WORKMODE_E;

typedef struct hisnet_BATTERY_STATUS_S
{
    HI_S32 s32Capacity;       /*0-100*/
    HI_BOOL bCharge;
    HI_BOOL bAC;              /*0. power not connected,1 power connected*/
} HISNET_BATTERY_STATUS_S;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif/*HISNET_TYPE_DEFINE_H*/

