#include <inttypes.h>
#include <stdlib.h>
#include "../ffdemux_log.h"
#include "ffbitsreader.h"



typedef struct FFBitsReader
{
    HI_U8* mData;
    HI_U32 mSize;
    HI_U32 mOffset;
} FF_BITREADER;

static HI_U8 u_v(HI_MW_PTR pReader, HI_U8 u8Bits)
{
    HI_U8 u8Val = 0;
    for (HI_S32 i = 0; i < u8Bits; i++)
    {
        if (u_1(pReader))
        {
            u8Val += 1 << (u8Bits - i - 1);
        }
    }
    return u8Val;
}


HI_S32 FFBitsReader_create(HI_MW_PTR* ppReader , HI_U8* pu8Data, HI_U32 u32DataSize)
{
    if (ppReader == NULL || NULL == pu8Data || 0 == u32DataSize)
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "FFBitsReader_create fail illegal param\n");
        return HI_FAILURE;
    }
    FF_BITREADER* pstReader = NULL;
    pstReader = (FF_BITREADER*)malloc(sizeof(FF_BITREADER));
    if (NULL == pstReader)
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "FFBitsReader_create malloc fail\n");
        return HI_FAILURE;
    }

    pstReader->mData = pu8Data;
    pstReader->mSize = u32DataSize;
    pstReader->mOffset = 0;

    *ppReader = (HI_MW_PTR)pstReader;

    return HI_SUCCESS;
}

HI_S32 FFBitsReader_destroy(HI_MW_PTR pReader)
{
    if (NULL == pReader)
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "FFBitsReader_destroy  fail null param \n");
        return HI_FAILURE;
    }
    FF_BITREADER* pstReader = NULL;
    pstReader = (FF_BITREADER*)pReader;
    if (pstReader)
    {
        free(pstReader);
        pstReader = NULL;
    }

    return HI_SUCCESS;
}

HI_BOOL u_1(HI_MW_PTR pReader)
{
    if (NULL == pReader )
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "FFBitsReader  null param \n");
        return HI_FAILURE;
    }
    FF_BITREADER* pstReader = NULL;
    pstReader = (FF_BITREADER*)pReader;
    if (NULL == pstReader->mData )
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "FFBitsReader  null param \n");
        return HI_FAILURE;
    }


    if (pstReader->mOffset < pstReader->mSize * 8)
    {
        HI_U32 offset = pstReader->mOffset++;
        if (1 == ((pstReader->mData[offset / 8] >> (8 - offset % 8 - 1)) & 1) )
        {
            return HI_TRUE;
        }
        else
        {
            return HI_FALSE;
        }
    }
    else
    {
        printf( "OutOfRange Size %d Offset: %d \n", pstReader->mSize, pstReader->mOffset);
        return HI_FALSE;
    }
}

HI_U32 u_n(HI_MW_PTR pReader, HI_U8 u8Bits)
{
    HI_U32 u32Val = 0;
    for (HI_S32 i = 0; i < u8Bits; i++)
    {
        if (u_1(pReader))
        {
            u32Val += 1 << (u8Bits - i - 1);
        }
    }
    return u32Val;
}


HI_U8 u_2(HI_MW_PTR pReader)
{
    return u_v(pReader, 2);
}

HI_U8 u_3(HI_MW_PTR pReader)
{
    return u_v(pReader, 3);
}

HI_U8 u_4(HI_MW_PTR pReader)
{
    return u_v(pReader, 4);
}

HI_U8 u_5(HI_MW_PTR pReader)
{
    return u_v(pReader, 5);
}

HI_U8 u_6(HI_MW_PTR pReader)
{
    return u_v(pReader, 6);
}

HI_U8 u_8(HI_MW_PTR pReader)
{
    return u_v(pReader, 8);
}

HI_U16 u_16(HI_MW_PTR pReader)
{
    HI_U16 u16Val = 0;
    u16Val = u_8(pReader) << 8;
    u16Val += u_8(pReader);
    return u16Val;
}

HI_U32 u_32(HI_MW_PTR pReader)
{
    HI_U32 u32Val = 0;
    u32Val = u_8(pReader) << 24;
    u32Val += u_8(pReader) << 16;
    u32Val += u_8(pReader) << 8;
    u32Val += u_8(pReader);
    return u32Val;
}

HI_U32 ue_v(HI_MW_PTR pReader)
{
    HI_U32 u32CodeNum = 0;
    HI_U32 u32Suffix = 0;
    HI_U32 u32NumZeros = 0;
    HI_U32 u32TryTimes = 33;

    FF_BITREADER* pstReader = NULL;
    pstReader = (FF_BITREADER*)pReader;
    if (NULL == pstReader )
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "ue_v  null param \n");
        return 0;
    }

    while (!u_1(pReader) && pstReader->mOffset < pstReader->mSize * 8 && u32TryTimes > 0)
    {
        u32NumZeros++;
        u32TryTimes--;
    }

    for (HI_U32 i = 0; i < u32NumZeros; i++)
    {
        if (u_1(pReader))
        {
            u32Suffix += 1 << (u32NumZeros - i - 1);
        }
    }
    u32CodeNum = (1 << u32NumZeros) + u32Suffix - 1;
    return u32CodeNum;
}

HI_S32 se_v(HI_MW_PTR pReader)
{
    HI_S32 s32Val = 0;
    HI_U32 u32CodeNum = ue_v(pReader);
    if (u32CodeNum % 2)
    {
        s32Val = -1 * (u32CodeNum / 2);
    }
    else
    {
        s32Val = u32CodeNum / 2;
    }
    return s32Val;
}
