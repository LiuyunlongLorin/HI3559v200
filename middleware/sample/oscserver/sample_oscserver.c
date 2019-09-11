#include <sys/prctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <sys/stat.h>
#include <signal.h>
/*osc component*/
#include "oscserver_network.h"
#include "sample_oscserver.h"
#include "hi_osc_server.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */
HI_OSC_CONFIG_S g_stOSCConf;
static HI_CHAR s_aszCaptureMode[HI_OSC_OPTION_CAPTURE_MODE_LEN] = {0};
static HI_S32 s_s32Finger = 1;
static HI_MW_PTR s_Handle = NULL;
static HI_BOOL s_bVideo = HI_FALSE;
static HI_OSC_STATE_E s_enOSCState = HIOSC_STATE_SERVICES_READY;
static HI_BOOL s_bLooper = HI_TRUE;
HI_OSCSAMPLE_FILE_S g_stListFiles[OSCSAMPLE_LISTFILES_NUMBER];
pthread_mutex_t g_OSCFILELock;          /*the lock of sess1*/
HI_S32 g_s32ListFile = 0;
HI_CHAR g_aszCommandId[OSCSAMPLE_COMMAND_ID_MAX_LEN] = {0};
HI_CHAR g_aszTakePicture[HI_OSC_FILEURL_MAX_NUM][HI_OSC_FILEURL_MAX_LEN];

HI_S32 OSCSVR_WriteImage(HI_CHAR* Frame, HI_S32 s32FrameLen)
{
    FILE* ImageFile = NULL;
    HI_CHAR FileUrl[HI_OSC_FILEURL_MAX_LEN] = {0};
    HI_CHAR aszRealFileUrl[HI_OSC_FILEURL_MAX_LEN] = {0};
    static HI_S32 s_s32ImageNum = 0;

    if (NULL == Frame)
    {
        printf("OSCSVR_WriteImage input is NULL!!\n");
        return HI_FAILURE;
    }

    snprintf_s(FileUrl, HI_OSC_FILEURL_MAX_LEN, HI_OSC_FILEURL_MAX_LEN - 1, "/home/osc/live/Live_%04d.jpg", s_s32ImageNum);
    s_s32ImageNum ++;

    if (NULL == realpath(FileUrl, aszRealFileUrl))
    {
        printf("the File url:%s is err!!\n", FileUrl);
        return HI_FAILURE;
    }

    /*get image file size!!*/
    ImageFile = fopen(aszRealFileUrl, "a+");

    if ( NULL == ImageFile)
    {
        printf("fopen Image file fail:%s!!\n", FileUrl);
        return HI_FAILURE;
    }

    (HI_VOID)fwrite(Frame, s32FrameLen, 1, ImageFile);
    (HI_VOID)fclose(ImageFile);
    ImageFile = NULL;

    return HI_SUCCESS;
}

HI_VOID* OSCSVR_Thread_Takepicture(void* args)
{
    HI_MW_PTR OSCHandle = NULL;

    OSCHandle = args;
    /*test the take picture time*/
    sleep(2);

    if (HI_SUCCESS != HI_OSCSVR_Update_CommandStatus(OSCHandle, g_aszCommandId, HIOSC_COMM_STATUS_DONE, g_aszTakePicture))
    {
        printf("Add the takepiture result into server is error!!\r\n");
        s_enOSCState = HIOSC_STATE_SERVICES_READY;
        return HI_NULL_PTR;
    }

    s_enOSCState = HIOSC_STATE_SERVICES_READY;
    return HI_NULL_PTR;
}

HI_S32 OSCSVR_Listener_RegisterCallback(HI_MW_PTR u32Nethandle, onClientConnection onConnection, HI_MW_PTR u32OscHandle)
{
    HiOSC_NETWORK_CTX* pListenCtx = (HiOSC_NETWORK_CTX*)u32Nethandle;
    pListenCtx->connectCallback = onConnection;
    pListenCtx->s32ServerHandle = u32OscHandle;

    return HI_SUCCESS;
}

HI_S32 OSCSVR_Get_FileSize(HI_CHAR* pcUrl, HI_S32* ps32FileNum)
{
    FILE* ImageFile = NULL;
    HI_CHAR aszRealFileUrl[HI_OSC_FILEURL_MAX_LEN] = {0};

    if (NULL == realpath(pcUrl, aszRealFileUrl))
    {
        printf("the File url:%s is err!!\n", pcUrl);
        return HI_FAILURE;
    }

    /*get image file size!!*/
    ImageFile = fopen(aszRealFileUrl, "rb");

    if (NULL == ImageFile)
    {
        printf("fopen file:%s err!!\n", aszRealFileUrl);
        return HI_FAILURE;
    }

    (HI_VOID)fseek(ImageFile, 0, SEEK_END);
    *ps32FileNum = ftell(ImageFile);
    (HI_VOID)fseek(ImageFile, 0, SEEK_SET);
    (HI_VOID)fclose(ImageFile);
    ImageFile = NULL;

    return HI_SUCCESS;
}

HI_S32 OSCSVR_Update_ListFiles(HI_OSCSAMPLE_FILE_S* pstListFiles)
{
    HI_S32 s32Looper = 0;

    OSCSAM_LOCK(g_OSCFILELock);

    for (s32Looper = 0; s32Looper < OSCSAMPLE_LISTFILES_NUMBER; s32Looper ++)
    {
        if (0 == strlen(g_stListFiles[s32Looper].aszFileName))
        {
            strncpy_s(g_stListFiles[s32Looper].aszFileType, HI_OSC_LISTFILE_FILETYPE_LEN, pstListFiles->aszFileType, HI_OSC_LISTFILE_FILETYPE_LEN);
            strncpy_s(g_stListFiles[s32Looper].aszFileName, HI_OSC_LISTFILE_FILE_NAME, pstListFiles->aszFileName, HI_OSC_LISTFILE_FILE_NAME);
            strncpy_s(g_stListFiles[s32Looper].aszFileURL, HI_OSC_LISTFILE_FILE_URL, pstListFiles->aszFileURL, HI_OSC_LISTFILE_FILE_URL);
            strncpy_s(g_stListFiles[s32Looper].aszDownURL, HI_OSC_LISTFILE_FILE_URL, pstListFiles->aszDownURL, HI_OSC_LISTFILE_FILE_URL);
            g_stListFiles[s32Looper].s32FileSize = pstListFiles->s32FileSize;   /*byte*/
            strncpy_s(g_stListFiles[s32Looper].aszDateTimeZone, HI_OSC_LISTFILE_DATE_TIME, pstListFiles->aszDateTimeZone, HI_OSC_LISTFILE_DATE_TIME);
            g_stListFiles[s32Looper].dLat = pstListFiles->dLat;
            g_stListFiles[s32Looper].dLng = pstListFiles->dLng;
            g_stListFiles[s32Looper].s32Height = pstListFiles->s32Height;
            g_stListFiles[s32Looper].s32Width = pstListFiles->s32Width;
            strncpy_s(g_stListFiles[s32Looper].aszThumbnail, HI_OSC_LISTFILE_THUMB_ENCODE, pstListFiles->aszThumbnail, HI_OSC_LISTFILE_THUMB_ENCODE);
            g_stListFiles[s32Looper].bIsProcessed = pstListFiles->bIsProcessed;
            memset_s(g_stListFiles[s32Looper].aszPreviewUrl, HI_OSC_LISTFILE_PREVIEW_URL, 0x00, HI_OSC_LISTFILE_PREVIEW_URL);
            g_stListFiles[s32Looper].bDownLoad = pstListFiles->bDownLoad;

            g_s32ListFile ++;
            break;
        }
    }

    OSCSAM_UNLOCK(g_OSCFILELock);

    return HI_SUCCESS;
}

HI_S32 OSCSVR_Delete_ListFiles(HI_CHAR* pcFileUrl)
{
    HI_S32 s32Looper = 0;
    HI_BOOL bFlag = HI_FALSE;

    OSCSAM_LOCK(g_OSCFILELock);

    for (s32Looper = 0; s32Looper < OSCSAMPLE_LISTFILES_NUMBER; s32Looper ++)
    {
        printf("delete ListFiles file: %s\n", g_stListFiles[s32Looper].aszDownURL);

        if (0 == strncmp(g_stListFiles[s32Looper].aszDownURL, pcFileUrl, strlen(pcFileUrl) + 1))
        {
            memset_s(&g_stListFiles[s32Looper], sizeof(HI_OSCSAMPLE_FILE_S), 0x00, sizeof(HI_OSCSAMPLE_FILE_S));
            bFlag = HI_TRUE;

            g_s32ListFile --;
            break;
        }
    }

    OSCSAM_UNLOCK(g_OSCFILELock);

    if (!bFlag)
    {
        printf("delete ListFiles not find the url: %s\n", pcFileUrl);
    }

    return HI_SUCCESS;
}

HI_S32 OSCSVR_Init_ListFiles(HI_S32 s32FileNum)
{
    HI_S32 s32Looper = 0;
    HI_S32 s32FileSize = 0;

    OSCSAM_LOCK(g_OSCFILELock);

    for (s32Looper = 0; s32Looper < OSCSAMPLE_LISTFILES_NUMBER; s32Looper ++)
    {
        memset_s(&g_stListFiles[s32Looper], sizeof(HI_OSCSAMPLE_FILE_S), 0x00, sizeof(HI_OSCSAMPLE_FILE_S));
    }

    for (s32Looper = 0; s32Looper < s32FileNum; s32Looper ++)
    {
        snprintf_s(g_stListFiles[s32Looper].aszFileType, HI_OSC_LISTFILE_FILETYPE_LEN, HI_OSC_LISTFILE_FILETYPE_LEN - 1, "image");
        snprintf_s(g_stListFiles[s32Looper].aszFileName, HI_OSC_LISTFILE_FILE_NAME, HI_OSC_LISTFILE_FILE_NAME - 1, "%d.jpg", s32Looper + 1);
        snprintf_s(g_stListFiles[s32Looper].aszFileURL, HI_OSC_LISTFILE_FILE_URL, HI_OSC_LISTFILE_FILE_URL - 1, "http://192.168.1.1/home/osc/files/%s", g_stListFiles[s32Looper].aszFileName);
        snprintf_s(g_stListFiles[s32Looper].aszDownURL, HI_OSC_LISTFILE_FILE_URL, HI_OSC_LISTFILE_FILE_URL - 1, "/home/osc/files/%s", g_stListFiles[s32Looper].aszFileName);
        (HI_VOID)OSCSVR_Get_FileSize(g_stListFiles[s32Looper].aszDownURL, &s32FileSize);
        g_stListFiles[s32Looper].s32FileSize = s32FileSize;   /*byte*/
        snprintf_s(g_stListFiles[s32Looper].aszDateTimeZone, HI_OSC_LISTFILE_DATE_TIME, HI_OSC_LISTFILE_DATE_TIME- 1, "2018:01:11 20:11:%02d+08:00", s32Looper);
        g_stListFiles[s32Looper].dLat = 0.0f;
        g_stListFiles[s32Looper].dLng = 0.0f;
        g_stListFiles[s32Looper].s32Height = 400;
        g_stListFiles[s32Looper].s32Width = 600;
        snprintf_s(g_stListFiles[s32Looper].aszThumbnail, HI_OSC_LISTFILE_THUMB_ENCODE, HI_OSC_LISTFILE_THUMB_ENCODE - 1, "ENCODEDSTRING");
        g_stListFiles[s32Looper].bIsProcessed = HI_TRUE;
        memset_s(g_stListFiles[s32Looper].aszPreviewUrl, HI_OSC_LISTFILE_PREVIEW_URL, 0x00, HI_OSC_LISTFILE_PREVIEW_URL);
        g_stListFiles[s32Looper].bDownLoad = HI_FALSE;
    }

    g_s32ListFile = s32FileNum;
    OSCSAM_UNLOCK(g_OSCFILELock);

    return HI_SUCCESS;
}

HI_S32 OSCSVR_GetStreamData(HI_CHAR* pszStreamTmp, HI_S32 s32BuffLen, HI_S32* ps32StreamByte)
{
    FILE* fStream = NULL;
    HI_CHAR FileUrl[HI_OSC_FILEURL_MAX_LEN] = {0};
    HI_CHAR aszRealFileUrl[HI_OSC_FILEURL_MAX_LEN] = {0};

    if (NULL == pszStreamTmp)
    {
        printf("OSCSVR_GetStreamData input NULL!!\r\n");
        return HI_FAILURE;
    }

    snprintf_s(FileUrl, HI_OSC_FILEURL_MAX_LEN, HI_OSC_FILEURL_MAX_LEN - 1, "/home/stream_chn0.mjp");

    if (NULL == realpath(FileUrl, aszRealFileUrl))
    {
        printf("the File url:%s is err!!\n", FileUrl);
        return HI_FAILURE;
    }

    fStream = fopen(aszRealFileUrl, "r");

    if (NULL == fStream)
    {
        printf("open Motion Jpeg stream fail!!\r\n");
        return HI_FAILURE;
    }

    *ps32StreamByte = fread(pszStreamTmp, 1, s32BuffLen - 1,  fStream);

    (HI_VOID)fclose(fStream);
    return HI_SUCCESS;
}

HI_S32 OSCSVR_GetFrameData(HI_CHAR* pszStreamTmp, HI_S32* ps32StreamLen, HI_S32* ps32FrameEnd, HI_CHAR* pszFrameData, HI_S32* ps32FrameLen)
{
    HI_S32 s32Looper = 0;
    HI_S32 s32FrameBegin = -1;
    HI_S32 s32FrameEnd = -1;
    HI_BOOL bBegin = HI_FALSE;

    if (NULL == pszStreamTmp || NULL == pszFrameData || NULL == ps32FrameLen)
    {
        printf("OSCSVR_GetFrameData input NULL!!\r\n");
        return HI_FAILURE;
    }

    for (s32Looper = 0; s32Looper < *ps32StreamLen; s32Looper++)
    {
        if ((pszStreamTmp[s32Looper] == 0xFF) && (pszStreamTmp[s32Looper + 1] == 0xD8))
        {
            s32FrameBegin = s32Looper;
            bBegin = HI_TRUE;
        }

        if ((pszStreamTmp[s32Looper] == 0xD9) && (pszStreamTmp[s32Looper - 1] == 0xFF))
        {
            s32FrameEnd = s32Looper;

            if ((s32FrameEnd > s32FrameBegin) && (bBegin == HI_TRUE))
            {
                break;
            }
        }

    }

    *ps32FrameEnd = s32FrameEnd;

    if ((s32FrameEnd == -1) || (s32FrameBegin == -1) || (s32FrameEnd < s32FrameBegin))
    {
        return HI_FAILURE;
    }

    *ps32FrameLen = s32FrameEnd - s32FrameBegin + 1;
    memcpy_s(pszFrameData, SAMPLE_OSC_STREAM_FRAME_LEN, pszStreamTmp + s32FrameBegin, *ps32FrameLen);

    return HI_SUCCESS;
}

HI_S32 OSCSVR_Start(HI_MW_PTR OSCHandle,  HI_MW_PTR pNetHandle)
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = OSCSVR_Listener_RegisterCallback(pNetHandle, (onClientConnection)HI_OSCSVR_HTTP_DistribLink, OSCHandle);

    if (HI_SUCCESS != s32Ret)
    {
        printf("Reg OSC net callback fail!!  s32Ret:%d\n", s32Ret);
        return HI_FAILURE;
    }

    s32Ret = OSCSVR_Listen_Start(pNetHandle);

    if (HI_SUCCESS != s32Ret)
    {
        printf("Start OSC network fail!!  s32Ret:%d\n", s32Ret);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 OSCSVR_Listener_Destroy(HI_MW_PTR* pNetHandle)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 s32Looper = 0;
    HiOSC_NETWORK_CTX* pListenCtx = (HiOSC_NETWORK_CTX*)(*pNetHandle);

    if (!pListenCtx)
    {
        printf("the listen has not create!!\n");
        return HI_FAILURE;
    }

    /*set the listen svr stop flag*/
    for (s32Looper = 0; s32Looper < OSC_DEFAULT_MAX_THD_NUM; s32Looper ++)
    {
        printf(">listen thread id>:%d, %ld\n", s32Looper, pListenCtx->lThdList[s32Looper]);

        if (pListenCtx->lThdList[s32Looper] != OSC_INVALID_THID)
        {
            s32Ret = pthread_join((pthread_t)pListenCtx->lThdList[s32Looper], NULL);

            if (s32Ret != 0)
            {
                printf("destroy process thread error\n");
                return HI_FAILURE;
            }

            pListenCtx->lThdList[s32Looper] = OSC_INVALID_THID;
        }
    }

    /*set the listen svr stop flag*/
    if (pListenCtx->bInListening)
    {
        pListenCtx->bInListening = HI_FALSE;
        s32Ret = pthread_join((pthread_t)pListenCtx->lListenThd, NULL);

        if (s32Ret != 0)
        {
            printf("destroy network listener thread error\n");
            return HI_FAILURE;
        }
    }

    usleep(500000);

    if (pListenCtx->s32ListenSock != OSC_INVALID_SOCKETFD)
    {
        close(pListenCtx->s32ListenSock) ;
        pListenCtx->s32ListenSock = OSC_INVALID_SOCKETFD;
    }

    memset_s(pListenCtx, sizeof(HiOSC_NETWORK_CTX), 0x00, sizeof(HiOSC_NETWORK_CTX));
    free(pListenCtx);
    pListenCtx = NULL;
    *pNetHandle = NULL;

    return HI_SUCCESS;
}

/*API: /osc/info*/
HI_S32 OSCSVR_API_Info(HI_OSCSERVER_INFO_S* pstOscInfo)
{
    HI_CHAR aszManufacturer[OSCSVR_MANUFACTYRER_MAX_LEN] = "RICOH";
    HI_CHAR aszModel[OSCSVR_MODEL_MAX_LEN] = "RICOH THETA S";
    HI_CHAR aszSerialNumber[OSCSVR_SERIAL_NUMBER_MAX_LEN] = "01106502";          /*the serial number of camera*/
    HI_CHAR aszFirmwareVersion[OSCSVR_FIRMWARE_VERSION_MAX_LEN] = "01.82";    /*current firmware version*/

    if (NULL == pstOscInfo)
    {
        printf("\r\nthe info input is null!!\r\n");
        return HI_FAILURE;
    }

    memset_s(pstOscInfo, sizeof(HI_OSCSERVER_INFO_S), 0x00, sizeof(HI_OSCSERVER_INFO_S));

    strncpy_s(pstOscInfo->aszManufacturer, HI_OSCSVR_MANUFACTYRER_MAX_LEN, aszManufacturer, strlen(aszManufacturer) + 1);
    strncpy_s(pstOscInfo->aszModel, HI_OSCSVR_MODEL_MAX_LEN, aszModel, strlen(aszModel) + 1);
    strncpy_s(pstOscInfo->aszSerialNumber, HI_OSCSVR_SERIAL_NUMBER_MAX_LEN, aszSerialNumber, strlen(aszSerialNumber) + 1);
    strncpy_s(pstOscInfo->aszFirmwareVersion, HI_OSCSVR_FIRMWARE_VERSION_MAX_LEN, aszFirmwareVersion, strlen(aszFirmwareVersion) + 1);
    snprintf_s(pstOscInfo->aszSupportURL, HI_OSCSVR_SUPPORT_URL_VERSION_MAX_LEN, HI_OSCSVR_SUPPORT_URL_VERSION_MAX_LEN - 1, "https://theta360.com/en/support/");
    pstOscInfo->bGps = HI_FALSE;
    pstOscInfo->bGyro = HI_FALSE;
    pstOscInfo->s32Uptime = 60;
    pstOscInfo->s32Httpport = 80;
    pstOscInfo->s32HttpUpdatesport = 80;
    pstOscInfo->s32HttpsUpdatesport = 0;

    return HI_SUCCESS;
}

HI_S32 OSCSVR_API_State(HI_OSCSERVER_STATE_S* pstOscState)
{
    HI_CHAR aszStorageUri[HI_OSCSVR_STORAGE_URL_MAX_LEN] = {0};
    HI_CHAR aszFingerPrint[HI_OSC_FINGER_PRINT_MAX_LEN] = {0};

    if (NULL == pstOscState)
    {
        printf("\r\nthe state input is null!!\r\n");
        return HI_FAILURE;
    }

    memset_s(pstOscState, sizeof(HI_OSCSERVER_STATE_S), 0x00, sizeof(HI_OSCSERVER_STATE_S));

    snprintf_s(aszStorageUri, HI_OSCSVR_STORAGE_URL_MAX_LEN, HI_OSCSVR_STORAGE_URL_MAX_LEN - 1, "http://192.168.1.1/home/osc/files");
    snprintf_s(aszFingerPrint, HI_OSC_FINGER_PRINT_MAX_LEN, HI_OSC_FINGER_PRINT_MAX_LEN - 1, "FIG_%04d", s_s32Finger);

    s_s32Finger ++;
    strncpy_s(pstOscState->aszFingerPrint, HI_OSC_FINGER_PRINT_MAX_LEN, aszFingerPrint, HI_OSC_FINGER_PRINT_MAX_LEN);
    strncpy_s(pstOscState->aszStorageUri, HI_OSCSVR_STORAGE_URL_MAX_LEN, aszStorageUri, strlen(aszStorageUri) + 1);
    pstOscState->dBatteryLevel = 0.3f;
    pstOscState->bStorageChanged = HI_FALSE;
    pstOscState->hVendorSpecific = NULL;

    return HI_SUCCESS;
}

HI_S32 OSCSVR_Command_Takepicture(HI_MW_PTR OSCHandle, HI_OSC_PATH_TYPE_E enPathType, HI_CHAR* pszCommandId, HI_BOOL* pbComDisabled)
{
    static HI_U32 u32CommnadId = 1;
    HI_CHAR aszFileUrl[HI_OSC_FILEURL_MAX_LEN] = {0};
    HI_CHAR aszResults[HI_OSC_FILEURL_MAX_NUM][HI_OSC_FILEURL_MAX_LEN];
    HI_S32 s32Looper = 0;
    HI_S32 s32Rest = 0;
    HI_CHAR aszName[OSCSAMPLE_FILES_NAME_MAX_LEN] = {0};
    HI_CHAR aszCommand[OSCSAMPLE_FILES_COMMAND_MAX_LEN] = {0};
    HI_OSCSAMPLE_FILE_S* pstListFiles = NULL;
    HI_S32 s32FileSize = 0;
    pthread_t lTakePictureThId;

    s_enOSCState = HIOSC_STATE_SERVICES_UNABLE;

    if ((NULL == pszCommandId) || (NULL == pbComDisabled))
    {
        printf("\r\nthe takepicture command input is null!!\r\n");
        return HI_FAILURE;
    }

    for (s32Looper = 0; s32Looper < HI_OSC_FILEURL_MAX_NUM; s32Looper ++)
    {
        memset_s(aszResults[s32Looper], HI_OSC_FILEURL_MAX_LEN, 0x00, HI_OSC_FILEURL_MAX_LEN);
    }

    *pbComDisabled = HI_FALSE;

    if (HI_TRUE == *pbComDisabled)
    {
        snprintf_s(aszResults[0], HI_OSC_FILEURL_MAX_LEN, HI_OSC_FILEURL_MAX_LEN - 1, "the command is forbidden!");

        if (HI_SUCCESS != HI_OSCSVR_Update_CommandStatus(OSCHandle, pszCommandId, HIOSC_COMM_STATUS_ERROR, aszResults))
        {
            printf("Add the takepiture result into server is error!!\r\n");
            return HI_FAILURE;
        }

        return HI_SUCCESS;
    }

    pstListFiles = (HI_OSCSAMPLE_FILE_S*)malloc(sizeof(HI_OSCSAMPLE_FILE_S));

    if (NULL == pstListFiles)
    {
        printf("take picture malloc fail!!\r\n");
        return HI_FAILURE;
    }

    memset_s(pstListFiles, sizeof(HI_OSCSAMPLE_FILE_S), 0x00, sizeof(HI_OSCSAMPLE_FILE_S));

    s32Rest = u32CommnadId % 2;

    if (HI_OSC_ABSOLUTE_PATH == enPathType)
    {
        switch (s32Rest)
        {
            case 0:
                snprintf_s(aszName, OSCSAMPLE_FILES_NAME_MAX_LEN, OSCSAMPLE_FILES_NAME_MAX_LEN - 1, "/home/osc/files/snap_%d.jpg", u32CommnadId);
                snprintf_s(aszCommand, OSCSAMPLE_FILES_COMMAND_MAX_LEN, OSCSAMPLE_FILES_COMMAND_MAX_LEN - 1, "cp /home/osc/files/snap_a.jpg %s", aszName);
                system(aszCommand);
                snprintf_s(aszFileUrl, HI_OSC_FILEURL_MAX_LEN, HI_OSC_FILEURL_MAX_LEN - 1, "http://192.168.1.1%s", aszName);
                snprintf_s(pstListFiles->aszFileName, HI_OSC_LISTFILE_FILE_NAME, HI_OSC_LISTFILE_FILE_NAME - 1, "snap_%d.jpg", u32CommnadId);
                OSCSVR_Get_FileSize(aszName, &s32FileSize);
                pstListFiles->s32FileSize = s32FileSize;
                snprintf_s(pstListFiles->aszDownURL, HI_OSC_LISTFILE_FILE_URL, HI_OSC_LISTFILE_FILE_URL - 1, "/home/osc/files/snap_%d.jpg", u32CommnadId);
                snprintf_s(pstListFiles->aszFileType, HI_OSC_LISTFILE_FILETYPE_LEN, HI_OSC_LISTFILE_FILETYPE_LEN - 1, "image");
                snprintf_s(pstListFiles->aszFileURL, HI_OSC_LISTFILE_FILE_URL, HI_OSC_LISTFILE_FILE_URL - 1, "http://192.168.1.1%s", aszName);
                snprintf_s(pstListFiles->aszDateTimeZone, HI_OSC_LISTFILE_DATE_TIME, HI_OSC_LISTFILE_DATE_TIME - 1, "2018:01:17 20:11:%02d+08:00", u32CommnadId);
                memset_s(pstListFiles->aszPreviewUrl, HI_OSC_LISTFILE_PREVIEW_URL, 0x00, HI_OSC_LISTFILE_PREVIEW_URL);
                snprintf_s(pstListFiles->aszThumbnail, HI_OSC_LISTFILE_THUMB_ENCODE, HI_OSC_LISTFILE_THUMB_ENCODE - 1, "an encording string");
                pstListFiles->dLat = 0.0f;
                pstListFiles->dLng = 0.0f;
                pstListFiles->bDownLoad = HI_FALSE;
                pstListFiles->bIsProcessed = HI_TRUE;
                pstListFiles->s32Height = 600;
                pstListFiles->s32Width = 400;
                u32CommnadId ++;
                break;

            case 1:
                snprintf_s(aszName, OSCSAMPLE_FILES_NAME_MAX_LEN, OSCSAMPLE_FILES_NAME_MAX_LEN - 1, "/home/osc/files/snap_%d.jpg", u32CommnadId);
                snprintf_s(aszCommand, OSCSAMPLE_FILES_COMMAND_MAX_LEN, OSCSAMPLE_FILES_COMMAND_MAX_LEN - 1, "cp /home/osc/files/snap_s.jpg %s", aszName);
                system(aszCommand);
                snprintf_s(aszFileUrl, HI_OSC_FILEURL_MAX_LEN, HI_OSC_FILEURL_MAX_LEN - 1, "http://192.168.1.1%s", aszName);
                snprintf_s(pstListFiles->aszFileName, HI_OSC_LISTFILE_FILE_NAME, HI_OSC_LISTFILE_FILE_NAME - 1, "snap_%d.jpg", u32CommnadId);
                OSCSVR_Get_FileSize(aszName, &s32FileSize);
                pstListFiles->s32FileSize = s32FileSize;
                snprintf_s(pstListFiles->aszDownURL, HI_OSC_LISTFILE_FILE_URL, HI_OSC_LISTFILE_FILE_URL - 1, "/home/osc/files/snap_%d.jpg", u32CommnadId);
                snprintf_s(pstListFiles->aszFileType, HI_OSC_LISTFILE_FILETYPE_LEN, HI_OSC_LISTFILE_FILETYPE_LEN - 1, "image");
                snprintf_s(pstListFiles->aszFileURL, HI_OSC_LISTFILE_FILE_URL, HI_OSC_LISTFILE_FILE_URL - 1, "http://192.168.1.1%s", aszName);
                snprintf_s(pstListFiles->aszDateTimeZone, HI_OSC_LISTFILE_DATE_TIME, HI_OSC_LISTFILE_DATE_TIME - 1, "2018:01:17 20:11:%02d+08:00", u32CommnadId);
                memset_s(pstListFiles->aszPreviewUrl, HI_OSC_LISTFILE_PREVIEW_URL, 0x00, HI_OSC_LISTFILE_PREVIEW_URL);
                snprintf_s(pstListFiles->aszThumbnail, HI_OSC_LISTFILE_THUMB_ENCODE, HI_OSC_LISTFILE_THUMB_ENCODE- 1, "an encording string");
                pstListFiles->dLat = 0.0f;
                pstListFiles->dLng = 0.0f;
                pstListFiles->bDownLoad = HI_FALSE;
                pstListFiles->bIsProcessed = HI_TRUE;
                pstListFiles->s32Height = 600;
                pstListFiles->s32Width = 400;
                u32CommnadId ++;
                break;
        }

    }
    else if (HI_OSC_RELATIVE_PATH == enPathType)
    {
        snprintf_s(aszFileUrl, HI_OSC_FILEURL_MAX_LEN, HI_OSC_FILEURL_MAX_LEN - 1, "Getimage.jpg");
        u32CommnadId ++;
    }
    else
    {
        printf("the input type is error!!");
    }

    strncpy_s(aszResults[0], HI_OSC_FILEURL_MAX_LEN, aszFileUrl, HI_OSC_FILEURL_MAX_LEN);

    for (s32Looper = 0; s32Looper < HI_OSC_FILEURL_MAX_NUM; s32Looper ++)
    {
        memset_s(g_aszTakePicture[s32Looper], HI_OSC_FILEURL_MAX_LEN, 0x00, HI_OSC_FILEURL_MAX_LEN);
    }

    strncpy_s(g_aszTakePicture[0], HI_OSC_FILEURL_MAX_LEN, aszFileUrl, HI_OSC_FILEURL_MAX_LEN);
    memset_s(g_aszCommandId, OSCSAMPLE_COMMAND_ID_MAX_LEN, 0x00, OSCSAMPLE_COMMAND_ID_MAX_LEN);
    strncpy_s(g_aszCommandId, OSCSAMPLE_COMMAND_ID_MAX_LEN, pszCommandId, strlen(pszCommandId) + 1);

    if (HI_SUCCESS != pthread_create(&lTakePictureThId, NULL, (HI_VOID*)OSCSVR_Thread_Takepicture, OSCHandle))
    {
        printf("the input type is error!!");
        free(pstListFiles);
        pstListFiles = NULL;
        return HI_FAILURE;
    }

    (HI_VOID)OSCSVR_Update_ListFiles(pstListFiles);
    free(pstListFiles);
    pstListFiles = NULL;

    return HI_SUCCESS;
}

HI_S32 OSCSVR_Command_GetLivePreview(HI_MW_PTR OSCHandle, HI_S32* ps32SockFd, HI_BOOL* pbDisabled)
{
    HI_CHAR* pcStream = NULL;
    HI_CHAR* pcStreamTmp = NULL;
    HI_S32 s32StreamLen = 0;
    HI_CHAR* pcFrame = 0;
    HI_S32 s32FrameLen = 0;
    HI_S32 s32FrameEnd = 0;
    HI_BOOL bFirstFrame = HI_TRUE;

    if (s_enOSCState != HIOSC_STATE_SERVICES_READY)
    {
        *pbDisabled = HI_TRUE;
        return HI_SUCCESS;
    }

    *pbDisabled = HI_FALSE;

    if (NULL == ps32SockFd)
    {
        printf("getLivePreview input NULL!!\r\n");
        return HI_FAILURE;
    }

    while (s_enOSCState == HIOSC_STATE_SERVICES_READY)
    {
        if ((s_bLooper == HI_FALSE))
        {
            printf("2. need to exit the live preview\n");
            return HI_SUCCESS;
        }

        pcStream = (HI_CHAR*)malloc(OSCSAMPLE_STREAM_BUFF_MAX_LEN);

        if (NULL == pcStream)
        {
            printf("the stream temp malloc fail!\n");
            return HI_FAILURE;
        }

        memset_s(pcStream, OSCSAMPLE_STREAM_BUFF_MAX_LEN, 0x00, OSCSAMPLE_STREAM_BUFF_MAX_LEN);
        pcStreamTmp = pcStream;

        if (HI_SUCCESS != OSCSVR_GetStreamData(pcStream, OSCSAMPLE_STREAM_BUFF_MAX_LEN, &s32StreamLen))
        {
            printf("OSCSVR_GetStreamData fail!\n");
            free(pcStreamTmp);
            pcStreamTmp = NULL;
            return HI_FAILURE;
        }

        pcFrame = (HI_CHAR*)malloc(SAMPLE_OSC_STREAM_FRAME_LEN);

        if (NULL == pcFrame)
        {
            printf("the pcFrame malloc fail!\n");
            free(pcStreamTmp);
            pcStreamTmp = NULL;
            return HI_FAILURE;
        }

        memset_s(pcFrame, SAMPLE_OSC_STREAM_FRAME_LEN, 0x00, SAMPLE_OSC_STREAM_FRAME_LEN);
        s32FrameLen = 0;

        while (1)
        {
            memset_s(pcFrame, SAMPLE_OSC_STREAM_FRAME_LEN, 0x00, SAMPLE_OSC_STREAM_FRAME_LEN);
            s32FrameLen = 0;

            if (HI_SUCCESS != OSCSVR_GetFrameData(pcStream, &s32StreamLen, &s32FrameEnd, pcFrame, &s32FrameLen))
            {
                break;
            }

            OSCSVR_WriteImage(pcFrame, s32FrameLen);

            if ((s_enOSCState == HIOSC_STATE_SERVICES_READY) && (s_bLooper != HI_FALSE))
            {
                if (HI_SUCCESS != HI_OSCSVR_Send_FrameData(OSCHandle, bFirstFrame, ps32SockFd, pcFrame, s32FrameLen))
                {
                    free(pcStreamTmp);
                    pcStreamTmp = NULL;
                    free(pcFrame);
                    pcFrame = NULL;
                    return HI_SUCCESS;
                }
            }
            else
            {
                printf("the OSC live preview need to exit!\r\n");
                free(pcStreamTmp);
                pcStreamTmp = NULL;
                free(pcFrame);
                pcFrame = NULL;
                return HI_SUCCESS;
            }

            if (s32StreamLen <= 0)
            {
                printf("OSCSVR_GetFrameData read frame end!!\r\n");
                break;
            }

            pcStream =  pcStream + s32FrameEnd + 1;
            s32StreamLen = s32StreamLen - s32FrameEnd - 1;
            bFirstFrame = HI_FALSE;
        }

        free(pcStreamTmp);
        pcStreamTmp = NULL;
        free(pcFrame);
        pcFrame = NULL;
    }

    return HI_SUCCESS;
}

HI_S32 OSCSVR_Command_ProcessPictures(HI_MW_PTR OSCHandle, HI_OSC_PATH_TYPE_E enPathType, HI_CHAR aszFileUrl[HI_OSC_FILEURL_MAX_NUM][HI_OSC_FILEURL_MAX_LEN], HI_CHAR* pszCommandId, HI_BOOL* pbComDisabled, HI_BOOL* pbUrlInvaild)
{
    HI_S32 s32Looper = 0;
    HI_CHAR aszResults[HI_OSC_FILEURL_MAX_NUM][HI_OSC_FILEURL_MAX_LEN];

    for (s32Looper = 0; s32Looper < HI_OSC_FILEURL_MAX_NUM; s32Looper ++)
    {
        memset_s(aszResults[s32Looper], HI_OSC_FILEURL_MAX_LEN, 0x00, HI_OSC_FILEURL_MAX_LEN);
    }

    *pbUrlInvaild = HI_FALSE;

    if (HI_TRUE == *pbUrlInvaild)
    {
        snprintf_s(aszResults[0], HI_OSC_FILEURL_MAX_LEN, HI_OSC_FILEURL_MAX_LEN - 1, "some urls is invaild!");

        if (HI_SUCCESS != HI_OSCSVR_Update_CommandStatus(OSCHandle, pszCommandId, HIOSC_COMM_STATUS_ERROR, aszResults))
        {
            printf("Add the processpicture result into server is error!!\r\n");
            return HI_FAILURE;
        }

        return HI_SUCCESS;
    }

    *pbComDisabled = HI_FALSE;

    for (s32Looper = 0; s32Looper < HI_OSC_FILEURL_MAX_NUM; s32Looper ++)
    {
        if (strlen(aszFileUrl[s32Looper]) == 0)
        {
            break;
        }

        snprintf_s(aszResults[s32Looper], HI_OSC_FILEURL_MAX_LEN, HI_OSC_FILEURL_MAX_LEN - 1, "process_%s", aszFileUrl[s32Looper]);
        printf("the %dth process is %s %d\n", s32Looper, aszResults[s32Looper], enPathType);
    }

    sleep(1);

    if (HI_SUCCESS != HI_OSCSVR_Update_CommandStatus(OSCHandle, pszCommandId, HIOSC_COMM_STATUS_DONE, aszResults))
    {
        printf("Add the takepiture result into server is error!!\r\n");
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 OSCSVR_Command_StartCapture(HI_MW_PTR OSCHandle, HI_OSC_PATH_TYPE_E enPathType, HI_CHAR* pszCommandId, HI_BOOL* pbComDisabled, HI_OSC_CAPTURE_MODE_E* penCaptureMode)
{
    HI_S32 s32Looper = 0;
    HI_CHAR aszCaptureUrl[HI_OSC_FILEURL_MAX_NUM][HI_OSC_FILEURL_MAX_LEN];

    if (NULL == penCaptureMode)
    {
        printf("startcapture input is NULL!!\r\n");
        return HI_FAILURE;
    }

    for (s32Looper = 0; s32Looper < HI_OSC_FILEURL_MAX_NUM; s32Looper ++)
    {
        memset_s(aszCaptureUrl[s32Looper], HI_OSC_FILEURL_MAX_LEN, 0x00, HI_OSC_FILEURL_MAX_LEN);
    }

    if (HI_FALSE == s_bVideo)
    {
        *pbComDisabled = HI_TRUE;
        snprintf_s(aszCaptureUrl[0], HI_OSC_FILEURL_MAX_LEN, HI_OSC_FILEURL_MAX_LEN - 1, "some urls is invaild!");

        if (HI_SUCCESS != HI_OSCSVR_Update_CommandStatus(OSCHandle, pszCommandId, HIOSC_COMM_STATUS_ERROR, aszCaptureUrl))
        {
            printf("Add the startcapture result into server is error!!\r\n");
            return HI_FAILURE;
        }

        return HI_SUCCESS;
    }

    if ((0 == strlen(s_aszCaptureMode)) || (0 == strncmp(s_aszCaptureMode, "interval", strlen("interval"))))
    {
        s_bVideo = HI_TRUE;
        *penCaptureMode = HI_OSC_CAPTURE_MODE_INTERVAL;

        for (s32Looper = 0; s32Looper < 5; s32Looper ++)
        {
            snprintf_s(aszCaptureUrl[s32Looper], HI_OSC_FILEURL_MAX_LEN, HI_OSC_FILEURL_MAX_LEN - 1, "interval_%d.jpg", s32Looper);
        }

        printf("start capture command id is %s %d\n", pszCommandId, enPathType);
        s_bVideo = HI_FALSE;

        if (HI_SUCCESS != HI_OSCSVR_Update_CommandStatus(OSCHandle, pszCommandId, HIOSC_COMM_STATUS_DONE, aszCaptureUrl))
        {
            printf("Add the takepiture result into server is error!!\r\n");
            return HI_FAILURE;
        }

    }
    else if (0 == strncmp(s_aszCaptureMode, "interval", strlen("interval")))
    {
        s_bVideo = HI_TRUE;
        *penCaptureMode = HI_OSC_CAPTURE_MODE_VIDEO;
        printf("the app is capturing the video now!!\r\n");
    }
    else
    {
        snprintf_s(aszCaptureUrl[0], HI_OSC_FILEURL_MAX_LEN, HI_OSC_FILEURL_MAX_LEN - 1, "the mode is unsupported!!");

        if (HI_SUCCESS != HI_OSCSVR_Update_CommandStatus(OSCHandle, pszCommandId, HIOSC_COMM_STATUS_ERROR, aszCaptureUrl))
        {
            printf("Add the startcapture result into server is error!!\r\n");
            return HI_FAILURE;
        }
    }

    return HI_SUCCESS;
}

HI_S32 OSCSVR_Command_StopCapture(HI_OSC_PATH_TYPE_E enPathType, HI_CHAR aszCaptureUrl[HI_OSC_FILEURL_MAX_NUM][HI_OSC_FILEURL_MAX_LEN], HI_BOOL* pbDisabled)
{
    static HI_S32 s32Flag = 0;

    if (s32Flag != 0)
    {
        s32Flag = 0;
        *pbDisabled = HI_TRUE;
        return HI_SUCCESS;
    }

    if (enPathType == HI_OSC_ABSOLUTE_PATH)
    {
        snprintf_s(aszCaptureUrl[0], HI_OSC_FILEURL_MAX_LEN, HI_OSC_FILEURL_MAX_LEN - 1, "http://192.168.1.1/home/osc/files/snap_a.jpg");
        s32Flag ++;
    }

    return HI_SUCCESS;
}

HI_S32 OSCSVR_Command_ListFile(HI_OSC_PATH_TYPE_E enPathType, HI_OSCSERVER_LIST_FILE_S* pstProperty)
{
    HI_S32 s32Looper = 0;
    HI_S32 s32Num = 0;

    pstProperty->bInvaild = HI_FALSE;
    OSCSAM_LOCK(g_OSCFILELock);

    for (s32Looper = 0; s32Looper < g_s32ListFile; s32Looper ++)
    {
        if ((0 != strlen(g_stListFiles[g_s32ListFile - s32Looper].aszFileName)) && (g_stListFiles[g_s32ListFile - s32Looper].bDownLoad != HI_TRUE))
        {
            strncpy_s(pstProperty->aszEntries[s32Looper].aszFileName, HI_OSC_LISTFILE_FILE_NAME, g_stListFiles[g_s32ListFile - s32Looper].aszFileName, HI_OSC_LISTFILE_FILE_NAME);
            strncpy_s(pstProperty->aszEntries[s32Looper].aszFileURL, HI_OSC_LISTFILE_FILE_URL, g_stListFiles[g_s32ListFile - s32Looper].aszFileURL, HI_OSC_LISTFILE_FILE_URL);
            pstProperty->aszEntries[s32Looper].s32FileSize = g_stListFiles[g_s32ListFile - s32Looper].s32FileSize;   /*byte*/
            strncpy_s(pstProperty->aszEntries[s32Looper].aszDateTimeZone, HI_OSC_LISTFILE_DATE_TIME, g_stListFiles[g_s32ListFile - s32Looper].aszDateTimeZone, HI_OSC_LISTFILE_DATE_TIME);
            pstProperty->aszEntries[s32Looper].dLat = g_stListFiles[g_s32ListFile - s32Looper].dLat;
            pstProperty->aszEntries[s32Looper].dLng = g_stListFiles[g_s32ListFile - s32Looper].dLng;
            pstProperty->aszEntries[s32Looper].s32Height = g_stListFiles[g_s32ListFile - s32Looper].s32Height;
            pstProperty->aszEntries[s32Looper].s32Width = g_stListFiles[g_s32ListFile - s32Looper].s32Width;
            strncpy_s(pstProperty->aszEntries[s32Looper].aszThumbnail, HI_OSC_LISTFILE_THUMB_ENCODE, g_stListFiles[g_s32ListFile - s32Looper].aszThumbnail, HI_OSC_LISTFILE_THUMB_ENCODE);
            pstProperty->aszEntries[s32Looper].bIsProcessed = g_stListFiles[g_s32ListFile - s32Looper].bIsProcessed;
            strncpy_s(pstProperty->aszEntries[s32Looper].aszPreviewUrl, HI_OSC_LISTFILE_PREVIEW_URL, g_stListFiles[g_s32ListFile - s32Looper].aszPreviewUrl, HI_OSC_LISTFILE_PREVIEW_URL);
            s32Num ++;
        }
    }

    pstProperty->s32TotalEntries = s32Num;
    OSCSAM_UNLOCK(g_OSCFILELock);

    if (s32Num == 0)
    {
        snprintf_s(pstProperty->aszEntries[0].aszFileName, HI_OSC_LISTFILE_FILE_NAME, HI_OSC_LISTFILE_FILE_NAME - 1, "1.mp4");
        snprintf_s(pstProperty->aszEntries[0].aszFileURL, HI_OSC_LISTFILE_FILE_URL, HI_OSC_LISTFILE_FILE_URL - 1, "http://192.168.1.1/home/osc/%d_1.mp4", enPathType);
        pstProperty->aszEntries[0].s32FileSize = 20000;   /*byte*/
        snprintf_s(pstProperty->aszEntries[0].aszDateTimeZone, HI_OSC_LISTFILE_DATE_TIME, HI_OSC_LISTFILE_DATE_TIME - 1, "2018:01:11 20:11:%02d+08:00", s32Looper);
        pstProperty->aszEntries[0].dLat = 0.0f;
        pstProperty->aszEntries[0].dLng = 0.0f;
        pstProperty->aszEntries[0].s32Height = 400;
        pstProperty->aszEntries[0].s32Width = 600;
        snprintf_s(pstProperty->aszEntries[0].aszThumbnail, HI_OSC_LISTFILE_THUMB_ENCODE, HI_OSC_LISTFILE_THUMB_ENCODE - 1, "ENCODEDSTRING");
        pstProperty->aszEntries[0].bIsProcessed = HI_TRUE;
        memset_s(pstProperty->aszEntries[0].aszPreviewUrl, HI_OSC_LISTFILE_PREVIEW_URL, 0x00, HI_OSC_LISTFILE_PREVIEW_URL);

        s32Num ++;
        pstProperty->s32TotalEntries = s32Num;
    }

    return HI_SUCCESS;
}

HI_S32 OSCSVR_Command_ListImage(HI_OSC_PATH_TYPE_E enPathType, HI_OSCSERVER_LIST_IMAGE_S* pstListImage)
{
    HI_S32 s32Looper = 0;
    HI_S32 s32Num = 0;

    s32Num = 3;
    pstListImage->bInvaild = HI_FALSE;
    printf("recv from the client is entrycount: %d maxthumbSize: %d token: %s", pstListImage->s32EntryCounts, pstListImage->s32MaxThumbSize, pstListImage->aszInContinuationToken);

    for (s32Looper = 0; s32Looper < s32Num; s32Looper ++)
    {
        snprintf_s(pstListImage->aszEntries[s32Looper].aszFileName, HI_OSC_LISTFILE_FILE_NAME, HI_OSC_LISTFILE_FILE_NAME - 1, "listimage_%d", s32Looper);
        snprintf_s(pstListImage->aszEntries[s32Looper].aszFileURL, HI_OSC_LISTFILE_FILE_URL, HI_OSC_LISTFILE_FILE_URL - 1, "/home/listimages_%d_%d", s32Looper, enPathType);
        pstListImage->aszEntries[s32Looper].s32FileSize = s32Looper + 300 ;
        snprintf_s(pstListImage->aszEntries[s32Looper].aszDateTimeZone, HI_OSC_LISTFILE_DATE_TIME, HI_OSC_LISTFILE_DATE_TIME - 1, "2017-11-11(%s)", __DATE__);
        pstListImage->aszEntries[s32Looper].dLat = 45.0;
        pstListImage->aszEntries[s32Looper].dLng = 135.0;
        pstListImage->aszEntries[s32Looper].s32Height = 1680 + s32Looper;
        pstListImage->aszEntries[s32Looper].s32Width = 1920 + s32Looper;
        snprintf_s(pstListImage->aszEntries[s32Looper].aszThumbnail, HI_OSC_LISTFILE_THUMB_ENCODE, HI_OSC_LISTFILE_THUMB_ENCODE - 1, "listimage Based 64bit thunmb(%d)", s32Looper);
    }

    pstListImage->s32TotalEntries = s32Num;
    snprintf_s(pstListImage->aszOutContinuationToken, HI_OSC_CONTINUDATION_TOKEN_MAX_LEN, HI_OSC_CONTINUDATION_TOKEN_MAX_LEN - 1, "%d", s32Looper);

    return HI_SUCCESS;
}

HI_S32 OSCSVR_Command_GetMetaData(HI_OSC_PATH_TYPE_E enPathType, HI_OSCSERVER_METADATA_S* pstMetaData)
{
    pstMetaData->bUrlInvaild = HI_FALSE;
    /*exif*/
    snprintf_s(pstMetaData->stExif.aszExifVersion, HI_OSC_METADATA_EXIF_VERSION, HI_OSC_METADATA_EXIF_VERSION - 1, "hiberry_V1.0.0");
    snprintf_s(pstMetaData->stExif.aszImageDescription, HI_OSC_METADATA_EXIF_IMAGE_DESCRIPTION, HI_OSC_METADATA_EXIF_IMAGE_DESCRIPTION - 1, "%d_image", enPathType);
    pstMetaData->stExif.s32ImageWidth = 1920;
    pstMetaData->stExif.s32ImageLength = 1680;
    pstMetaData->stExif.s32ColorSpace = 1;
    pstMetaData->stExif.s32Compression = 1;
    pstMetaData->stExif.s32Flash = 2;
    pstMetaData->stExif.s32Orientation = 3;
    pstMetaData->stExif.s32FocalLength = 4;
    pstMetaData->stExif.s32WhiteBalance = 4;
    pstMetaData->stExif.s32ExposureTime = 15;
    pstMetaData->stExif.s32FNumber = 20;
    pstMetaData->stExif.s32ExposureProgram = 1;
    pstMetaData->stExif.s32ISOSpeedRatings = 12;
    pstMetaData->stExif.s32ShutterSpeedValue = 13;
    pstMetaData->stExif.s32ApertureValue = 1;
    pstMetaData->stExif.s32BrightnessValue = 2;
    pstMetaData->stExif.s32ExposureBiasValue = 3;
    snprintf_s(pstMetaData->stExif.aszGPSProcessingMethod, HI_OSC_METADATA_EXIF_GPS_PROCESS, HI_OSC_METADATA_EXIF_GPS_PROCESS - 1, "hiberry_GPSProcess");
    snprintf_s(pstMetaData->stExif.aszGPSLatitudeRef, HI_OSC_METADATA_EXIF_GPSLATITUDE_REF, HI_OSC_METADATA_EXIF_GPSLATITUDE_REF - 1, "Latitude_GPSRef");
    pstMetaData->stExif.dGPSLatitude = 95.0;
    snprintf_s(pstMetaData->stExif.aszGPSLatitudeRef, HI_OSC_METADATA_EXIF_GPSLATITUDE_REF, HI_OSC_METADATA_EXIF_GPSLATITUDE_REF - 1, "Longitude_GPSRef");
    pstMetaData->stExif.dGPSLongitude = 45.0;
    snprintf_s(pstMetaData->stExif.aszMake, HI_OSC_METADATA_EXIF_MAKE, HI_OSC_METADATA_EXIF_MAKE - 1, "hiberry_make");
    snprintf_s(pstMetaData->stExif.aszModel, HI_OSC_METADATA_EXIF_MODEL, HI_OSC_METADATA_EXIF_MODEL - 1, "hiberry_model");
    snprintf_s(pstMetaData->stExif.aszSoftware, HI_OSC_METADATA_EXIF_SOFTWARE, HI_OSC_METADATA_EXIF_SOFTWARE - 1, "hiberry_software");
    snprintf_s(pstMetaData->stExif.aszCopyright, HI_OSC_METADATA_EXIF_COPYRIGHT, HI_OSC_METADATA_EXIF_COPYRIGHT - 1, "hiberry_huawei");
    snprintf_s(pstMetaData->stExif.aszMakerNote, HI_OSC_METADATA_EXIF_MAKERNOTE, HI_OSC_METADATA_EXIF_MAKERNOTE - 1, "makenote");
    snprintf_s(pstMetaData->stExif.aszImageUniqueID, HI_OSC_METADATA_EXIF_IMAGE_ID, HI_OSC_METADATA_EXIF_IMAGE_ID - 1, "1234567");
    /*xmp*/
    snprintf_s(pstMetaData->stXMP.aszProjectionType, HI_OSC_METADATA_XMP_PROJECT_TYPE, HI_OSC_METADATA_XMP_PROJECT_TYPE - 1, "xmp_projection");
    pstMetaData->stXMP.bUsePanoramaViewer = HI_TRUE;
    pstMetaData->stXMP.dPoseHeadingDegrees = 1.8;
    pstMetaData->stXMP.s32CroppedAreaImageWidthPixels = 1;
    pstMetaData->stXMP.s32CroppedAreaImageHeightPixels = 2;
    pstMetaData->stXMP.s32FullPanoWidthPixels = 3;
    pstMetaData->stXMP.s32FullPanoHeightPixels = 4;
    pstMetaData->stXMP.s32CroppedAreaLeftPixels = 5;
    pstMetaData->stXMP.s32CroppedAreaTopPixels = 6;

    return HI_SUCCESS;
}

HI_S32 OSCSVR_Command_Delete(HI_OSC_PATH_TYPE_E enPathType, HI_CHAR aszFileUrl[HI_OSC_FILEURL_MAX_NUM][HI_OSC_FILEURL_MAX_LEN], HI_BOOL* bInvaild, HI_CHAR aszResultUrl[HI_OSC_FILEURL_MAX_NUM][HI_OSC_FILEURL_MAX_LEN])
{
    HI_S32 s32Looper = 0;
    HI_CHAR* pcUrl = NULL;
    HI_CHAR aszRealFileUrl[HI_OSC_FILEURL_MAX_LEN] = {0};
    HI_CHAR aszCommand[HI_OSC_FILEURL_MAX_LEN] = {0};
    FILE* DelFile = NULL;

    for (s32Looper = 0; s32Looper < HI_OSC_FILEURL_MAX_NUM; s32Looper ++)
    {
        if (strlen(aszFileUrl[s32Looper]) != 0)
        {
            pcUrl = strstr(aszFileUrl[s32Looper], "http://192.168.1.1");

            if (pcUrl == NULL)
            {
                printf("delete url format is err:%d!!\r\n", enPathType);
                return HI_FAILURE;
            }

            pcUrl += strlen("http://192.168.1.1");

            if (NULL == realpath(pcUrl, aszRealFileUrl))
            {
                printf("the File url:%s is err!!\n", pcUrl);
                return HI_FAILURE;
            }

            DelFile = fopen(aszRealFileUrl, "r");

            if (NULL == DelFile)
            {
                printf("fopen file err!\r\n");
                *bInvaild = HI_TRUE;
                return HI_SUCCESS;
            }

            fclose(DelFile);
            DelFile = NULL;

            snprintf_s(aszCommand, HI_OSC_FILEURL_MAX_LEN, HI_OSC_FILEURL_MAX_LEN - 1, "rm %s", pcUrl);
            system(aszCommand);
            (HI_VOID)OSCSVR_Delete_ListFiles(pcUrl);
        }
    }

    *bInvaild = HI_FALSE;
    return HI_SUCCESS;
}

HI_S32 OSCSVR_Command_SetOptions(HI_OSC_SERVER_LEVEL_E enLevel, HI_OSCSERVER_OPTIONS_S* pstSetOptions, HI_OSC_OPTIONS_TABLE_S* pstSetOptionsTable)
{
    if ((pstSetOptions == NULL) || (pstSetOptionsTable == NULL))
    {
        printf("the setoption input is error!!!\n");
        return HI_FAILURE;
    }

    if ((HI_OSC_SERVER_LEVEL_1 != enLevel) && (HI_OSC_SERVER_LEVEL_2 != enLevel))
    {
        printf("the setoption input level is error!!!\n");
        return HI_FAILURE;
    }

    if (HI_TRUE == pstSetOptionsTable->bCaptureMode)
    {
        printf("capture mode is %s\n", pstSetOptions->aszCaptureMode);
        memset_s(s_aszCaptureMode, HI_OSC_OPTION_CAPTURE_MODE_LEN, 0x00, HI_OSC_OPTION_CAPTURE_MODE_LEN);
        strncpy_s(s_aszCaptureMode, HI_OSC_OPTION_CAPTURE_MODE_LEN, pstSetOptions->aszCaptureMode, HI_OSC_OPTION_CAPTURE_MODE_LEN);
        s_aszCaptureMode[strlen(pstSetOptions->aszCaptureMode)] = '\0';
    }

    return HI_SUCCESS;
}

HI_S32 OSCSVR_Command_GetOptions(HI_OSC_SERVER_LEVEL_E enLevel, HI_OSCSERVER_OPTIONS_S* pstGetOptions, HI_OSC_OPTIONS_TABLE_S* pstGetOptionsTable)
{
    HI_BOOL bFlag = HI_FALSE;

    if ((pstGetOptions == NULL) || (pstGetOptionsTable == NULL))
    {
        printf("the getoption input is error!!!\n");
        return HI_FAILURE;
    }

    if ((HI_OSC_SERVER_LEVEL_1 != enLevel) && (HI_OSC_SERVER_LEVEL_2 != enLevel))
    {
        printf("the getoption input level is error!!!\n");
        return HI_FAILURE;
    }

    if (HI_TRUE == pstGetOptionsTable->bCaptureMode)
    {
        bFlag = HI_TRUE;
    }

    if (HI_OSC_OPTIONS_MODE_IMAGE == pstGetOptions->enOptionMode)
    {
        if (HI_TRUE == pstGetOptionsTable->stImageTable.bISO)
        {
            pstGetOptions->unModeStuct.stImage.s32ISO = 1600;
        }

        bFlag = HI_TRUE;
    }
    else if (HI_OSC_OPTIONS_MODE_INTERVAL == pstGetOptions->enOptionMode)
    {
        bFlag = HI_TRUE;
    }

    if (HI_TRUE == pstGetOptions->bForMat)
    {
        if (pstGetOptionsTable->stFormatTable.bFileFormat)
        {
            snprintf_s(pstGetOptions->stForMat.stFileFormat.aszType, HI_OSC_OPTION_FORMAT_TYPE_NAME, HI_OSC_OPTION_FORMAT_TYPE_NAME - 1, "jpeg");
            pstGetOptions->stForMat.stFileFormat.s32Width = 1366;
            pstGetOptions->stForMat.stFileFormat.s32Height = 720;
        }

        bFlag = HI_TRUE;
    }

    if (HI_TRUE == pstGetOptions->bInfo)
    {
        if (pstGetOptionsTable->stInfoTable.bTotalSpace)
        {
            pstGetOptions->stInfo.s32TotalSpace = 1024;
        }

        bFlag = HI_TRUE;
    }

    /*compensation*/
    pstGetOptions->unModeStuct.stImage.dExposureCompensation = 0.3;
    pstGetOptions->unModeStuct.stImage.aszExposureCompensationSupport[0] = 0.1;
    pstGetOptions->unModeStuct.stImage.aszExposureCompensationSupport[1] = 0.2;
    pstGetOptions->unModeStuct.stImage.aszExposureCompensationSupport[2] = 0.3;
    pstGetOptions->unModeStuct.stImage.aszExposureCompensationSupport[3] = 0.4;
    pstGetOptions->unModeStuct.stImage.aszExposureCompensationSupport[4] = 0.5;
    pstGetOptions->unModeStuct.stImage.aszExposureCompensationSupport[5] = 0.6;
    /*HDR*/
    snprintf_s(pstGetOptions->unModeStuct.stImage.aszLevel2Hdr, HI_OSC_OPTION_HDR, HI_OSC_OPTION_HDR - 1, "off");
    snprintf_s(pstGetOptions->unModeStuct.stImage.aszLevel2HdrSupport[0], HI_OSC_OPTION_HDR, HI_OSC_OPTION_HDR - 1, "off");
    snprintf_s(pstGetOptions->unModeStuct.stImage.aszLevel2HdrSupport[1], HI_OSC_OPTION_HDR, HI_OSC_OPTION_HDR - 1, "hdr");
    /*s32CaptureInterval*/
    pstGetOptions->unModeStuct.stImage.s32CaptureInterval = 10;
    pstGetOptions->unModeStuct.stImage.stCaptureIntervalSupport.u32MaxInterval = 3600;
    pstGetOptions->unModeStuct.stImage.stCaptureIntervalSupport.u32MinInterval = 8;
    /*sleepdelay*/
    pstGetOptions->stInfo.s32SleepDelay = 300;
    pstGetOptions->stInfo.aszSleepDelaySupport[0] = 180;
    pstGetOptions->stInfo.aszSleepDelaySupport[1] = 300;
    pstGetOptions->stInfo.aszSleepDelaySupport[2] = 420;
    pstGetOptions->stInfo.aszSleepDelaySupport[3] = 65535;

    if (HI_TRUE !=  bFlag)
    {
        printf("err! no argue need to be get!!\n");
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 OSCSVR_Command_Reset()
{
    sleep(2);

    return HI_SUCCESS;
}

HI_S32 OSCSVR_Command_SwitchWifi(HI_OSCSERVER_SWITCH_WIFI_S* pstSwitchWifi)
{
    if (NULL == pstSwitchWifi)
    {
        printf("th switch input is null!!\n");
        return HI_FAILURE;
    }

    pstSwitchWifi->bInvaild = HI_FALSE;
    printf("the switchwifi data is sharekey: %s , ssid: %s  pwd: %s\n", pstSwitchWifi->aszpreSharedKey, pstSwitchWifi->aszWifiSSID, pstSwitchWifi->aszWifiPWD);

    return HI_SUCCESS;
}

HI_S32 OSCSVR_Command_UploadFile(HI_OSCSERVER_UPLOAD_FILE_S* pstUploadFile)
{
    if (NULL == pstUploadFile)
    {
        printf("the switch input is null!!\n");
        return HI_FAILURE;
    }

    pstUploadFile->bInvaild = HI_FALSE;

    printf("the switchwifi data is fileurl: %s , upload: %s  access: %s\n", pstUploadFile->aszFileUrl, pstUploadFile->aszUploadUrl, pstUploadFile->aszAccessToken);

    return HI_SUCCESS;
}

HI_S32 OSCSVR_Register_API_CallBack(HI_MW_PTR* pHandle)
{
    HI_S32 s32Ret = HI_SUCCESS;

    /*register /osc/info cb*/
    s32Ret = HI_OSCSVR_RegisterEventCallback(*pHandle, HI_OSC_COMMAND_INFO, (HI_VOID*)OSCSVR_API_Info, "/osc/info");

    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_OSCSVR_RegisterEventCallback info fail s32Ret:%d\n", s32Ret);
        return HI_FAILURE;
    }

    /*register /osc/state cb*/
    s32Ret = HI_OSCSVR_RegisterEventCallback(*pHandle, HI_OSC_COMMAND_STATE, (HI_VOID*)OSCSVR_API_State, "/osc/state");

    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_OSCSVR_RegisterEventCallback state!!! s32Ret:%d\n", s32Ret);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 OSCSVR_Register_COMM_CallBack(HI_MW_PTR* pHandle)
{
    HI_S32 s32Ret = HI_SUCCESS;

    /*register takePicture cb*/
    s32Ret = HI_OSCSVR_RegisterEventCallback(*pHandle, HI_OSC_COMMAND_TAKE_PICTURE, (HI_VOID*)OSCSVR_Command_Takepicture, "camera.takePicture");

    if (HI_SUCCESS != s32Ret)
    {
        printf("register camera.takePicture fail!!! s32Ret:%d\n", s32Ret);
        return HI_FAILURE;
    }

    /*register getLivePreview cb*/
    s32Ret = HI_OSCSVR_RegisterEventCallback(*pHandle, HI_OSC_COMMAND_GET_PRELIVE_VIEW, (HI_VOID*)OSCSVR_Command_GetLivePreview, "camera.getLivePreview");

    if (HI_SUCCESS != s32Ret)
    {
        printf("register camera.getLivePreview fail!!! s32Ret:%d\n", s32Ret);
        return HI_FAILURE;
    }

    /*register processPicture cb*/
    s32Ret = HI_OSCSVR_RegisterEventCallback(*pHandle, HI_OSC_COMMAND_PROCESS_PICTURE, (HI_VOID*)OSCSVR_Command_ProcessPictures, "camera.processPicture");

    if (HI_SUCCESS != s32Ret)
    {
        printf("register camera.processpicture fail!!! s32Ret:%d\n", s32Ret);
        return HI_FAILURE;
    }

    /*register startcapture cb*/
    s32Ret = HI_OSCSVR_RegisterEventCallback(*pHandle, HI_OSC_COMMAND_START_CAPTURE, (HI_VOID*)OSCSVR_Command_StartCapture, "camera.startCapture");

    if (HI_SUCCESS != s32Ret)
    {
        printf("register camera.startcapture fail!!! s32Ret:%d\n", s32Ret);
        return HI_FAILURE;
    }

    /*register listfile cb*/
    s32Ret = HI_OSCSVR_RegisterEventCallback(*pHandle, HI_OSC_COMMAND_LIST_FILE, (HI_VOID*)OSCSVR_Command_ListFile, "camera.listFile");

    if (HI_SUCCESS != s32Ret)
    {
        printf("register camera.listFile fail!!! s32Ret:%d\n", s32Ret);
        return HI_FAILURE;
    }

    /*register listimage cb*/
    s32Ret = HI_OSCSVR_RegisterEventCallback(*pHandle, HI_OSC_COMMAND_LIST_IMAGE, (HI_VOID*)OSCSVR_Command_ListImage, "camera.listImages");

    if (HI_SUCCESS != s32Ret)
    {
        printf("register camera.listimage fail!!! s32Ret:%d\n", s32Ret);
        return HI_FAILURE;
    }

    /*register getmetadata cb*/
    s32Ret = HI_OSCSVR_RegisterEventCallback(*pHandle, HI_OSC_COMMAND_GET_METADATA, (HI_VOID*)OSCSVR_Command_GetMetaData, "camera.getMetadata");

    if (HI_SUCCESS != s32Ret)
    {
        printf("register camera.getMetadata fail!!! s32Ret:%d\n", s32Ret);
        return HI_FAILURE;
    }

    /*register Delete cb*/
    s32Ret = HI_OSCSVR_RegisterEventCallback(*pHandle, HI_OSC_COMMAND_DELETE, (HI_VOID*)OSCSVR_Command_Delete, "camera.delete");

    if (HI_SUCCESS != s32Ret)
    {
        printf("register camera.delete fail!!! s32Ret:%d\n", s32Ret);
        return HI_FAILURE;
    }

    /*register setoptions cb*/
    s32Ret = HI_OSCSVR_RegisterEventCallback(*pHandle, HI_OSC_COMMAND_SET_OPTIONS, (HI_VOID*)OSCSVR_Command_SetOptions, "camera.setOptions");

    if (HI_SUCCESS != s32Ret)
    {
        printf("register camera.setOptions fail!!! s32Ret:%d\n", s32Ret);
        return HI_FAILURE;
    }

    /*register getoptions cb*/
    s32Ret = HI_OSCSVR_RegisterEventCallback(*pHandle, HI_OSC_COMMAND_GET_OPTIONS, (HI_VOID*)OSCSVR_Command_GetOptions, "camera.getOptions");

    if (HI_SUCCESS != s32Ret)
    {
        printf("register camera.getOptions fail!!! s32Ret:%d\n", s32Ret);
        return HI_FAILURE;
    }

    /*register reset cb*/
    s32Ret = HI_OSCSVR_RegisterEventCallback(*pHandle, HI_OSC_COMMAND_RESET, (HI_VOID*)OSCSVR_Command_Reset, "camera.reset");

    if (HI_SUCCESS != s32Ret)
    {
        printf("register camera.reset fail!!! s32Ret:%d\n", s32Ret);
        return HI_FAILURE;
    }

    /*register stopcapture cb*/
    s32Ret = HI_OSCSVR_RegisterEventCallback(*pHandle, HI_OSC_COMMAND_STOP_CAPTURE, (HI_VOID*)OSCSVR_Command_StopCapture, "camera.stopCapture");

    if (HI_SUCCESS != s32Ret)
    {
        printf("register camera.stopCapture fail!!! s32Ret:%d\n", s32Ret);
        return HI_FAILURE;
    }

    /*register switchwifi cb*/
    s32Ret = HI_OSCSVR_RegisterEventCallback(*pHandle, HI_OSC_COMMAND_SWITCH_WIFI, (HI_VOID*)OSCSVR_Command_SwitchWifi, "camera.switchWifi");

    if (HI_SUCCESS != s32Ret)
    {
        printf("register camera.switchWifi fail!!! s32Ret:%d\n", s32Ret);
        return HI_FAILURE;
    }

    /*register uploadfile cb*/
    s32Ret = HI_OSCSVR_RegisterEventCallback(*pHandle, HI_OSC_COMMAND_UPLOAD_FILE, (HI_VOID*)OSCSVR_Command_UploadFile, "camera.uploadFile");

    if (HI_SUCCESS != s32Ret)
    {
        printf("register camera.uploadFile fail!!! s32Ret:%d\n", s32Ret);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 SAMPLE_OSCSVR_Start()
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_MW_PTR pOSCHandle = NULL;
    HI_MW_PTR pOSCNetHandle = NULL;
    HI_S32 s32HttpPort = 80;

    memset_s(&g_stOSCConf, sizeof(HI_OSC_CONFIG_S), 0x00, sizeof(HI_OSC_CONFIG_S));
    /*init the oscconfig and create oscserver context*/
    g_stOSCConf.s32ListFiles = 10;
    g_stOSCConf.s32TimeOut = 180;

    s32Ret = HI_OSCSVR_Create(&pOSCHandle, &g_stOSCConf);

    if (HI_SUCCESS != s32Ret)
    {
        printf("create OSC server handle fail!!! s32Ret:%d\n", s32Ret);
        return HI_FAILURE;
    }

    s_Handle = pOSCHandle;
    s_bLooper = HI_TRUE;

    printf("create OSC server handle successfully\n");

    s32Ret = OSCSVR_Register_API_CallBack(&pOSCHandle);

    if (HI_SUCCESS != s32Ret)
    {
        printf("register OSC api fail!!! s32Ret:%d\n", s32Ret);
        (HI_VOID)HI_OSCSVR_ProcessExit(pOSCHandle);
        (HI_VOID)HI_OSCSVR_Destroy(&pOSCHandle);
        return HI_FAILURE;
    }

    s32Ret = OSCSVR_Register_COMM_CallBack(&pOSCHandle);

    if (HI_SUCCESS != s32Ret)
    {
        printf("register OSC comm fail!!! s32Ret:%d\n", s32Ret);
        (HI_VOID)HI_OSCSVR_ProcessExit(pOSCHandle);
        (HI_VOID)HI_OSCSVR_Destroy(&pOSCHandle);
        return HI_FAILURE;
    }

    printf("register OSC command and api successfully\n");

    s32Ret = OSCSVR_Create_Listener(s32HttpPort, &pOSCNetHandle);

    if (HI_SUCCESS != s32Ret)
    {
        printf("create OSC net handle s32Ret:%d\n", s32Ret);
        (HI_VOID)HI_OSCSVR_ProcessExit(pOSCHandle);
        (HI_VOID)HI_OSCSVR_Destroy(&pOSCHandle);
        return HI_FAILURE;
    }

    printf("create osc net work listen successfully\n");

    s32Ret = OSCSVR_Start(pOSCHandle, pOSCNetHandle);

    if (HI_SUCCESS != s32Ret)
    {
        printf("start OSC server fail!!! need to destroy the handle! s32Ret:%d\n", s32Ret);
        (HI_VOID)OSCSVR_Listener_Destroy(&pOSCNetHandle);
        (HI_VOID)HI_OSCSVR_ProcessExit(pOSCHandle);
        (HI_VOID)HI_OSCSVR_Destroy(&pOSCHandle);

        return HI_FAILURE;
    }

    if (getchar())
    {
        printf("start to stop the server !\n");
    }

    (HI_VOID)HI_OSCSVR_ProcessExit(pOSCHandle);

    printf("permit to stop the server !\n");
    s_bLooper = HI_FALSE;

    s32Ret = OSCSVR_Listener_Destroy(&pOSCNetHandle);

    if (HI_SUCCESS != s32Ret)
    {
        printf("Destroy OSC server listener!!!\n");
        return HI_FAILURE;
    }

    printf("Destroy OSC server listener successfully!!!\n");

    /*server stop process */
    if (HI_SUCCESS != HI_OSCSVR_Destroy(&pOSCHandle))
    {
        printf("Destroy OSC server fail!!!\n");
        return HI_FAILURE;
    }

    printf("Destroy OSC server successfully!!!\n");

    return HI_SUCCESS;
}

#ifdef __HuaweiLite__
int app_main(int argc, char* argv[])
#else
int main(int argc, char* argv[])
#endif
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 s32FileNum = 3;

    printf("ready to start osc module!\n");

    pthread_mutex_init(&g_OSCFILELock, NULL);

    s32Ret = OSCSVR_Init_ListFiles(s32FileNum);

    if (HI_SUCCESS != s32Ret)
    {
        printf("SAMPLE_Init_Files fail!!!\n");
        return HI_FAILURE;
    }

    s32Ret = SAMPLE_OSCSVR_Start();

    if (HI_SUCCESS != s32Ret)
    {
        printf("SAMPLE_OSCSVR_Start start fail!!!\n");
        return HI_FAILURE;
    }

    pthread_mutex_destroy(&g_OSCFILELock);

    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
