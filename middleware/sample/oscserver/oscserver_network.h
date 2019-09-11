#include "hi_type.h"
#include "hi_mw_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */
#define OSC_MAX_RECV_BUFFER             (1024)
#define OSC_DEFAULT_MAX_FDNUM           (32)
#define OSC_INVALID_SOCKETFD            (-1)
#define OSC_DEFAULT_MAX_THD_NUM         (15)
#define OSC_INVALID_THID                (-1)

/*network*/
#define OSC_NET_TIMOUT_SEC              (0)
#define OSC_NET_TIMOUT_USEC             (10000)

/*get image*/
#define OSCSAM_DOWNLOAD_IMAGES_HEADER              (512)
/*image binary data len*/
#define OSCSAM_IMAGES_BINARAY_DATA_LEN             (1600)

typedef struct hiOSC_THREAD_ARG_S
{
    HI_MW_PTR OSCHandle;
    HI_MW_PTR pNetHandle;
} HiOSC_THREAD_ARG_S;

typedef HI_S32 (*onClientConnection)(HI_MW_PTR OSCHandle, HI_S32* s32LinkFd, HI_CHAR* pMsgBuff, HI_U32 u32MsgLen);

typedef struct hiOSC_NETWORK_CTX
{
    long  lListenThd;
    HI_S32  s32ListenSock;
    HI_U16  s32HttpPort;
    HI_BOOL bInListening;
    HI_CHAR aszFirstMsgBuff[OSC_MAX_RECV_BUFFER];
    HI_S32  s32ClientSockList[OSC_DEFAULT_MAX_FDNUM];
    long  lThdList[OSC_DEFAULT_MAX_THD_NUM];
    HI_S32  s32CurrentFd;
    onClientConnection connectCallback;
    HI_MW_PTR  s32ServerHandle;
} HiOSC_NETWORK_CTX;

typedef struct hiOSC_NET_S
{
    long*  plThreadId;
    HI_CHAR aszFirstMsgBuff[OSC_MAX_RECV_BUFFER];
    HI_MW_PTR  s32ServerHandle;
    HI_S32     s32MsgLen;
    HI_S32     s32CurrentFd;
} HI_OSC_NET_S;

HI_S32 OSCSVR_Create_Listener(HI_S32 s32HttpPort,  HI_MW_PTR* pNetHandle);
HI_S32 OSCSVR_Network_Start(HI_MW_PTR OSCHandle,  HI_MW_PTR pNetHandle);
HI_S32 OSCSVR_CloseSocket(HI_S32* ps32Socket);
HI_S32 OSCSVR_Create_Thread(HI_MW_PTR pNetHandle);
HI_VOID* OSCSVR_Network_Process(HI_MW_PTR pNetHandle);
HI_S32 OSCSVR_Listen_Start(HI_MW_PTR pNetHandle);
HI_S32 OSCSVR_Response_Send(HI_S32 s32WritSock, HI_CHAR* pszBuff, HI_U32 u32DataLen);
HI_S32 OSCSVR_CheckIsDownload(HI_CHAR* pcUrl, HI_S32 s32UrlLen);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
