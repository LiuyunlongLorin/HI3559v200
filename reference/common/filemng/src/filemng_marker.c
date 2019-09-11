/**
 * @file    filemng_marker.c
 * @brief   file manager marker function.
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/19
 * @version   1.0

 */
#include <string.h>
#include <errno.h>
#include "hi_appcomm_util.h"
#include "filemng_comm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */


#define FILEMNG_MARKER_FAVORITES ".Favorites/"/**<This directory will save the marked files */
static HI_CHAR s_szTopDir[HI_APPCOMM_MAX_PATH_LEN];
static HI_CHAR s_szFavorites[HI_APPCOMM_MAX_PATH_LEN];

HI_S32 FILEMNG_MARKER_Init(const HI_CHAR *pszTopDir)
{
    HI_S32 s32Ret = 0;
    snprintf(s_szTopDir, HI_APPCOMM_MAX_PATH_LEN, pszTopDir);
    snprintf(s_szFavorites, HI_APPCOMM_MAX_PATH_LEN, "%s%s", pszTopDir, FILEMNG_MARKER_FAVORITES);
    s32Ret = HI_PathIsDirectory(s_szFavorites);
    if (1 == s32Ret) {
        FILEMNG_HideDir(s_szFavorites, HI_TRUE);
        return HI_SUCCESS;
    } else if (-1 == s32Ret) {
        s32Ret = HI_mkdir(s_szFavorites, 0777);
        if (HI_SUCCESS != s32Ret) {
            return HI_FILEMNG_EINTER;
        }
        FILEMNG_HideDir(s_szFavorites, HI_TRUE);
        return HI_SUCCESS;
    } else {
        return HI_FILEMNG_EINTER;
    }
}

HI_S32 FILEMNG_MARKER_SetFlag(const HI_CHAR *pszFileName, HI_U8 u8Flag)
{
    HI_S32 s32Ret = 0;
    HI_CHAR szFavFile[HI_APPCOMM_MAX_PATH_LEN];
    HI_CHAR *pstr = (HI_CHAR *)pszFileName + strnlen(s_szTopDir, HI_APPCOMM_MAX_PATH_LEN);
    snprintf(szFavFile, HI_APPCOMM_MAX_PATH_LEN, "%s%s%s", s_szTopDir, FILEMNG_MARKER_FAVORITES, pstr);
    s32Ret = HI_PathIsDirectory(szFavFile);
    if (-1 == s32Ret) {
        s32Ret = HI_mkdir(szFavFile, 0777);
        if (HI_SUCCESS != s32Ret) {
            return HI_FILEMNG_EINTER;
        }
        s32Ret = HI_rmdir(szFavFile);
        if (HI_SUCCESS != s32Ret) {
            return HI_FILEMNG_EINTER;
        }
    } else if (1 == s32Ret) {
        return HI_FILEMNG_EEXIST;
    }

    FILE *fp = fopen(szFavFile, "wb");
    if (NULL != fp) {
        if (1 == fwrite(&u8Flag, 1, 1, fp)) {
            fclose(fp);
            return HI_SUCCESS;
        }
        fclose(fp);
    }

    MLOGE("MARKER_SetFlag Fail:%s\n", strerror(errno));
    return HI_FILEMNG_EINTER;
}

HI_S32 FILEMNG_MARKER_GetFlag(const HI_CHAR *pszFileName, HI_U8 *pu8Flag)
{
    HI_CHAR szFavFile[HI_APPCOMM_MAX_PATH_LEN];
    HI_CHAR *pstr = (HI_CHAR *)pszFileName + strnlen(s_szTopDir, HI_APPCOMM_MAX_PATH_LEN);
    snprintf(szFavFile, HI_APPCOMM_MAX_PATH_LEN, "%s%s%s", s_szTopDir, FILEMNG_MARKER_FAVORITES, pstr);
    *pu8Flag = 0;
    FILE *fp = fopen(szFavFile, "rb");
    if (NULL != fp) {
        fread(pu8Flag, 1, 1, fp);
        fclose(fp);
    }
    return HI_SUCCESS;
}

HI_S32 FILEMNG_MARKER_CleanFlag(const HI_CHAR *pszFileName)
{
    HI_S32 s32Ret = 0;
    HI_CHAR szFavFile[HI_APPCOMM_MAX_PATH_LEN];
    HI_CHAR *pstr = (HI_CHAR *)pszFileName + strnlen(s_szTopDir, HI_APPCOMM_MAX_PATH_LEN);
    snprintf(szFavFile, HI_APPCOMM_MAX_PATH_LEN, "%s%s%s", s_szTopDir, FILEMNG_MARKER_FAVORITES, pstr);
    s32Ret = HI_PathIsDirectory(szFavFile);
    if (0 == s32Ret) {
        remove(szFavFile);
    }
    return HI_SUCCESS;
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

