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

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

extern HI_U64  CMPI_MmzMalloc(HI_CHAR *pMmzName, HI_CHAR *pBufName, HI_UL ulSize);
extern HI_VOID CMPI_MmzFree(HI_U64 u64PhyAddr, HI_VOID* pVirAddr);
extern HI_S32  CMPI_MmzMallocNocache(HI_CHAR* cpMmzName, HI_CHAR* pBufName,
                             HI_U64* pu64PhyAddr, HI_VOID** ppVirAddr, HI_UL ulLen);
extern HI_S32  CMPI_MmzMallocCached(HI_CHAR* cpMmzName, HI_CHAR* pBufName,
                            HI_U64* pu64PhyAddr, HI_VOID** ppVirAddr, HI_UL ulLen);


/******************************************************************************
** The following functions can be used for physical memory which is outside OS.
** CMPI_Remap_Cached 
** CMPI_Remap_Nocache
** CMPI_Unmap
******************************************************************************/


__inline static HI_VOID * CMPI_Remap_Cached(HI_U64 u64PhyAddr, HI_UL ulSize)
{
	return osal_ioremap_cached(u64PhyAddr, ALIGN_UP(ulSize, 4));
}

__inline static HI_VOID * CMPI_Remap_Nocache(HI_U64 u64PhyAddr, HI_UL ulSize)
{
    return osal_ioremap_nocache(u64PhyAddr, ALIGN_UP(ulSize, 4));
}

__inline static HI_VOID   CMPI_Unmap(HI_VOID *pVirtAddr)
{
    osal_iounmap(pVirtAddr);
}


/******************************************************************************
** Attention: Only the memory is malloced by CMPI_Kmalloc can use it!!
** CMPI_Virt2Phys
** CMPI_Phys2Virt
******************************************************************************/
/*
__inline static HI_U32 CMPI_Virt2Phys(HI_VOID *pVirAddr)
{
    return virt_to_phys(pVirAddr);
}

__inline static HI_VOID * CMPI_Phys2Virt(HI_U32 u32PhyAddr)
{
    return phys_to_virt(u32PhyAddr);
}
*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef  __MM_EXT_H__ */
