#include <string.h>
#include "zlib.h"
#include "hi_type.h"

#define ADD_BITS_NUMBER (16)
#if (defined(HI3559V200) || defined(HI3556V200))
#define COMPRESS_WBITS  11  /**<Window Size: 8K */
#else
#define COMPRESS_WBITS  15  /**<Window Size: 32K */
#endif

HI_S32 HI_Compress(HI_U8* pu8Data, HI_U32 u32DataLen, HI_U8* pu8ZData, HI_U32* pu32ZDataLen)
{
#define ADD_LEVEL_NUMBER (8)

    z_stream stStream, *pstStream;
    pstStream = &stStream;
    memset(&stStream, 0, sizeof(stStream));
    HI_S32 err = 0;

    if (pu8Data && u32DataLen > 0)
    {
        if (deflateInit2(&stStream, Z_DEFAULT_COMPRESSION, Z_DEFLATED,
            COMPRESS_WBITS + ADD_BITS_NUMBER, ADD_LEVEL_NUMBER, Z_DEFAULT_STRATEGY) != HI_SUCCESS)
        {
            return HI_FAILURE;
        }

        pstStream->next_in  = pu8Data;
        pstStream->avail_in  = u32DataLen;
        pstStream->next_out = pu8ZData;
        pstStream->avail_out  = *pu32ZDataLen;

        while (pstStream->avail_in != HI_NULL && pstStream->total_out < *pu32ZDataLen)
        {
            if (deflate(pstStream, Z_NO_FLUSH) != HI_SUCCESS)
            {
                return HI_FAILURE;
            }
        }

        if (pstStream->avail_in != 0)
        {
            return pstStream->avail_in;
        }

        while (1)
        {
            if ((err = deflate(pstStream, Z_FINISH)) == Z_STREAM_END)
            {
                break;
            }

            if (err != HI_SUCCESS)
            {
                return HI_FAILURE;
            }
        }

        if (deflateEnd(pstStream) != HI_SUCCESS)
        {
            return HI_FAILURE;
        }

        *pu32ZDataLen = pstStream->total_out;
        return HI_SUCCESS;
    }

    return HI_FAILURE;
}

HI_S32 HI_Decompress(HI_U8* pu8zData, HI_U32 nzData, HI_U8* pu8Data, HI_U32* pnDataLen)
{
    HI_S32 err = 0;
    z_stream d_stream = {0}; /* decompression stream */
    static HI_CHAR dummy_head[2] =
    {
        0x8 + 0x7 * 0x10,
        (((0x8 + 0x7 * 0x10) * 0x100 + 30) / 31 * 31) & 0xFF,
    };
    d_stream.zalloc = NULL;
    d_stream.zfree = NULL;
    d_stream.opaque = NULL;
    d_stream.next_in  = pu8zData;
    d_stream.avail_in = 0;
    d_stream.next_out = pu8Data;

    if (inflateInit2(&d_stream, COMPRESS_WBITS + ADD_BITS_NUMBER) != Z_OK) { return -1; }

    while (d_stream.total_out < *pnDataLen && d_stream.total_in < nzData)
    {
        d_stream.avail_in = d_stream.avail_out = 1; /* force small buffers */

        if ((err = inflate(&d_stream, Z_NO_FLUSH)) == Z_STREAM_END) { break; }

        if (err != Z_OK)
        {
            if (err == Z_DATA_ERROR)
            {
                d_stream.next_in = (HI_U8*)dummy_head;
                d_stream.avail_in = sizeof(dummy_head);

                if ((err = inflate(&d_stream, Z_NO_FLUSH)) != Z_OK)
                {
                    return -1;
                }
            }
            else { return -1; }
        }
    }

    if (inflateEnd(&d_stream) != Z_OK) { return -1; }

    *pnDataLen = d_stream.total_out;
    return HI_SUCCESS;
}
