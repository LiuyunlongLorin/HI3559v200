#include "securec.h"
#include "../ffdemux_log.h"
#include "ffaacparser.h"
#include "ffbitsreader.h"

HI_S32 FFAACParser_parseAdtsHeader(AAC_ADTS_HEADER_S* pstHeader, HI_U8* pData, HI_U32 u32DataLen, HI_U32* pu32HeaderLen)
{
    HI_S32 s32Ret = 0;
    HI_U32 u32Ret = 0;
    if ((NULL == pstHeader ) || (NULL == pData ) || (NULL == pu32HeaderLen ) || (0 == u32DataLen ))
    {
        return HI_FAILURE;
    }

    AAC_ADTS_HEADER_S mAdtsHeader = {0};

    HI_MW_PTR bitReader =  0;
    s32Ret = FFBitsReader_create(&bitReader, pData, u32DataLen);
    if (HI_SUCCESS != s32Ret)
    {
        return HI_FAILURE;
    }

    mAdtsHeader.u32Sync = u_n(bitReader, 12);   /* syncword */
    if ((HI_U32)0xFFF != mAdtsHeader.u32Sync)
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "audio syncword error %x \n", mAdtsHeader.u32Sync);
        s32Ret = HI_FAILURE;
        goto error;
    }

    mAdtsHeader.u8ID = u_1(bitReader);       /* ID */
    mAdtsHeader.u8Layer = u_2(bitReader);        /* layer */
    mAdtsHeader.u8ProtectAbsentBit = u_1(bitReader);        /* protection_absent */
    mAdtsHeader.u8Profile = u_2(bitReader); /* profile_objecttype */
    HI_U8 sample_rate_index = u_4(bitReader);
    mAdtsHeader.u8SampRateIdx = sample_rate_index;
    mAdtsHeader.u8PrivateBit = u_1(bitReader);        /* private_bit */
    HI_U8 channel_conf = u_3(bitReader); /* channel_configuration */
    mAdtsHeader.u8ChannelConfig = channel_conf;

    mAdtsHeader.u8OrigCopy = u_1(bitReader);        /* original_copy */
    mAdtsHeader.u8Home = u_1(bitReader);        /* home */

    /* adts_variable_header */
    mAdtsHeader.u8CopyBit = u_1(bitReader);        /* copyright_identification_bit */
    mAdtsHeader.u8CopyStart = u_1(bitReader);        /* copyright_identification_start */
    HI_U32 u32FrameLen = u_n(bitReader, 13); /* aac_frame_length */
    mAdtsHeader.u32FrameLength = u32FrameLen;
    mAdtsHeader.u32BufferFull = u_n(bitReader, 11);  /* adts_buffer_fullness */
    mAdtsHeader.u8NumRawDataBlocks = u_2(bitReader);        /* number_of_raw_data_blocks_in_frame */

    if ( 0 == mAdtsHeader.u8ProtectAbsentBit )
    {
        mAdtsHeader.s32CrcCheckWord = u_16(bitReader);       /* 16-bit CRC check word (present if u8ProtectBit == 0) */
        u32Ret = 9;
    }
    else
    {
        u32Ret = 7;
    }
    *pu32HeaderLen = u32Ret;

    if (HI_SUCCESS != memcpy_s(pstHeader, sizeof(mAdtsHeader), &mAdtsHeader, sizeof(mAdtsHeader))) //get atds info
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "copy pstHeader error \n");
    }
error:
    FFBitsReader_destroy(bitReader);

    return s32Ret;

}
