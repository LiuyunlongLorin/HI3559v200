/******************************************************************************

  Copyright (C), 2016, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : isp_vreg.c
  Version       : Initial Draft
  Author        : Hisilicon multimedia software group
  Created       : 2013/01/09
  Description   :
  History       :
  1.Date        : 2013/01/09
    Author      :
    Modification: Created file

******************************************************************************/
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include "mpi_isp.h"
#include "mpi_sys.h"

#include "isp_vreg.h"
#include "hi_comm_isp.h"
#include "isp_drv_vreg.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

typedef struct hiVREG_ARGS_S {
    HI_U64  u64Size;
    HI_U64  u64BaseAddr;
    HI_U64  u64PhyAddr;
    HI_U64  u64VirtAddr;
} VREG_ARGS_S;

typedef struct hiHI_VREG_ADDR_S {
    HI_U64  u64PhyAddr;
    HI_VOID  *pVirtAddr;
} HI_VREG_ADDR_S;

typedef struct hiHI_VREG_S {
    HI_VREG_ADDR_S stSlaveTimeAddr;
    HI_VREG_ADDR_S stSlaveRegAddr[CAP_SLAVE_MAX_NUM];
    HI_VREG_ADDR_S stIspFeRegAddr[ISP_MAX_PIPE_NUM];
    HI_VREG_ADDR_S stIspBeRegAddr[ISP_MAX_BE_NUM];
    HI_VREG_ADDR_S stIspHdrRegAddr[ISP_MAX_BE_NUM];
    HI_VREG_ADDR_S stViprocRegAddr[ISP_MAX_BE_NUM];
    HI_VREG_ADDR_S stIspVRegAddr[ISP_MAX_PIPE_NUM];
    HI_VREG_ADDR_S astAeVRegAddr[MAX_ALG_LIB_VREG_NUM];
    HI_VREG_ADDR_S astAwbVRegAddr[MAX_ALG_LIB_VREG_NUM];
    HI_VREG_ADDR_S astAfVRegAddr[MAX_ALG_LIB_VREG_NUM];
} HI_VREG_S;

static HI_VREG_S g_stHiVreg = {{ 0 }};

HI_S32 g_s32VregFd[ISP_MAX_PIPE_NUM] = {[0 ... (ISP_MAX_PIPE_NUM - 1)] = -1};

static inline HI_S32 VREG_CHECK_OPEN(VI_PIPE ViPipe)
{
    if (g_s32VregFd[ViPipe] <= 0) {
        g_s32VregFd[ViPipe] = open("/dev/isp_dev", O_RDONLY, S_IRUSR);
        if (g_s32VregFd[ViPipe] < 0) {
            perror("open isp device error!\n");
            return HI_FAILURE;
        }
        if (ioctl(g_s32VregFd[ViPipe], VREG_DRV_FD, &ViPipe)) {
            close(g_s32VregFd[ViPipe]);
            g_s32VregFd[ViPipe] = -1;
            return HI_FAILURE;
        }
    }
    return HI_SUCCESS;
}

static inline HI_S32 VREG_CHECK0_OPEN(HI_VOID)
{
    if (g_s32VregFd[0] <= 0) {
        g_s32VregFd[0] = open("/dev/isp_dev", O_RDONLY, S_IRUSR);
        if (g_s32VregFd[0] < 0) {
            perror("open isp device error!\n");
            return HI_FAILURE;
        }
    }
    return HI_SUCCESS;
}

static HI_S32 s_s32MemDev = -1;
#define ISP_MEM_DEV_OPEN if (s_s32MemDev <= 0)\
    {\
        s_s32MemDev = open("/dev/mem", O_RDWR|O_SYNC,S_IRUSR | S_IRUSR);\
        if (s_s32MemDev < 0)\
        {\
            perror("Open dev/mem error");\
            return NULL;\
        }\
    }\

static HI_VOID *VReg_IOMmap(HI_U64 u64PhyAddr, HI_U32 u32Size)
{
    HI_U32 u32Diff;
    HI_U64 u64PagePhy;
    HI_U8 *pPageAddr;
    HI_UL  ulPageSize;

    ISP_MEM_DEV_OPEN;

    /**********************************************************
     PageSize will be 0 when u32size is 0 and u32Diff is 0,
     and then mmap will be error (error: Invalid argument)
     ***********************************************************/
    if (!u32Size) {
        ISP_TRACE(HI_DBG_ERR, "u32Size can't be 0.\n");
        return NULL;
    }

    /* The mmap address should align with page */
    u64PagePhy = u64PhyAddr & 0xfffffffffffff000ULL;
    u32Diff    = u64PhyAddr - u64PagePhy;

    /* The mmap size shuld be mutliples of 1024 */
    ulPageSize = ((u32Size + u32Diff - 1) & 0xfffff000UL) + 0x1000;

    pPageAddr = mmap ((void *)0, ulPageSize, PROT_READ | PROT_WRITE,
                      MAP_SHARED, s_s32MemDev, u64PagePhy);

    if (pPageAddr == MAP_FAILED) {
        perror("mmap error");
        return NULL;
    }

    return (HI_VOID *)(pPageAddr + u32Diff);
}

static inline HI_BOOL VREG_CHECK_SLAVE_BASE(HI_U32 u32BaseAddr) {
    return (HI_BOOL)ISP_CHECK_REG_BASE(u32BaseAddr, SLAVE_REG_BASE, CAP_SLAVE_REG_BASE(CAP_SLAVE_MAX_NUM));
}

static inline HI_BOOL VREG_CHECK_SLAVE_MODE_TIME_BASE(HI_U32 u32BaseAddr) {
    return (HI_BOOL)ISP_CHECK_REG_BASE(u32BaseAddr, SLAVE_MODE_REG_BASE, SLAVE_MODE_REG_BASE);
}

static inline HI_BOOL VREG_CHECK_ISPFE_BASE(HI_U32 u32BaseAddr) {
    return (HI_BOOL)ISP_CHECK_REG_BASE(GET_ISP_REG_BASE(u32BaseAddr), FE_REG_BASE, ISP_FE_REG_BASE(ISP_MAX_PIPE_NUM));
}

static inline HI_BOOL VREG_CHECK_ISPBE_BASE(HI_U32 u32BaseAddr) {
    return (HI_BOOL)(ISP_CHECK_REG_BASE(GET_ISP_REG_BASE(u32BaseAddr), BE_REG_BASE, ISP_BE_REG_BASE(ISP_MAX_BE_NUM)) &&
                     ((GET_ISP_REG_BASE(u32BaseAddr)) == (ISP_BE_REG_BASE(ISP_GET_BE_ID(u32BaseAddr)))));
}

static inline HI_BOOL VREG_CHECK_VIPROC_BASE(HI_U32 u32BaseAddr) {
    return (HI_BOOL)(ISP_CHECK_REG_BASE(GET_VIPROC_REG_BASE(u32BaseAddr), VIPROC_REG_BASE, ISP_VIPROC_REG_BASE(ISP_MAX_BE_NUM)) && \
                      ((GET_VIPROC_REG_BASE(u32BaseAddr)) == (ISP_VIPROC_REG_BASE(ISP_GET_VIPROC_ID(u32BaseAddr)))));
}

static inline HI_BOOL VREG_CHECK_ISP_VREG_BASE_EX(HI_U32 u32BaseAddr) {
    return (HI_BOOL)ISP_CHECK_REG_BASE_EX(GET_ISP_VREG_BASE(u32BaseAddr), ISP_VREG_BASE, ISP_VIR_REG_BASE(ISP_MAX_PIPE_NUM));
}

static inline HI_BOOL VREG_CHECK_ISP_VREG_BASE(HI_U32 u32BaseAddr) {
    return (HI_BOOL)ISP_CHECK_REG_BASE(GET_ISP_VREG_BASE(u32BaseAddr), ISP_VREG_BASE, ISP_VIR_REG_BASE(ISP_MAX_PIPE_NUM));
}

static inline HI_BOOL VREG_CHECK_AE_VREG_BASE_EX(HI_U32 u32BaseAddr) {
    return (HI_BOOL)ISP_CHECK_REG_BASE_EX(GET_3A_VREG_BASE(u32BaseAddr), AE_VREG_BASE, AE_LIB_VREG_BASE(ALG_LIB_MAX_NUM));
}

static inline HI_BOOL VREG_CHECK_AE_VREG_BASE(HI_U32 u32BaseAddr) {
    return (HI_BOOL)ISP_CHECK_REG_BASE(GET_3A_VREG_BASE(u32BaseAddr), AE_VREG_BASE, AE_LIB_VREG_BASE(ALG_LIB_MAX_NUM));
}

static inline HI_BOOL VREG_CHECK_AWB_VREG_BASE_EX(HI_U32 u32BaseAddr) {
    return (HI_BOOL)ISP_CHECK_REG_BASE_EX(GET_3A_VREG_BASE(u32BaseAddr), AWB_VREG_BASE, AWB_LIB_VREG_BASE(ALG_LIB_MAX_NUM));
}

static inline HI_BOOL VREG_CHECK_AWB_VREG_BASE(HI_U32 u32BaseAddr) {
    return (HI_BOOL)ISP_CHECK_REG_BASE(GET_3A_VREG_BASE(u32BaseAddr), AWB_VREG_BASE, AWB_LIB_VREG_BASE(ALG_LIB_MAX_NUM));
}

static inline HI_BOOL VREG_CHECK_AF_VREG_BASE(HI_U32 u32BaseAddr) {
    return (HI_BOOL)ISP_CHECK_REG_BASE(GET_3A_VREG_BASE(u32BaseAddr), AF_VREG_BASE, AF_LIB_VREG_BASE(ALG_LIB_MAX_NUM));
}

static inline HI_BOOL VREG_CHECK_3A_VREG_BASE(HI_U32 u32BaseAddr) {
    return (HI_BOOL)(VREG_CHECK_AE_VREG_BASE(GET_3A_VREG_BASE(u32BaseAddr)) || \
                     VREG_CHECK_AWB_VREG_BASE(GET_3A_VREG_BASE(u32BaseAddr)) || \
                     VREG_CHECK_AF_VREG_BASE(GET_3A_VREG_BASE(u32BaseAddr)));
}

#define VREG_MUNMAP_VIRTADDR(pVirtAddr, u32Size)\
    do{\
        if (HI_NULL != (pVirtAddr))\
        {\
            HI_MPI_SYS_Munmap((pVirtAddr), (u32Size));\
        }\
    }while(0);

static inline HI_VREG_ADDR_S *VReg_Match(HI_U32 u32BaseAddr)
{
    if (VREG_CHECK_SLAVE_MODE_TIME_BASE(u32BaseAddr)) {
        return &g_stHiVreg.stSlaveTimeAddr;
    }

    if (VREG_CHECK_SLAVE_BASE(u32BaseAddr)) {
        return &g_stHiVreg.stSlaveRegAddr[GET_SLAVE_ID(u32BaseAddr)];
    }

    if (VREG_CHECK_ISPFE_BASE(u32BaseAddr)) {
        return &g_stHiVreg.stIspFeRegAddr[ISP_GET_FE_ID(u32BaseAddr)];
    }

    if (VREG_CHECK_ISPBE_BASE(u32BaseAddr)) {
        return &g_stHiVreg.stIspBeRegAddr[ISP_GET_BE_ID(u32BaseAddr)];
    }

    if (VREG_CHECK_VIPROC_BASE(u32BaseAddr)) {
        return &g_stHiVreg.stViprocRegAddr[ISP_GET_VIPROC_ID(u32BaseAddr)];
    }

    if (VREG_CHECK_ISP_VREG_BASE(u32BaseAddr)) {
        return &g_stHiVreg.stIspVRegAddr[ISP_GET_VREG_ID(u32BaseAddr)];
    }

    if (VREG_CHECK_AE_VREG_BASE(u32BaseAddr)) {
        return &g_stHiVreg.astAeVRegAddr[ISP_GET_AE_ID(u32BaseAddr)];
    }

    if (VREG_CHECK_AWB_VREG_BASE(u32BaseAddr)) {
        return &g_stHiVreg.astAwbVRegAddr[ISP_GET_AWB_ID(u32BaseAddr)];
    }

    if (VREG_CHECK_AF_VREG_BASE(u32BaseAddr)) {
        return &g_stHiVreg.astAfVRegAddr[ISP_GET_AF_ID(u32BaseAddr)];
    }

    return HI_NULL;
}

static inline HI_U32 VReg_BaseAlign(HI_U32 u32BaseAddr)
{
    if (VREG_CHECK_AE_VREG_BASE(u32BaseAddr)) {
        return (u32BaseAddr & 0xFFFFE000);
    } else if (VREG_CHECK_AWB_VREG_BASE(u32BaseAddr) || VREG_CHECK_AF_VREG_BASE(u32BaseAddr)) {
        return (u32BaseAddr & 0xFFFFF000);
    } else {
        return (u32BaseAddr & 0xFFFE0000);
    }
}

static inline HI_U32 VReg_SizeAlign(HI_U32 u32Size)
{
    return (ALG_LIB_VREG_SIZE * ((u32Size + ALG_LIB_VREG_SIZE - 1) / ALG_LIB_VREG_SIZE));
}

HI_S32 VReg_Init(VI_PIPE ViPipe, HI_U32 u32BaseAddr, HI_U32 u32Size)
{
    VREG_ARGS_S stVreg = { 0 };

    ISP_CHECK_PIPE(ViPipe);

    if (VREG_CHECK_OPEN(ViPipe)) {
        return HI_FAILURE;
    }

    if (u32BaseAddr != VReg_BaseAlign(u32BaseAddr)) {
        return HI_FAILURE;
    }

    /* malloc vreg's phyaddr in kernel */
    stVreg.u64BaseAddr = (HI_U64)VReg_BaseAlign(u32BaseAddr);
    stVreg.u64Size = (HI_U64)VReg_SizeAlign(u32Size);
    if (ioctl(g_s32VregFd[ViPipe], VREG_DRV_INIT, &stVreg)) {
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 VReg_Exit(VI_PIPE ViPipe, HI_U32 u32BaseAddr, HI_U32 u32Size)
{
    HI_VREG_ADDR_S *pstVReg = HI_NULL;
    VREG_ARGS_S stVreg;

    ISP_CHECK_PIPE(ViPipe);

    if (VREG_CHECK_OPEN(ViPipe)) {
        return HI_FAILURE;
    }

    if (u32BaseAddr != VReg_BaseAlign(u32BaseAddr)) {
        return HI_FAILURE;
    }

    /* check base */
    pstVReg = VReg_Match(VReg_BaseAlign(u32BaseAddr));
    if (pstVReg == HI_NULL) {
        return HI_FAILURE;
    }

    if (pstVReg->pVirtAddr != HI_NULL) {
        /* munmap virtaddr */
        VREG_MUNMAP_VIRTADDR(pstVReg->pVirtAddr, VReg_SizeAlign(u32Size));
        pstVReg->pVirtAddr  = HI_NULL;
        pstVReg->u64PhyAddr = 0;
    }

    /* release the buf in the kernel */
    stVreg.u64BaseAddr = (HI_U64)VReg_BaseAlign(u32BaseAddr);
    if (ioctl(g_s32VregFd[ViPipe], VREG_DRV_EXIT, &stVreg)) {
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 VReg_ReleaseAll(VI_PIPE ViPipe)
{
    VREG_ARGS_S stVreg = { 0 };

    ISP_CHECK_PIPE(ViPipe);

    if (VREG_CHECK_OPEN(ViPipe)) {
        return HI_FAILURE;
    }

    stVreg.u64BaseAddr = ISP_VREG_BASE;
    stVreg.u64Size     = ISP_VREG_SIZE;

    /* release all buf in the kernel */
    if (ioctl(g_s32VregFd[ViPipe], VREG_DRV_RELEASE_ALL, &stVreg)) {
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_VOID *VReg_GetVirtAddrBase(HI_U32 u32BaseAddr)
{
    HI_U32 u32Base, u32Size;
    HI_VREG_ADDR_S *pstVReg = HI_NULL;
    VREG_ARGS_S stVreg;

    /* check base */
    pstVReg = VReg_Match(u32BaseAddr);
    if (pstVReg == HI_NULL) {
        return pstVReg;
    }

    if (pstVReg->pVirtAddr != HI_NULL) {
        return pstVReg->pVirtAddr;
    }

    /* get phyaddr first */
    if (VREG_CHECK_SLAVE_MODE_TIME_BASE(u32BaseAddr)) {
        pstVReg->u64PhyAddr = SLAVE_MODE_REG_BASE;
        u32Size = SLAVE_MODE_TIME_ALIGN;
        pstVReg->pVirtAddr = VReg_IOMmap(pstVReg->u64PhyAddr, u32Size);
    } else if (VREG_CHECK_SLAVE_BASE(u32BaseAddr)) {
        pstVReg->u64PhyAddr = CAP_SLAVE_REG_BASE(GET_SLAVE_ID(u32BaseAddr));
        u32Size = SLAVE_MODE_ALIGN;
        pstVReg->pVirtAddr = VReg_IOMmap(pstVReg->u64PhyAddr, u32Size);
    } else if (VREG_CHECK_ISPFE_BASE(u32BaseAddr)) {
        pstVReg->u64PhyAddr = ISP_FE_REG_BASE(ISP_GET_FE_ID(u32BaseAddr));
        u32Size = FE_REG_SIZE_ALIGN;
        pstVReg->pVirtAddr = VReg_IOMmap(pstVReg->u64PhyAddr, u32Size);
    } else if (VREG_CHECK_ISPBE_BASE(u32BaseAddr)) {
        pstVReg->u64PhyAddr = ISP_BE_REG_BASE(ISP_GET_BE_ID(u32BaseAddr));
        u32Size = BE_REG_SIZE_ALIGN;
        pstVReg->pVirtAddr = VReg_IOMmap(pstVReg->u64PhyAddr, u32Size);
    } else if (VREG_CHECK_VIPROC_BASE(u32BaseAddr)) {
        pstVReg->u64PhyAddr = ISP_VIPROC_REG_BASE(ISP_GET_VIPROC_ID(u32BaseAddr));
        u32Size = VIPROC_REG_SIZE;
        pstVReg->pVirtAddr = VReg_IOMmap(pstVReg->u64PhyAddr, u32Size);
    } else {
        if (VREG_CHECK0_OPEN()) {
            return HI_NULL;
        }

        if (VREG_CHECK_ISP_VREG_BASE(u32BaseAddr)) {
            u32Base = GET_ISP_VREG_BASE(u32BaseAddr);
            u32Size = ISP_VREG_SIZE;
        } else if (VREG_CHECK_AE_VREG_BASE(u32BaseAddr)) {
            u32Base = GET_AE_ID_VREG_BASE(u32BaseAddr);
            u32Size = AE_VREG_SIZE;
        }else if (VREG_CHECK_AWB_VREG_BASE(u32BaseAddr)) {
            u32Base = GET_AWB_ID_VREG_BASE(u32BaseAddr);
            u32Size = ALG_LIB_VREG_SIZE;
        } else {
            u32Base = GET_AF_ID_VREG_BASE(u32BaseAddr);
            u32Size = ALG_LIB_VREG_SIZE;
        }

        stVreg.u64BaseAddr = (HI_U64)u32Base;
        if (ioctl(g_s32VregFd[0], VREG_DRV_GETADDR, &stVreg)) {
            return HI_NULL;
        }
        pstVReg->u64PhyAddr = stVreg.u64PhyAddr;

        /* Mmap virtaddr */
        pstVReg->pVirtAddr = HI_MPI_SYS_Mmap(pstVReg->u64PhyAddr, u32Size);
    }

    return pstVReg->pVirtAddr;
}


HI_S32 VReg_Munmap(HI_U32 u32BaseAddr, HI_U32 u32Size)
{
    HI_VREG_ADDR_S *pstVReg = HI_NULL;

    if (u32BaseAddr != VReg_BaseAlign(u32BaseAddr)) {
        return HI_FAILURE;
    }

    /* check base */
    pstVReg = VReg_Match(VReg_BaseAlign(u32BaseAddr));
    if (pstVReg == HI_NULL) {
        return HI_FAILURE;
    }

    if (pstVReg->pVirtAddr != HI_NULL) {
        /* munmap virtaddr */
        VREG_MUNMAP_VIRTADDR(pstVReg->pVirtAddr, VReg_SizeAlign(u32Size));
        pstVReg->pVirtAddr = HI_NULL;
    }

    return HI_SUCCESS;
}

static HI_U32 Get_SlaveAddrOffset(HI_U32 u32BaseAddr)
{
    HI_U8 u8AddrId;

    u8AddrId = GET_SLAVE_ID(u32BaseAddr);

    switch (u8AddrId) {
        case 0:
            return ((u32BaseAddr - CAP_SLAVE_REG_BASE(u8AddrId)) & 0xFF);
        case 1:
            return ((u32BaseAddr - CAP_SLAVE_REG_BASE(u8AddrId)) & 0xFF);
        default:
            return 0;
    }
}

#define HIVREG_WRITE_REG32(b, addr) *(addr) = b
#define HIVREG_READ_REG32(addr) *(addr)

HI_VOID *VReg_GetVirtAddr(HI_U32 u32BaseAddr)
{
    HI_VOID *pVirtAddr = HI_NULL;

    pVirtAddr = VReg_GetVirtAddrBase(u32BaseAddr);
    if (pVirtAddr == HI_NULL) {
        return pVirtAddr;
    }

    if ((VREG_CHECK_ISPFE_BASE(u32BaseAddr)) ||
        (VREG_CHECK_ISPBE_BASE(u32BaseAddr))) {
        return ((HI_U8 *)pVirtAddr + (u32BaseAddr & 0x3FFFF));
    } else if ((VREG_CHECK_ISP_VREG_BASE(u32BaseAddr))) {
        return ((HI_U8 *)pVirtAddr + (u32BaseAddr & 0x1FFFF));
    } else if (VREG_CHECK_AE_VREG_BASE(u32BaseAddr)) {
        return ((HI_U8 *)pVirtAddr + (u32BaseAddr & 0x1FFF));
    } else if (VREG_CHECK_AWB_VREG_BASE(u32BaseAddr)){
        return ((HI_U8 *)pVirtAddr + (u32BaseAddr & 0xFFF));
    } else if (VREG_CHECK_AF_VREG_BASE(u32BaseAddr)){
        return ((HI_U8 *)pVirtAddr + (u32BaseAddr & 0xFFF));
    } else if (VREG_CHECK_SLAVE_MODE_TIME_BASE(u32BaseAddr)) {
        return ((HI_U8 *)pVirtAddr + (u32BaseAddr - SLAVE_MODE_REG_BASE));
    } else if (VREG_CHECK_SLAVE_BASE(u32BaseAddr)) {
        return ((HI_U8 *)pVirtAddr + Get_SlaveAddrOffset(u32BaseAddr));
    } else if (VREG_CHECK_VIPROC_BASE(u32BaseAddr)) {
        return ((HI_U8 *)pVirtAddr + (u32BaseAddr & 0xFFF));
    } else {
        return pVirtAddr;
    }
}

HI_VOID *VReg_GetVirtAddr_Ex(HI_U32 u32BaseAddr)
{
    HI_VOID *pVirtAddr = HI_NULL;

    pVirtAddr = VReg_GetVirtAddrBase(u32BaseAddr);
    if (pVirtAddr == HI_NULL) {
        return pVirtAddr;
    }

    if ((VREG_CHECK_ISPFE_BASE(u32BaseAddr)) ||
        (VREG_CHECK_ISPBE_BASE(u32BaseAddr))) {
        return HI_NULL;
    } else if ((VREG_CHECK_ISP_VREG_BASE_EX(u32BaseAddr))) {
        return ((HI_U8 *)pVirtAddr + (u32BaseAddr & 0x1FFFF));
    } else if ((VREG_CHECK_AE_VREG_BASE_EX(u32BaseAddr))) {
        return ((HI_U8 *)pVirtAddr + (u32BaseAddr & 0x1FFF));
    } else if((VREG_CHECK_AWB_VREG_BASE_EX(u32BaseAddr))) {
        return ((HI_U8 *)pVirtAddr + (u32BaseAddr & 0xFFF));
    } else if (VREG_CHECK_SLAVE_BASE(u32BaseAddr)) {
        return HI_NULL;
    } else if (VREG_CHECK_VIPROC_BASE(u32BaseAddr)){
        return HI_NULL;
    } else {
        return HI_NULL;
    }
}

HI_U32 IO_READ32(HI_U32 u32Addr)
{
    HI_VOID *pVirtAddr = HI_NULL;
    HI_U32  *pu32Addr, u32Value;

    pVirtAddr = VReg_GetVirtAddr(u32Addr);
    if (pVirtAddr == HI_NULL) {
        return 0;
    }

    pu32Addr = (HI_U32 *)((HI_UL)pVirtAddr & IO_MASK_BITXX);
    u32Value = HIVREG_READ_REG32(pu32Addr);

    return u32Value;
}

HI_S32 IO_READ32_Ex(HI_U32 u32Addr,HI_U32 *pu32Value)
{
    HI_VOID *pVirtAddr = HI_NULL;
    HI_U32  *pu32Addr, u32Value;
    ISP_CHECK_POINTER(pu32Value);
    pVirtAddr = VReg_GetVirtAddr(u32Addr);
    if (pVirtAddr == HI_NULL) {
        return HI_FAILURE;
    }
    pu32Addr = (HI_U32 *)((HI_UL)pVirtAddr & IO_MASK_BITXX);
    u32Value = HIVREG_READ_REG32(pu32Addr);
    *pu32Value = u32Value;
    return HI_SUCCESS;
}

HI_S32 IO_WRITE32(HI_U32 u32Addr, HI_U32 u32Value)
{
    HI_VOID *pVirtAddr = HI_NULL;
    HI_U32  *pu32Addr;

    pVirtAddr = VReg_GetVirtAddr(u32Addr);
    if (pVirtAddr == HI_NULL) {
        return 0;
    }

    pu32Addr = (HI_U32 *)((HI_UL)pVirtAddr & IO_MASK_BITXX);
    HIVREG_WRITE_REG32(u32Value, pu32Addr);

    return HI_SUCCESS;
}

HI_S32 IO_WRITE32_Ex(HI_U32 u32Addr, HI_U32 u32Value)
{
    HI_VOID *pVirtAddr = HI_NULL;
    HI_U32  *pu32Addr;

    pVirtAddr = VReg_GetVirtAddr_Ex(u32Addr);
    if (pVirtAddr == HI_NULL) {
        return HI_FAILURE;
    }

    pu32Addr = (HI_U32 *)((HI_UL)pVirtAddr & IO_MASK_BITXX);
    HIVREG_WRITE_REG32(u32Value, pu32Addr);

    return HI_SUCCESS;
}

HI_U8 IO_READ8(HI_U32 u32Addr)
{
    HI_U32 u32Value;

    u32Value = IO_READ32(u32Addr & IO_MASK_BIT32);

    return (u32Value >> GET_SHIFT_BIT(u32Addr)) & 0xFF;
}

HI_S32 IO_WRITE8(HI_U32 u32Addr, HI_U32 u32Value)
{
    HI_U32  u32Current;
    HI_U32  u32CurrentMask = 0;
    HI_U32  u32ValueTmp = 0;

    u32CurrentMask = ~(0xFF << GET_SHIFT_BIT(u32Addr));
    u32ValueTmp    = (u32Value & 0xFF) << GET_SHIFT_BIT(u32Addr);
    u32Current = IO_READ32(u32Addr & IO_MASK_BIT32);
    IO_WRITE32((u32Addr & IO_MASK_BIT32), u32ValueTmp | (u32Current & u32CurrentMask));

    return HI_SUCCESS;
}

HI_U16 IO_READ16(HI_U32 u32Addr)
{
    HI_U32  u32Value;

    u32Value = IO_READ32(u32Addr & IO_MASK_BIT32);

    return (u32Value >> GET_SHIFT_BIT(u32Addr & 0xFFFFFFFE)) & 0xFFFF;
}

HI_S32 IO_WRITE16(HI_U32 u32Addr, HI_U32 u32Value)
{
    HI_U32  u32Current;
    HI_U32  u32CurrentMask = 0;
    HI_U32  u32ValueTmp = 0;

    u32CurrentMask = ~(0xFFFF << GET_SHIFT_BIT(u32Addr & 0xFFFFFFFE));
    u32ValueTmp    = (u32Value & 0xFFFF) << GET_SHIFT_BIT(u32Addr & 0xFFFFFFFE);
    u32Current = IO_READ32(u32Addr & IO_MASK_BIT32);
    IO_WRITE32((u32Addr & IO_MASK_BIT32), u32ValueTmp | (u32Current & u32CurrentMask));

    return HI_SUCCESS;
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
