#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <limits.h>
#include <libgen.h>
#include <errno.h>

#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "lcd_mw_adapt.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define DEFAULT_MD_LEN (128)

typedef struct tagMMAP_NODE_S
{
    HI_U64 u64Start_P;
    HI_U64 u64Start_V;
    HI_U32 u32Length;
    HI_U32 u32RefCount;
    struct tagMMAP_NODE_S* pstNext;
} MMAP_NODE_S;

/**static init mutex */
static pthread_mutex_t s_MmapMutex = PTHREAD_MUTEX_INITIALIZER;
static MMAP_NODE_S* s_pstMMAPNode = NULL;

/* no need considering page_size of 4K */
HI_VOID* HI_MemMap(HI_U64 u64PhyAddr, HI_U32 u32Size)
{
#define PAGE_SIZE 0x1000
#define PAGE_SIZE_MASK 0xfffffffffffff000UL

    HI_U64 phy_addr_in_page;
    HI_U64 page_diff;
    HI_U64 size_in_page;
    MMAP_NODE_S* pTmp;
    MMAP_NODE_S* pNew;
    HI_VOID* addr = NULL;

    if (0 == u64PhyAddr || 0 == u32Size)
    {
        printf("memmap():size can't be zero!\n");
        return NULL;
    }

    /* check if the physical memory space have been mmaped */
    pTmp = s_pstMMAPNode;
    HI_MUTEX_LOCK(s_MmapMutex);

    while (pTmp != NULL)
    {
        if ( (u64PhyAddr >= pTmp->u64Start_P) &&
             ( (u64PhyAddr + u32Size) <= (pTmp->u64Start_P + pTmp->u32Length) ) )
        {
            pTmp->u32RefCount++;
            HI_MUTEX_UNLOCK(s_MmapMutex);
            return (HI_VOID*)(HI_UL)(pTmp->u64Start_V + u64PhyAddr - pTmp->u64Start_P);
        }

        pTmp = pTmp->pstNext;
    }

    HI_MUTEX_UNLOCK(s_MmapMutex);
    static HI_S32 fd = -1;

    /* not mmaped yet */
    if (fd < 0)
    {
#if __WORDSIZE==64
        const char dev[] = "/dev/mmz_userdev";
#else
        const char dev[] = "/dev/mem";
#endif
        /* dev not opened yet, so open it */
        fd = open (dev, O_RDWR | O_SYNC);

        if (fd < 0)
        {
            printf("memmap():open %s error!\n", dev);
            return NULL;
        }
    }

    /* addr align in page_size(4K) */
    phy_addr_in_page = u64PhyAddr & PAGE_SIZE_MASK;
    page_diff = u64PhyAddr - phy_addr_in_page;
    /* size in page_size */
    size_in_page = ((u32Size + page_diff - 1) & PAGE_SIZE_MASK) + PAGE_SIZE;
#if __WORDSIZE==64
    addr = mmap ((HI_VOID*)0, size_in_page, PROT_READ | PROT_WRITE, MAP_SHARED, fd, phy_addr_in_page);
#else
    addr = mmap64 ((HI_VOID*)0, size_in_page, PROT_READ | PROT_WRITE, MAP_SHARED, fd, phy_addr_in_page);
#endif

    if (addr == MAP_FAILED)
    {
        printf("memmap():mmap @ 0x%llx error!%s\n", phy_addr_in_page, strerror(errno));
        return NULL;
    }

    /* add this mmap to MMAP Node */
    pNew = (MMAP_NODE_S*)malloc(sizeof(MMAP_NODE_S));

    if (NULL == pNew)
    {
        munmap(addr, size_in_page);
        printf("memmap():malloc new node failed!\n");
        return NULL;
    }

    HI_MUTEX_LOCK(s_MmapMutex);
    pNew->u64Start_P = phy_addr_in_page;
    pNew->u64Start_V = (HI_UL)addr;
    pNew->u32Length = size_in_page;
    pNew->u32RefCount = 1;
    pNew->pstNext = NULL;

    if (s_pstMMAPNode == NULL)
    {
        s_pstMMAPNode = pNew;
    }
    else
    {
        pTmp = s_pstMMAPNode;

        while (pTmp->pstNext != NULL)
        {
            pTmp = pTmp->pstNext;
        }

        pTmp->pstNext = pNew;
    }

    HI_MUTEX_UNLOCK(s_MmapMutex);
    return (HI_VOID*)((HI_CHAR*)addr + page_diff);
}

HI_S32 HI_MemUnmap(HI_VOID* pvMappedAddr)
{
    MMAP_NODE_S* pPre;
    MMAP_NODE_S* pTmp;

    if (s_pstMMAPNode == NULL)
    {
        printf("memunmap(): address have not been mmaped!\n");
        return HI_FAILURE;
    }

    /* check if the physical memory space have been mmaped */
    pTmp = s_pstMMAPNode;
    pPre = s_pstMMAPNode;
    HI_MUTEX_LOCK(s_MmapMutex);

    do
    {
        if ( ((HI_UL)pvMappedAddr >= pTmp->u64Start_V) &&
             ((HI_UL)pvMappedAddr <= (pTmp->u64Start_V + pTmp->u32Length)) )
        {
            pTmp->u32RefCount--;

            if (0 == pTmp->u32RefCount)
            {

                /* delete this map node from pMMAPNode */
                if (pTmp == s_pstMMAPNode)
                {
                    s_pstMMAPNode = pTmp->pstNext;
                }
                else
                {
                    pPre->pstNext = pTmp->pstNext;
                }

                /* munmap */
                if (munmap((HI_VOID*)(HI_UL)pTmp->u64Start_V, pTmp->u32Length) != 0 )
                {
                    printf("memunmap(): munmap failed!\n");
                }

                HI_APPCOMM_SAFE_FREE(pTmp);
            }

            HI_MUTEX_UNLOCK(s_MmapMutex);
            return HI_SUCCESS;
        }

        pPre = pTmp;
        pTmp = pTmp->pstNext;
    }
    while (pTmp != NULL);

    HI_MUTEX_UNLOCK(s_MmapMutex);
    printf("memunmap(): address have not been mmaped!\n");
    return HI_FAILURE;
}


static HI_S32 himm(HI_U32 u32Addr, HI_U32 u32Value)
{
    HI_VOID* pmem = HI_MemMap(u32Addr, DEFAULT_MD_LEN);

    if (pmem == NULL)
    {
        return HI_FAILURE;
    }

    *(HI_U32*)pmem = u32Value;
    return HI_MemUnmap(pmem);
}

static HI_S32 himd(HI_U32 u32Addr, HI_U32* pu32Value)
{
    HI_VOID* pmem = HI_MemMap(u32Addr, DEFAULT_MD_LEN);

    if (pmem == NULL)
    {
        return HI_FAILURE;
    }

    *pu32Value = *(HI_U32*)pmem ;
    return HI_MemUnmap(pmem);
}

static void system_clk_cfg(void)
{
    himm(0x120101B8, 0x0007f81f);
    himm(0x120100F0, 0x00000861);
    himm(0x120100F8, 0x0000000f);
    himm(0x120100A0, 0x00182001);
    himm(0x120100F4, 0x24903ff);
    himm(0x12010100, 0x00f00492);
    himm(0x120100FC, 0x10);
    himm(0x12030018, 0x540);
    himm(0x12030034, 0x1);
    himm(0x12030000, 0x0);
    himm(0x12010018, 0x12000000);
    himm(0x1201001C, 0x1002063);
}

static void spi3_pin_mux(void)
{
    himm(0x112F0020, 0x4f1);
    himm(0x112F0024, 0x4f1);
    himm(0x112F002c, 0x14f1);
    himm(0x112F0028, 0x4f1);
}

static void vo_lcd8bit_mode(void)
{
    himm(0x112F0068 , 0x4f4); //LCD_CLK B19
    himm(0x112F0084 , 0x454); //LCD_DE A15
    himm(0x112F007c , 0x474); //LCD_HSYNC B16
    himm(0x112F0088 , 0x674); //LCD_VSYNC B15
    himm(0x112F0080 , 0x674); //LCD_DATA0 C16
    himm(0x112F0074 , 0x474); //LCD_DATA1 A17
    himm(0x112F0078 , 0x474); //LCD_DATA2 B17
    himm(0x112F006c , 0x474); //LCD_DATA3 B18
    himm(0x112F0070 , 0x474); //LCD_DATA4 C18
    himm(0x112F0064 , 0x674); //LCD_DATA5 A19
}

HI_S32 MW_SCREEN_ST7789_Init(HI_VOID)
{
    system_clk_cfg();
    spi3_pin_mux();
    vo_lcd8bit_mode();
}

HI_S32 MW_TOUCHPAD_FT6236_Init(HI_VOID)
{
    himm(0x112F008C , 0x5f1);
    himm(0x112F0090 , 0x5f1);
}

HI_S32 MW_SCREEN_VDP_CFG(HI_VOID)
{
#if 0
    HI_U32 u32Vaule = 0;
    HI_U32 u32SetValue0 = 0x6 << 18;
    HI_U32 mask0 = 0x7 << 18;

    HI_U32 u32SetValue1 = 0x227a65;
    HI_U32 mask1 = 0x07ffffff;

    HI_U32 u32SetValue2 = 0x2 << 14;
    HI_U32 mask2 = 0x3 << 14;

    himd(0x04510124, &u32Vaule);
    u32Vaule &= ~(mask0);
    u32Vaule |= (u32SetValue0 & mask0);

    u32Vaule &= ~(mask2);
    u32Vaule |= (u32SetValue2 & mask2);
    himm(0x04510124, u32Vaule);

    himd(0x04510120, &u32Vaule);
    u32Vaule &= ~(mask1);
    u32Vaule |= (u32SetValue1 & mask1);
    himm(0x04510120, u32Vaule);
#endif
    return 0;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
