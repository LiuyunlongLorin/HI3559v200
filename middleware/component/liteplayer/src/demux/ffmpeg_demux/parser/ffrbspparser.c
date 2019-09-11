#include "ffrbspparser.h"

void genSODPByRBSP(HI_U8* pu8Src, HI_U32 u32SrcLen, HI_U8* pu8Dst)
{
    HI_U32 u32NumBytesInRbsp = 0;
    for (HI_U32 i = 0; i < u32SrcLen; i++)
    {
        if (i + 2 < u32SrcLen && 0x00 == pu8Src[i]
            && 0x00 == pu8Src[i + 1] && 0x03 == pu8Src[i + 2])
        {
            pu8Dst[u32NumBytesInRbsp++] = pu8Src[i];
            pu8Dst[u32NumBytesInRbsp++] = pu8Src[i + 1];
            i += 2;
        }
        else
        {
            pu8Dst[u32NumBytesInRbsp++] = pu8Src[i];
        }
    }
}
