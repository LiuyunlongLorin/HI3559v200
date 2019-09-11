/**
 * @file    hi_filetrans.c
 * @brief   filetrans module interface.
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/25
 * @version   1.0

 */
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <errno.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <pthread.h>
#include "hi_exif.h"
#include "hi_mp4_format.h"
#include "filetrans_http.h"
#include "hi_filetrans.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define FILETRANS_MAX_LINK_NUM (16)
#define FILETRANS_JPG_SUFFIX   ".JPG"
#define FILETRANS_THM_SUFFIX   ".THM"
#define FILETRANS_MP4_SUFFIX   ".MP4"
#define FILETRANS_MAX_THM_SIZE (1024*200)

typedef struct tagFILETRANS_LINK_S
{
    pthread_t tid;
    HI_BOOL bFlag;
    HI_S32 s32fd;
} FILETRANS_LINK_S;

static pthread_mutex_t s_FILETRANSLinkListLock;
static FILETRANS_LINK_S s_astFILETRANSLinkList[FILETRANS_MAX_LINK_NUM];
static HI_BOOL s_bFILETRANSInitFlg = HI_FALSE;

static HI_S32 FILETRANS_GetThmInJpg(HI_CHAR* pszFileName, HI_U8* pu8Buf, HI_U32* pu32Size)
{
    HI_S32 s32Ret = 0;
    HI_S32 s32PicIndex = 0;
    HI_MW_PTR pExif;
    HI_EXIF_ATTR_S stExifAttr;
    stExifAttr.enExifConfigType = HI_EXIF_CONFIG_DEMUXER;
    stExifAttr.stDemuxerConfig.pszFileName = pszFileName;
    HI_LOG_PrintInfo(pszFileName);
    s32Ret = HI_EXIF_Open(&pExif, &stExifAttr);

    if (HI_SUCCESS != s32Ret)
    {
        HI_LOG_PrintFuncErr(HI_EXIF_Open, s32Ret);
        return HI_EINTER;
    }

    HI_EXIF_INFO_S stExifInfo;
    s32Ret = HI_EXIF_GetInfo(pExif, &stExifInfo);

    if (HI_SUCCESS != s32Ret)
    {
        HI_LOG_PrintFuncErr(HI_EXIF_GetInfo, s32Ret);
        *pu32Size = 0;
        goto EXIT;
    }

    for (s32PicIndex = (HI_EXIF_SUBIMG_MAX_CNT - 1); s32PicIndex >= 0; s32PicIndex--)
    {
        if (stExifInfo.stSubImg[s32PicIndex].bThumbnail)
        {
            break;
        }
    }

    if (0 > s32PicIndex)
    {
        s32PicIndex = 0;
    }

    HI_EXIF_DATA_S stExifData;
    stExifData.u32PicIndex = (HI_U32)s32PicIndex;
    stExifData.u32DataLen = stExifInfo.stSubImg[s32PicIndex].u32DataLen;

    if (0 == stExifData.u32DataLen || *pu32Size < stExifData.u32DataLen)
    {
        HI_LOG_PrintU32(stExifData.u32DataLen);
        *pu32Size = 0;
        goto EXIT;
    }

    stExifData.pu8Data = pu8Buf;
    HI_LOG_PrintS32(s32PicIndex);
    s32Ret = HI_EXIF_ReadPicData(pExif, &stExifData);

    if (HI_SUCCESS != s32Ret)
    {
        HI_LOG_PrintFuncErr(HI_EXIF_ReadPicData, s32Ret);
        *pu32Size = 0;
        goto EXIT;
    }

    *pu32Size = stExifData.u32DataLen;
EXIT:
    HI_EXIF_Close(pExif);
    return s32Ret;
}

static HI_S32 FILETRANS_GetThmInMp4(HI_CHAR* pszFileName, HI_U8* pu8Buf, HI_U32* pu32Size)
{
    HI_S32 s32Ret = 0;
    HI_MW_PTR pMp4Demuxer = NULL;
    HI_MP4_CONFIG_S stMp4DemuxerCfg ;
    snprintf(stMp4DemuxerCfg.aszFileName, HI_MP4_MAX_FILE_NAME, pszFileName);
    stMp4DemuxerCfg.enConfigType = HI_MP4_CONFIG_DEMUXER;
    stMp4DemuxerCfg.stDemuxerConfig.u32VBufSize = *pu32Size;
    HI_LOG_PrintInfo(pszFileName);
    s32Ret = HI_MP4_Create(&pMp4Demuxer, &stMp4DemuxerCfg);

    if (HI_SUCCESS != s32Ret)
    {
        HI_LOG_PrintFuncErr(HI_MP4_Create, s32Ret);
        return HI_EINTER;
    }

    HI_MP4_ATOM_INFO_S stAtomInfo;
    stAtomInfo.u32DataLen = *pu32Size;
    stAtomInfo.pu8DataBuf = pu8Buf;
    s32Ret = HI_MP4_GetAtom(pMp4Demuxer, "/thm ", &stAtomInfo);

    if (s32Ret != HI_SUCCESS)
    {
        HI_LOG_PrintFuncErr(HI_MP4_GetAtom, s32Ret);
        *pu32Size = 0;
        goto EXIT;
    }

    if (*pu32Size < stAtomInfo.u32DataLen)
    {
        HI_LOG_PrintU32(stAtomInfo.u32DataLen);
        *pu32Size = 0;
        goto EXIT;
    }

    *pu32Size = stAtomInfo.u32DataLen;
EXIT:
    HI_MP4_Destroy(pMp4Demuxer, NULL);
    return s32Ret;
}

static HI_S32 FILETRANS_HandleReq(HI_S32 s32Socket, HI_CHAR* pszRequest)
{
    HI_S32 s32RetCode = -1;
    /* Parse request packet */
    FILETRANS_REQ_INFO_S stReqInfo;
    FILETRANS_RESP_INFO_S stRespInfo;
    memset(&stReqInfo, 0, sizeof(stReqInfo));
    memset(&stRespInfo, 0, sizeof(stRespInfo));
    s32RetCode = FILETRANS_ParseHttpReq(pszRequest, &stReqInfo);
    HI_APPCOMM_CHECK_RETURN(s32RetCode, HI_EINVAL);
    snprintf(stRespInfo.szConnection, FILETRANS_CONNECTION_LEN, stReqInfo.bKeepAlive ? FILETRANS_HTTP_KEEPALIVE : FILETRANS_HTTP_CLOSE);
    HI_LOG_PrintStr(stReqInfo.szFileName);
    HI_CHAR szResolvedPath[PATH_MAX+1] = {0};

    if (NULL != realpath(stReqInfo.szFileName, szResolvedPath))
    {
        /**File exsit */
        FILE* fpFile = fopen(szResolvedPath, "rb");

        if (NULL != fpFile)
        {
            s32RetCode = fseek(fpFile, 0, SEEK_END);

            if (HI_SUCCESS != s32RetCode)
            {
                MLOGE("fseek fail:%s\n", strerror(errno));
                fclose(fpFile);
                stRespInfo.s32ReturnCode = 500;
                goto SendResp;
            }

            stRespInfo.s32ContentLen = ftell(fpFile);
            s32RetCode = fseek(fpFile, 0, SEEK_SET);

            if (HI_SUCCESS != s32RetCode)
            {
                MLOGE("fseek fail:%s\n", strerror(errno));
                fclose(fpFile);
                stRespInfo.s32ContentLen = 0;
                stRespInfo.s32ReturnCode = 500;
                goto SendResp;
            }

            stRespInfo.s32ReturnCode = 200;

            if (NULL != strstr(stReqInfo.szFileName, FILETRANS_THM_SUFFIX)
                || NULL != strstr(stReqInfo.szFileName, FILETRANS_JPG_SUFFIX))
            {
                snprintf(stRespInfo.szContentType, FILETRANS_CONTENT_TYPE_LEN, "image/jpeg");
            }

            s32RetCode = FILETRANS_SendHttpResp(s32Socket, &stRespInfo);

            if (HI_SUCCESS != s32RetCode)
            {
                fclose(fpFile);
                return s32RetCode;
            }

            s32RetCode = FILETRANS_SendFile(s32Socket, fpFile);
            fclose(fpFile);
            return s32RetCode;
        }
        else
        {
            stRespInfo.s32ReturnCode = 403;
            goto SendResp;
        }

    }
    else
    {
        HI_CHAR* pstTemp = strstr(stReqInfo.szFileName, FILETRANS_THM_SUFFIX);

        if (NULL != pstTemp)
        {
            /**file not exsit and request thm */
            HI_U32 u32ThmSize = FILETRANS_MAX_THM_SIZE;
#ifndef __HuaweiLite__
            HI_U8 u8ThmDataBuf[FILETRANS_MAX_THM_SIZE] = {0};
#else
            HI_U8* u8ThmDataBuf = (HI_U8*)malloc(u32ThmSize*sizeof(HI_U8));
#endif
            HI_CHAR szFileNameSrc[HI_APPCOMM_MAX_PATH_LEN] = {0};
            snprintf(szFileNameSrc, MIN(HI_APPCOMM_MAX_PATH_LEN, pstTemp - stReqInfo.szFileName + 1), stReqInfo.szFileName);
            strncat(szFileNameSrc, FILETRANS_JPG_SUFFIX, 4);

            if (NULL != realpath(szFileNameSrc, szResolvedPath))
            {
                s32RetCode = FILETRANS_GetThmInJpg(szResolvedPath, u8ThmDataBuf, &u32ThmSize);
            }
            else
            {
                snprintf(szFileNameSrc, MIN(HI_APPCOMM_MAX_PATH_LEN, pstTemp - stReqInfo.szFileName + 1), stReqInfo.szFileName);
                strncat(szFileNameSrc, FILETRANS_MP4_SUFFIX, 4);

                if (NULL != realpath(szFileNameSrc, szResolvedPath))
                {
                    s32RetCode = FILETRANS_GetThmInMp4(szResolvedPath, u8ThmDataBuf, &u32ThmSize);
                }
                else
                {
                    /**file not exist & not thm file */
                    stRespInfo.s32ReturnCode = 404;
#ifdef __HuaweiLite__
                    HI_APPCOMM_SAFE_FREE(u8ThmDataBuf);
#endif
                    goto SendResp;
                }
            }

            if (0 < u32ThmSize && FILETRANS_MAX_THM_SIZE >= u32ThmSize)
            {
                stRespInfo.s32ReturnCode = 200;
                stRespInfo.s32ContentLen = u32ThmSize;
                snprintf(stRespInfo.szContentType, FILETRANS_CONTENT_TYPE_LEN, "image/jpeg");

                s32RetCode = FILETRANS_SendHttpResp(s32Socket, &stRespInfo);

                if (HI_SUCCESS != s32RetCode)
                {
#ifdef __HuaweiLite__
                    HI_APPCOMM_SAFE_FREE(u8ThmDataBuf);
#endif
                    return s32RetCode;
                }

                s32RetCode = FILETRANS_SendData(s32Socket, u8ThmDataBuf, u32ThmSize);
#ifdef __HuaweiLite__
                HI_APPCOMM_SAFE_FREE(u8ThmDataBuf);
#endif
                return s32RetCode;
            }
            else
            {
                MLOGE("Get Thm Fail.\n");
                stRespInfo.s32ReturnCode = 404;
#ifdef __HuaweiLite__
                HI_APPCOMM_SAFE_FREE(u8ThmDataBuf);
#endif
                goto SendResp;
            }
        }
        else
        {
            /**file not exist & not thm file */
            stRespInfo.s32ReturnCode = 404;
            goto SendResp;
        }
    }

SendResp:
    return FILETRANS_SendHttpResp(s32Socket, &stRespInfo);
}

static HI_VOID* FILETRANS_ThreadProc(HI_VOID* pvArg)
{
    prctl(PR_SET_NAME, __func__, 0, 0, 0);
    HI_S32 s32RtnCode = -1;
    HI_CHAR szRecvBuf[FILETRANS_HTTP_BUFFLEN];
    FILETRANS_LINK_S* pstLink = (FILETRANS_LINK_S*)pvArg;

    while (pstLink->bFlag)
    {
        s32RtnCode = FILETRANS_RecvHttpReq(pstLink->s32fd, szRecvBuf, sizeof(szRecvBuf));

        if (HI_SUCCESS != s32RtnCode)
        {
            pthread_detach(pthread_self());
            break;
        }

        s32RtnCode = FILETRANS_HandleReq(pstLink->s32fd, szRecvBuf);

        if (HI_SUCCESS != s32RtnCode)
        {
            pthread_detach(pthread_self());
            break;
        }
    }

    pstLink->bFlag = HI_FALSE;
    close(pstLink->s32fd);
    HI_LOG_FuncExit();
    return NULL;
}

/**
 * @brief    init filetrans module.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/25
 */
HI_S32 HI_FILETRANS_Init(HI_VOID)
{
    if (HI_TRUE == s_bFILETRANSInitFlg)
    {
        return HI_EINITIALIZED;
    }
    HI_EXIF_Init();
    HI_MUTEX_INIT_LOCK(s_FILETRANSLinkListLock);
    memset(s_astFILETRANSLinkList, 0, FILETRANS_MAX_LINK_NUM * sizeof(FILETRANS_LINK_S));
    s_bFILETRANSInitFlg = HI_TRUE;
    return  HI_SUCCESS;
}

/**
 * @brief    deinit filetrans module.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/25
 */
HI_S32 HI_FILETRANS_Deinit(HI_VOID)
{
    if (HI_FALSE == s_bFILETRANSInitFlg)
    {
        return HI_ENOINIT;
    }

    HI_FILETRANS_CloseAllLink();
    s_bFILETRANSInitFlg = HI_FALSE;
    HI_MUTEX_DESTROY(s_FILETRANSLinkListLock);
    HI_EXIF_DeInit();
    return  HI_SUCCESS;
}

/**
 * @brief    parse file path, and transmit the file with http protocol.
 * @param[in] s32Socket:http socket.
 * @param[in] pszRequest:http requset packet.
 * @param[in] s32ReqLen:http requset packet length.
 * @return 0 success,non-zero error code.
 * @exception    register this function to webserver and take over the socket.
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/25
 */
HI_S32 HI_FILETRANS_DistribLink(HI_S32 s32Socket, HI_CHAR* pszRequest, HI_U32 s32ReqLen)
{
    HI_S32 s32RtnCode = -1;
    HI_S32 i;
    /* Check Input Param */
    HI_APPCOMM_CHECK_EXPR(0 < s32Socket, HI_EINVAL);
    HI_APPCOMM_CHECK_POINTER(pszRequest, HI_EINVAL);
    HI_APPCOMM_CHECK_EXPR(0 < s32ReqLen, HI_EINVAL);

    if (HI_FALSE == s_bFILETRANSInitFlg)
    {
        return HI_ENOINIT;
    }

    s32RtnCode = FILETRANS_HandleReq(s32Socket, pszRequest);

    if (HI_SUCCESS != s32RtnCode)
    {
        close(s32Socket);
        return HI_EINTER;
    }

    if ((NULL == strstr(pszRequest, FILETRANS_HTTP_KEEPALIVE)) && (NULL == strstr(pszRequest, "keep-alive")))
    {
        close(s32Socket);
        return HI_SUCCESS;
    }

    HI_MUTEX_LOCK(s_FILETRANSLinkListLock);

    for (i = 0; i < FILETRANS_MAX_LINK_NUM; i++)
    {
        if (HI_FALSE == s_astFILETRANSLinkList[i].bFlag)
        {
            s_astFILETRANSLinkList[i].bFlag = HI_TRUE;
            s_astFILETRANSLinkList[i].s32fd = s32Socket;
            s32RtnCode = pthread_create(&(s_astFILETRANSLinkList[i].tid), NULL, FILETRANS_ThreadProc, (HI_VOID*)&s_astFILETRANSLinkList[i]);

            if (HI_SUCCESS != s32RtnCode)
            {
                MLOGE("pthread_create fail:%s\n", strerror(errno));
                s_astFILETRANSLinkList[i].bFlag = HI_FALSE;
                close(s32Socket);
                s_astFILETRANSLinkList[i].s32fd = -1;
                HI_MUTEX_UNLOCK(s_FILETRANSLinkListLock);
                return HI_ENORES;
            }

            HI_MUTEX_UNLOCK(s_FILETRANSLinkListLock);
            return HI_SUCCESS;
        }
    }

    HI_MUTEX_UNLOCK(s_FILETRANSLinkListLock);
    close(s32Socket);
    MLOGE("reach the maximum number of connections %d\n", FILETRANS_MAX_LINK_NUM);
    return HI_ENORES;
}

/**
 * @brief    close all linked socket.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/25
 */
HI_S32 HI_FILETRANS_CloseAllLink(HI_VOID)
{
    HI_S32 i;

    if (HI_FALSE == s_bFILETRANSInitFlg)
    {
        return HI_ENOINIT;
    }

    HI_MUTEX_LOCK(s_FILETRANSLinkListLock);

    for (i = 0; i < FILETRANS_MAX_LINK_NUM; i++)
    {
        if (s_astFILETRANSLinkList[i].bFlag)
        {
            s_astFILETRANSLinkList[i].bFlag = HI_FALSE;
            pthread_join(s_astFILETRANSLinkList[i].tid, NULL);
        }
    }

    HI_MUTEX_UNLOCK(s_FILETRANSLinkListLock);
    return HI_SUCCESS;
}

/**
 * @brief    get link number.
 * @return   link number.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/25
 */
HI_S32 HI_FILETRANS_GetLinkNum(HI_VOID)
{
    HI_S32 i;
    HI_S32 LinkNum = 0;

    if (HI_FALSE == s_bFILETRANSInitFlg)
    {
        return 0;
    }

    HI_MUTEX_LOCK(s_FILETRANSLinkListLock);

    for (i = 0; i < FILETRANS_MAX_LINK_NUM; i++)
    {
        if (s_astFILETRANSLinkList[i].bFlag)
        {
            LinkNum++;
        }
    }

    HI_MUTEX_UNLOCK(s_FILETRANSLinkListLock);
    return LinkNum;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
