/**
* Copyright (C), 2016-2030, Hisilicon Tech. Co., Ltd.
* All rights reserved.
*
* @file      hi_oscserver_com_exec.h
* @brief     oscserver com exec head file
* @author    HiMobileCam middleware develop team
* @date      2017.10.10
*/
#ifndef __HI_OSCSERVER_COM_EXEC_H__
#define __HI_OSCSERVER_COM_EXEC_H__

#include <pthread.h>
#include "hi_list.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */
#define  OSCSVR_HTTP_TRANS_TIMEVAL_SEC   (5)
#define  OSCSVR_HTTP_TRANS_TIMEVAL_USEC  (0)
#define  OSCSVR_INIT_COMMAND_ID          (1)
#define  OSCSVR_INIT_SESSION_ID          (1)
#define  OSCSVR_INIT_FINGERPRINT_ID      (1)

#define  OSC_SESSIONID_MAX_LEN           (12)
#define  OSC_INVALID_THREAD_ID           (-1)
#define  OSCSVR_CHECKFORUPDATE_WAIT_TIME (60)

#define  OSC_IP_MAX_LEN                  (24)
#define  OSC_COMM_NAME_MAX_LEN           (32)
#define  OSC_SVR_COMMAND_ID_LEN          (12)

#define  OSC_IMAGE_BINARY_DATA_LEN       (2048)

#define  OSC_LIST_FILE_MIN_LEN           (128)
/*command id max ache num*/
#define  OSC_COMMANDID_STORAGE_MAX_NUM   (3)

#define HI_OSC_INVALID_SOCKET            (-1)
#define HI_OSC_COMMAND_NAME_MAX_LEN      (32)

/*OSC boundary*/
#define  OSC_GETLIVEPREVIEW_BOUNDARY     "---osclivepreview---"

#define OSC_LOCK(mutex) \
    do \
    { \
        (HI_VOID)pthread_mutex_lock(&mutex);  \
    }while(0)

#define OSC_UNLOCK(mutex)  \
    do \
    { \
        (HI_VOID)pthread_mutex_unlock(&mutex); \
    }while(0)

typedef HI_CHAR HI_IP_ADDR[64];
typedef HI_U16  HI_PORT;
typedef HI_S32  HI_SOCKET;

/**rec command callback set*/
typedef struct hiOSC_REGCOMMAND_CB_S
{
    HI_OSC_TAKEPICTURE_FN_PTR pfnTakePicture;        /**<callback  take picture*/
    HI_OSC_GETLIVEPREVIEW_FN_PTR pfnGetLivePreview;  /**<callback  get live preview*/
    HI_OSC_PROCESSPICTURE_FN_PTR pfnProcessPitcure;  /**<callback  process picture*/
    HI_OSC_STARTCAPTURE_FN_PTR pfnStartCapture;      /**<callback  start capture*/
    HI_OSC_STOPCAPTURE_FN_PTR pfnStopCapture;        /**<callback  stop capture*/
    HI_OSC_LISTFILE_FN_PTR pfnListFiles;              /**<callback  list file*/
    HI_OSC_LISTIMAGE_FN_PTR pfnListImage;            /**<callback  list image*/
    HI_OSC_GETMETADATA_FN_PTR pfnGetMetaData;        /**<callback  get meta data*/
    HI_OSC_DELETE_FN_PTR pfnDelete;                  /**<callback  delete*/
    HI_OSC_SETOPTION_FN_PTR pfnSetOption;            /**<callback  set option*/
    HI_OSC_GETOPTION_FN_PTR pfnGetOption;            /**<callback  get option*/
    HI_OSC_SWITCHWIFI_FN_PTR pfnSwitchWifi;          /**<callback  switch wifi*/
    HI_OSC_UPLOADFILE_FN_PTR pfnUploadFile;          /**<callback  upload file*/
    HI_OSC_RESET_FN_PTR pfnReset;                    /**<callback  reset*/
    HI_OSC_PRIVATECOM_FN_PTR pfnPrivateCom;          /**<callback  private command*/
} HI_OSC_REGCOMMAND_CB_S;

/** osc api callback set*/
typedef struct hiOSC_REGAPI_CB_S
{
    HI_OSC_INFO_FN_PTR pfnOSCSVRInfoCB;
    HI_OSC_STATE_FN_PTR pfnOSCSVRStateCB;
} HI_OSC_REGAPI_CB_S;

/*sess level2 struct*/
typedef struct hiOSC_SESS_LEVEL2_S
{
    HI_CHAR aszClientIP[OSC_IP_MAX_LEN];
    HI_S32  s32SessTimeOut;                     /*the time before session destroy*/
    time_t FirstTick;
    time_t CurrentTick;
    HI_BOOL bCapture;
    HI_OSCSERVER_STATE_S stOSCState;
} HI_OSC_SESS_LEVEL2_S;

/*sess level1 struct*/
typedef struct hi_OSC_SESS_LEVEL1_S
{
    /*used to maintain link list*/
    HI_CHAR aszClientIP[OSC_IP_MAX_LEN];
    HI_CHAR aszSessionId[OSC_SESSIONID_MAX_LEN];
    HI_S32  s32SessTimeOut;                     /*the time before session destroy*/
    time_t FirstTick;
    time_t CurrentTick;
    HI_BOOL bCapture;
    HI_OSCSERVER_STATE_S stOSCState;
} HI_OSC_SESS_LEVEL1_S;

/** OSC context object type*/
typedef struct _Hi_OSC_SERVER_CTX_S
{
    List_Head_S pRegCommandlist;             /*Reg Command list*/
    pthread_mutex_t RegCommandlistLock;      /*the lock of list*/
    List_Head_S pCommStatuslist;             /*command status list*/
    pthread_mutex_t CommStatusLock;          /*the lock of command status list*/
    HI_OSC_SESS_LEVEL1_S* pstLevel1Sess;     /*ache level1 session info*/
    pthread_mutex_t Level1SessLock;          /*the lock of sess1*/
    HI_OSC_SESS_LEVEL2_S* pstLevel2Sess;     /*ache level2 session info*/
    pthread_mutex_t Level2SessLock;          /*the lock of sess1*/
    HI_BOOL bExitLooper;                       /*the flag of exit */
    HI_S32 s32TimeOut;                       /*the session alive time*/
    HI_S32 s32ListFiles;                     /*the number of max list num*/
    pthread_mutex_t StateLock;               /*the lock of ChangeState*/
    HI_OSCSERVER_STATE_S stPreState;         /*ache the state of /osc/state results*/
    HI_OSCSERVER_STATE_S stChangeState;      /*the struct used to record the change state*/
    HI_U32 u32FingerId;                      /*fingerid number*/
    HI_U32 u32SessionId;                     /*sessionid number*/
    HI_U32 u32CommandId;                     /*commandid number*/
    HI_U32 u32ThrottleTimeout;               /*the time suggest checkforupdate before next time*/
    HI_BOOL bExclusiveUseFlag;               /*the camera whether is in Exclusive Use*/
    HI_OSC_REGAPI_CB_S stRegApiCB;           /*API callback struct*/
    HI_OSC_REGCOMMAND_CB_S stRegCommandCB;   /*command callback struct*/
} HI_OSC_SERVER_CTX_S;

#define OSCSVR_CHECK_NULL_ERROR(condition) \
    do \
    { \
        if(condition == NULL) \
        { \
            return HI_ERR_OSCSERVER_NULL_PTR; \
        }\
    }while(0)

/*sess level1 node struct*/
typedef struct hiOSC_COMM_STATUS_NODE_S
{
    /*used to maintain link list*/
    List_Head_S  listPtr;
    HI_OSC_COMM_STATUS_E enComStatus;
    HI_CHAR aszCommandName[OSC_COMM_NAME_MAX_LEN];
    HI_CHAR aszCommandID[OSC_SVR_COMMAND_ID_LEN];
    HI_DOUBLE dCompletion;
    HI_CHAR aszResults[HI_OSC_FILEURL_MAX_NUM][HI_OSC_FILEURL_MAX_LEN];
    HI_OSC_SERVER_LEVEL_E enClientVersion;
} HI_OSC_COMM_STATUS_NODE_S;

typedef struct hiOSC_REGCOMMAND_NODE_S
{
    /*used to maintain link list*/
    List_Head_S  listPtr;
    HI_OSC_API_COMMAND_E enOSCRegKey;
    HI_CHAR aszCommandName[HI_OSC_COMMAND_NAME_MAX_LEN];
    HI_VOID* pfnEventCB;
} HI_OSC_REGCOMMAND_NODE_S;

/*API: /osc/checkforUpdates*/
typedef struct hiOSCSERVER_CHECKFORUPDATES_S
{
    HI_CHAR   aszInFingerPrint[HI_OSC_FINGER_PRINT_MAX_LEN];             /*the value of the input fingerprint*/
    HI_S32    s32WaitTimeout;
} HI_OSCSERVER_CHECKFORUPDATES_S;

HI_S32 OSCSVR_Create_ClientLevel2Session(HI_OSC_SERVER_CTX_S* pstServerCtx);
HI_S32 OSCSVR_Create_ClientLevel1Session(HI_OSC_SERVER_CTX_S* pstServerCtx, HI_CHAR* pszClientIP, HI_S32 s32SessTimeOut, HI_OSC_SESS_LEVEL1_S* pstSessionInfo);
HI_S32 OSCSVR_Destroy_ClientLevel2Session(HI_OSC_SERVER_CTX_S* pstServerCtx, HI_OSC_SESS_LEVEL2_S* pstSessionInfo);
HI_S32 OSCSVR_Destroy_ClientLevel1Session(HI_OSC_SERVER_CTX_S* pstServerCtx, HI_OSC_SESS_LEVEL1_S* pstSessionInfo);
HI_S32 OSCSVR_Process_StartSession(HI_MW_PTR pOSC, HI_S32* ps32SocketFd, HI_CHAR* pClientReq, HI_U32 u32ReqLen);
HI_S32 OSCSVR_Process_CloseSession(HI_MW_PTR pOSC, HI_S32* ps32SocketFd, HI_CHAR* pClientReq, HI_U32 u32ReqLen);
HI_S32 OSCSVR_Process_UpdateSession(HI_MW_PTR pOSC, HI_S32* ps32SocketFd, HI_CHAR* pClientReq, HI_U32 u32ReqLen);
HI_S32 OSCSVR_Process_TakePicture(HI_MW_PTR pOSC, HI_S32* ps32SocketFd, HI_CHAR* pClientReq, HI_U32 u32ReqLen);
HI_S32 OSCSVR_Process_SetOption(HI_MW_PTR pOSC, HI_S32* ps32SocketFd, HI_CHAR* pClientReq, HI_U32 u32ReqLen);
HI_S32 OSCSVR_Process_GetOption(HI_MW_PTR pOSC, HI_S32* ps32SocketFd, HI_CHAR* pClientReq, HI_U32 u32ReqLen);
HI_S32 OSCSVR_Process_Reset(HI_MW_PTR pOSC, HI_S32* ps32SocketFd, HI_CHAR* pClientReq, HI_U32 u32ReqLen);
HI_S32 OSCSVR_Process_StartCapture(HI_MW_PTR pOSC, HI_S32* ps32SocketFd, HI_CHAR* pClientReq, HI_U32 u32ReqLen);
HI_S32 OSCSVR_Process_StopCapture(HI_MW_PTR pOSC, HI_S32* ps32SocketFd, HI_CHAR* pClientReq, HI_U32 u32ReqLen);
HI_S32 OSCSVR_Process_GetLivePreview(HI_MW_PTR pOSC, HI_S32* ps32SocketFd, HI_CHAR* pClientReq, HI_U32 u32ReqLen);
HI_S32 OSCSVR_Process_PrivateCommand(HI_MW_PTR pOSC, HI_S32* ps32SocketFd, HI_CHAR* pClientReq, HI_U32 u32ReqLen);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /*__HI_OSCSERVER_COM_EXEC_H__*/
