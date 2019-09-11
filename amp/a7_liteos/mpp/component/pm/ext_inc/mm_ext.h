/******************************************************************************

  Copyright (C), 2001-2011, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : mm_ext.h
  Version       : Initial Draft
  Author        : Hisilicon multimedia software group
  Created       : 2010/11/16
  Description   :
  History       :
  1.Date        : 2010/11/16
    Author      : Z44949
    Modification: Created file

******************************************************************************/
#ifndef __MM_EXT_H__
#define __MM_EXT_H__

#include "hi_osal.h"
#include "hi_math.h"
#include "hi_common.h"
#include "osal_mmz.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

extern HI_U64 CMPI_MmzMalloc(HI_CHAR *pMmzName, HI_CHAR *pBufName, HI_UL ulSize);
extern HI_VOID CMPI_MmzFree(HI_U64 u64PhyAddr, HI_VOID *pVirAddr);
extern HI_S32 CMPI_MmzMallocNocache(HI_CHAR *cpMmzName, HI_CHAR *pBufName,
                                    HI_U64 *pu64PhyAddr, HI_VOID **ppVirAddr, HI_UL ulLen);
extern HI_S32 CMPI_MmzMallocCached(HI_CHAR *cpMmzName, HI_CHAR *pBufName,
                                   HI_U64 *pu64PhyAddr, HI_VOID **ppVirAddr, HI_UL ulLen);

/******************************************************************************
** The following functions can be used for physical memory which is outside OS.
** CMPI_Remap_Cached
** CMPI_Remap_Nocache
** CMPI_Unmap
******************************************************************************/
HI_VOID *CMPI_Remap_Cached(HI_U64 u64PhyAddr, HI_UL ulSize);
HI_VOID *CMPI_Remap_Nocache(HI_U64 u64PhyAddr, HI_UL ulSize);
HI_VOID CMPI_Unmap(HI_VOID *pVirtAddr);

/******************************************************************************
** Attention: Only the memory is malloced by CMPI_Kmalloc can use it!!
** CMPI_Virt2Phys
** CMPI_Phys2Virt
******************************************************************************/

static inline HI_S32 CMPI_CheckMmzPhyAddr(HI_U64 u64PhyAddr, HI_U64 u64Len)
{
    /* if address in mmz of current system */
    if (hil_is_phys_in_mmz(u64PhyAddr, u64Len)) {
        /* if in other system */
        if (hil_map_mmz_check_phys(u64PhyAddr, u64Len)) {
            return HI_FAILURE;
        }
    }

    return HI_SUCCESS;
}

static inline HI_BOOL read_user_linear_space_valid(HI_U8 *pu8AddrStart, HI_U32 u32Len)
{
    HI_U8 u8Check;
    HI_U8 *pu8AddrEnd = HI_NULL;

    if (u32Len == 0) {
        return HI_FALSE;
    }

    if (!osal_access_ok(OSAL_VERIFY_READ, pu8AddrStart, u32Len)) {
        return HI_FALSE;
    }

    pu8AddrEnd = pu8AddrStart + u32Len - 1;
    if (osal_copy_from_user(&u8Check, pu8AddrEnd, 1)) {
        return HI_FALSE;
    }

    return HI_TRUE;
}

static inline HI_BOOL write_user_linear_space_valid(HI_U8 *pu8AddrStart, HI_U32 u32Len)
{
    HI_U8 u8Check = 0;
    HI_U8 *pu8AddrEnd = HI_NULL;

    if (u32Len == 0) {
        return HI_FALSE;
    }

    if (!osal_access_ok(OSAL_VERIFY_WRITE, pu8AddrStart, u32Len)) {
        return HI_FALSE;
    }

    pu8AddrEnd = pu8AddrStart + u32Len - 1;
    if (osal_copy_to_user(pu8AddrEnd, &u8Check, 1)) {
        return HI_FALSE;
    }

    return HI_TRUE;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
