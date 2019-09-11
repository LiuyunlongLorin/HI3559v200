#ifndef GOLOMB_H
#define GOLOMB_H
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include "hi_mw_type.h"



HI_S32 FFBitsReader_create(HI_MW_PTR* ppReader , HI_U8* pu8Data, HI_U32 u32DataSize);

HI_S32 FFBitsReader_destroy(HI_MW_PTR pReader);

HI_BOOL u_1(HI_MW_PTR pReader);
HI_U8 u_2(HI_MW_PTR pReader);
HI_U8 u_3(HI_MW_PTR pReader);
HI_U8 u_4(HI_MW_PTR pReader);
HI_U8 u_5(HI_MW_PTR pReader);
HI_U8 u_6(HI_MW_PTR pReader);
HI_U8 u_8(HI_MW_PTR pReader);
HI_U16 u_16(HI_MW_PTR pReader);
HI_U32 u_32(HI_MW_PTR pReader);
HI_U32 ue_v(HI_MW_PTR pReader);
HI_S32 se_v(HI_MW_PTR pReader);
HI_U32 u_n(HI_MW_PTR pReader, HI_U8 u8Bits);



#endif
