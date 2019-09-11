#ifndef AACPARSER_H
#define AACPARSER_H
#include <inttypes.h>
#include "hi_mw_type.h"

typedef struct hiAAC_ADTS_HEADER_S
{
    /* fixed */
    HI_U32  u32Sync;                           /* syncword */
    HI_U8   u8ID;                             /* MPEG bit - should be 1 */
    HI_U8   u8Layer;                          /* MPEG u8Layer - should be 0 */
    HI_U8   u8ProtectAbsentBit;                     /* 0 = CRC word follows, 1 = no CRC word */
    HI_U8   u8Profile;                        /* 0 = main, 1 = LC, 2 = SSR, 3 = reserved */
    HI_U8   u8SampRateIdx;                    /* sample rate index range = [0, 11] */
    HI_U8   u8PrivateBit;                     /* ignore */
    HI_U8   u8ChannelConfig;                  /* 0 = implicit, >0 = use default table */
    HI_U8   u8OrigCopy;                       /* 0 = copy, 1 = original */
    HI_U8   u8Home;                           /* ignore */

    /* variable */
    HI_U8   u8CopyBit;                        /* 1 bit of the 72-bit copyright ID (transmitted as 1 bit per frame) */
    HI_U8   u8CopyStart;                      /* 1 = this bit starts the 72-bit ID, 0 = it does not */
    HI_U32 u32FrameLength;                    /* length of frame */
    HI_U32 u32BufferFull;                     /* number of 32-bit words left in enc buffer, 0x7FF = VBR */
    HI_U8   u8NumRawDataBlocks;               /* number of raw data blocks in frame */

    /* CRC */
    HI_S32   s32CrcCheckWord;                   /* 16-bit CRC check word (present if u8ProtectBit == 0) */
} AAC_ADTS_HEADER_S;



HI_S32 FFAACParser_parseAdtsHeader(AAC_ADTS_HEADER_S* pstHeader, HI_U8* pu8Sps, HI_U32 u32SpsLen,HI_U32* pu32HeaderLen);



#endif
