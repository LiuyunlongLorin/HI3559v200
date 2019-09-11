/**
 * @file      hi_product_param_bin2image.c
 * @brief     param bin2image tool
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/3/29
 * @version   1.0

 */

#include <stdio.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "hi_cycle.h"
#include "hi_appcomm_log.h"
#include "hi_product_param_inner.h"
#include "zlib.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif

#define PDT_PARAM_IMG_SIZE (0x10000)
#define PDT_PARAM_SIZE     (0x10000)

/** cycle data compress align bytes */
#define BYTE_ALIGN ((HI_U32)16)  /* needed by decompress */

/** param config file path */
#define PARAM_PATH         "./param.bin"

/** param image file path */
#define PARAM_IMAGE        "./rawparam"

HI_S32 main()
{
    HI_CHAR szParamBuf[PDT_PARAM_IMG_SIZE];
    HI_CHAR *pszBuf = szParamBuf;
    HI_U32  u32ImgLen = 0;
    memset(szParamBuf, 0xff, PDT_PARAM_IMG_SIZE);

    /* Load Param Configure */
    HI_S32 s32Fd = HI_APPCOMM_FD_INVALID_VAL;
    PDT_PARAM_CFG_S stParamCfg;
    memset(&stParamCfg, 0, sizeof(PDT_PARAM_CFG_S));
    s32Fd = open(PARAM_PATH, O_RDWR, 777);
    if (s32Fd < 0)
    {
        MLOGE("load [%s] failed\n", PARAM_PATH);
        return HI_FAILURE;
    }
    ssize_t readCount = read(s32Fd, &stParamCfg, sizeof(PDT_PARAM_CFG_S));
    if (0 > readCount)
    {
        MLOGE("read %s failed\n", PARAM_PATH);
        close(s32Fd);
        return HI_FAILURE;
    }
    close(s32Fd);
    MLOGD("ParamSize[%lu]\n", (HI_UL)sizeof(PDT_PARAM_CFG_S));

    /* Fill CycleHead Information */
    HI_CYCLE_HEAD_S *pstCycleHead   = (HI_CYCLE_HEAD_S*)pszBuf;
    pstCycleHead->u32MagicHead      = CYCLE_MAGIC_HEAD;
    pstCycleHead->u32CycleFlashSize = PDT_PARAM_SIZE;
    pstCycleHead->u32Compress       = PDT_PARAM_COMPRESS;
    pstCycleHead->u32WriteFlag      = HI_CYCLE_WRITE_FLAG_BURN;
    pstCycleHead->u32AlignSize      = (CFG_FLASH_PAGESIZE < BYTE_ALIGN) ? BYTE_ALIGN : CFG_FLASH_PAGESIZE;
    MLOGD("cycle head: magichead[0x%08x], u32CycleFlashSize[%08x], compress[%u], alignsize[%u]\n",
        pstCycleHead->u32MagicHead, pstCycleHead->u32CycleFlashSize,
        pstCycleHead->u32Compress, pstCycleHead->u32AlignSize);
    MLOGD("cycle head align length[%lu]\n",
        HI_APPCOMM_ALIGN(sizeof(HI_CYCLE_HEAD_S), pstCycleHead->u32AlignSize));
    pszBuf    += HI_APPCOMM_ALIGN(sizeof(HI_CYCLE_HEAD_S), pstCycleHead->u32AlignSize);
    u32ImgLen += HI_APPCOMM_ALIGN(sizeof(HI_CYCLE_HEAD_S), pstCycleHead->u32AlignSize);

    /* Compress Param Configure */
    HI_U8  szCompressBuf[PDT_PARAM_SIZE] = {0,};
    HI_U32 u32ZLen = PDT_PARAM_SIZE;
    if (pstCycleHead->u32Compress)
    {
        extern HI_S32 HI_Compress(HI_U8* pu8Data, HI_U32 u32DataLen, HI_U8* pu8ZData, HI_U32* pu32ZDataLen);
        HI_Compress((HI_U8*)&stParamCfg, sizeof(PDT_PARAM_CFG_S), szCompressBuf, &u32ZLen);
    }
    else
    {
        if(sizeof(PDT_PARAM_CFG_S) > PDT_PARAM_SIZE)
        {
            return HI_FAILURE;
        }

        memcpy(szCompressBuf, &stParamCfg, sizeof(PDT_PARAM_CFG_S));
        u32ZLen = sizeof(PDT_PARAM_CFG_S);
    }
    u32ZLen = HI_APPCOMM_ALIGN(u32ZLen, BYTE_ALIGN);
    MLOGD("original len[%lu], compress len[%u]\n", (HI_UL)sizeof(PDT_PARAM_CFG_S), u32ZLen);

    /* Fill Item Head Information */
    HI_U32 u32ItemOffset = 0;
    HI_CYCLE_ITEM_START_S *pstItemHead = (HI_CYCLE_ITEM_START_S*)pszBuf;
    pstItemHead->u32MagicItemStart     = CYCLE_MAGIC_ITEM_START;
    pstItemHead->u32ItemOriginLen      = sizeof(PDT_PARAM_CFG_S);
    pstItemHead->u32ItemLen            = u32ZLen;
    MLOGD("item head: magichead[0x%08x], originlen[%u], itemlen[%u]\n",
        pstItemHead->u32MagicItemStart, pstItemHead->u32ItemOriginLen, pstItemHead->u32ItemLen);
    u32ItemOffset += HI_APPCOMM_ALIGN(sizeof(HI_CYCLE_ITEM_START_S), BYTE_ALIGN); /* Align Item Head */
    MLOGD("u32ItemOffset[%d]\n", u32ItemOffset);

    if((u32ItemOffset + u32ZLen) > PDT_PARAM_IMG_SIZE)
    {
        return HI_FAILURE;
    }

    /* Copy Compressed Param Configure */
    memcpy(pszBuf + u32ItemOffset, szCompressBuf, u32ZLen);
    u32ItemOffset += u32ZLen;
    MLOGD("u32ItemOffset[%d]\n", u32ItemOffset);

    /* Fill Item End Information */
    HI_U32* pu32End = (HI_U32*)(pszBuf + u32ItemOffset);
    *pu32End        = CYCLE_MAGIC_ITEM_END;
    u32ItemOffset  += sizeof(HI_U32);

    /* Align Item */
    pszBuf    += HI_APPCOMM_ALIGN(u32ItemOffset, pstCycleHead->u32AlignSize);
    u32ImgLen += HI_APPCOMM_ALIGN(u32ItemOffset, pstCycleHead->u32AlignSize);
    MLOGD("Image Length[%u]\n", u32ImgLen);

    /* Update Item Total Length */
    pstItemHead->u32ItemAllLen = (unsigned long)pszBuf - (unsigned long)pstItemHead;
    MLOGD("item alllen[%u]\n", pstItemHead->u32ItemAllLen);

    /* Save Param Image */
    FILE *fp = NULL;
    HI_U32 u32Count = 0;
    fp = fopen(PARAM_IMAGE, "w+b");
    if(!fp)
    {
        MLOGE("open [%s] failed\n", PARAM_IMAGE);
        return HI_FAILURE;
    }
    u32Count = fwrite(szParamBuf, PDT_PARAM_IMG_SIZE, 1, fp);
    if(u32Count != 1)
    {
        MLOGE("fwrite : total %u, write %u\n", u32ImgLen, u32Count);
    }
    fclose(fp);
    fp = NULL;

    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

