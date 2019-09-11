#include "hisoc/mmu_config.h"

extern void *memset(void *dst, int c, size_t n);
extern void v8_dma_clean_range(AARCHPTR start, AARCHPTR end);

SENCOND_PAGE stOsPage = {0};
SENCOND_PAGE stAppPage = {0};

__attribute__((aligned(MMU_4K))) __attribute__((section(".bss.prebss.translation_table"))) UINT8 first_page_table[MMU_4K * 9];
__attribute__((aligned(MMU_4K))) UINT8 second_page_table_os[MMU_4K * 8];
__attribute__((aligned(MMU_4K))) UINT8 second_page_table_app[MMU_1M * 8];

static inline void creat_pgd_item(UINT64 tbl, UINT64 pmd_table_addr)
{
    *(UINT64 *)(tbl) = (pmd_table_addr) | MMU_PTE_L012_DESCRIPTOR_TABLE; /*lint !e511*/
}

/*lint -e648*/
static void creat_pmd_table(UINT64 pte_table_addr, UINT64 start_addr, UINT64 end_addr)
{
    UINT64 tbl =PMD_PAGE_GET(start_addr);
    UINT64 index_start = start_addr >> SHIFT_2M;
    UINT64 index_end = end_addr >> SHIFT_2M;
    UINT64 index;
    index_start &= ITEM_MASK;
    index_end &= ITEM_MASK;
    index_end += ((end_addr - start_addr) >> SHIFT_1G) << 9;

    for (index = index_start; index <= index_end; ++index)
    {
        *(UINT64 *)(tbl + index * 8) = pte_table_addr | MMU_PTE_L012_DESCRIPTOR_TABLE; /*lint !e511*/
        pte_table_addr += PAGE_SIZE;
    }
    v8_dma_clean_range((tbl + index_start * 8), (tbl + index_end * 8));
    asm volatile("tlbi    vmalle1");
}

static void creat_pte_table(MMU_PARAM *mPara)
{
    UINT64 index_max;
    UINT64 index_temp;
    UINT64 phy = (mPara->startAddr >> SHIFT_4K) << SHIFT_4K;
    UINT64 ulOffset = (mPara->startAddr - mPara->stPage->page_addr) >> SHIFT_4K;
    UINT64 tbl = mPara->stPage->page_descriptor_addr;
    if (mPara->endAddr <= mPara->startAddr)
        return;
    index_max = (mPara->endAddr -1 - mPara->startAddr) >> SHIFT_4K;
    phy |= mPara->uwFlag;
    for (index_temp = 0; index_temp <= index_max; ++index_temp, ++ulOffset)
    {
        *(UINT64 *)(tbl + ulOffset * 8) = (phy) | MMU_PTE_L3_DESCRIPTOR_PAGE; /*lint !e511*/
        phy += PAGE_SIZE;
    }
    v8_dma_clean_range(tbl + (ulOffset - index_temp) * 8 , tbl + ulOffset * 8);
    asm volatile("tlbi    vmalle1");
}

static void creat_block_map(UINT64 tbl, UINT64 flags, UINT64 phys,
                      UINT64 start, UINT64 end, UINT64 block_shift)
{
    UINT64 offset_nG = (end - start) >> SHIFT_1G;
    phys = phys >> block_shift;
    start = start >> block_shift;
    start &= ITEM_MASK;
    phys = flags | (phys << block_shift);
    end = end >> block_shift;
    end &=ITEM_MASK;
    end += offset_nG << 9;
    while (start <= end)
    {
        *(UINT64 *)(tbl + start*8) = phys; /*lint !e511*/
        start++;
        phys += (1U << block_shift); /*lint !e647*/
    }
}

static void creat_block_maps(UINT64 flags, UINT64 start, UINT64 end)
{
    UINT64 uStartOffset;
    UINT64 uEndOffset;
    UINT64 uPageSize;
    if ((start & (MMU_1G - 1)) != 0)
    {
        PRINT_ERR("%s,%d, the start of mapping addr is 0x%lx, should be aligned as 1G \n",__FUNCTION__,__LINE__, start);
        return;
    }
    end = ((end + (MMU_1G - 1)) &~(MMU_1G - 1));
    uPageSize = (sizeof(first_page_table) >> SHIFT_4K) - 1;
    uStartOffset = start >> SHIFT_1G;
    uEndOffset = end >> SHIFT_1G;
    if (uEndOffset > uPageSize)
    {
        PRINT_ERR("%s,%d, the end of mapping addr is 0x%lx, should not be bigger than 0x%lx \n",__FUNCTION__,__LINE__, end, (uPageSize << SHIFT_1G));
        return;
    }
    for (; uStartOffset < uEndOffset; ++uStartOffset)
    {
        creat_pgd_item(PGD_ADDR + uStartOffset * 8, PMD_ADDR0 + uStartOffset * PAGE_SIZE);
        creat_block_map((PMD_ADDR0 + uStartOffset * PAGE_SIZE), flags, (uStartOffset << SHIFT_1G),
                                    (uStartOffset << SHIFT_1G), ((uStartOffset + 1) << SHIFT_1G) - 1, SHIFT_2M);
    }
}

static void set_block_maps(UINT64 flags, UINT64 start, UINT64 end)
{
    UINT64 uStartOffset;

    if ((start & ((1u << SHIFT_2M) - 1)) != 0)
    {
        PRINT_ERR("%s,%d, the start of mapping addr is 0x%lx, should be aligned as 2M \n",__FUNCTION__,__LINE__, start);
        return;
    }

    uStartOffset = start >> SHIFT_1G;

    creat_block_map((PMD_ADDR0 + uStartOffset * PAGE_SIZE),
                            flags, start, start, end - 1, SHIFT_2M);
}

static inline int osMMUFlagCheck(UINT64 uwFlag)
{
    switch (uwFlag)
    {
        case MMU_PTE_CACHE_RO_FLAGS:
        case MMU_PTE_CACHE_RW_FLAGS:
        case MMU_PTE_NONCACHE_RO_FLAGS:
        case MMU_PTE_NONCACHE_RW_FLAGS:
        case MMU_INITIAL_MAP_STRONGLY_ORDERED:
        case MMU_INITIAL_MAP_DEVICE: break;
        default:
            PRINT_ERR("illegal mmu flag 0x%lx\n", uwFlag);
            return -1;
    }
    return 0;
}

void LOS_MMUParamSet(MMU_PARAM *mPara) /*lint -e529*/
{
    UINT64 pmd_start, pmd_end, pmd_tmp;

    if (mPara == NULL)
    {
        PRINT_ERR("input is null\n");
        return;
    }

    if (osMMUFlagCheck(mPara->uwFlag) == -1)
        return;

    if (mPara->uwArea == PMD_AREA)
    {
        pmd_start = PMD_ADDR_GET(mPara->startAddr);
        pmd_end = PMD_ADDR_GET(mPara->endAddr);
        for (pmd_tmp = pmd_start; pmd_tmp < pmd_end; pmd_tmp += 8)
        {
            if ((*(UINT64 *)pmd_tmp & 0x3) != MMU_PTE_L012_DESCRIPTOR_BLOCK) /*lint !e511*/
            {
                PRINT_ERR("not all mem belongs to pmd section(2M every item), descriptor types:0x%lx\n", (*(UINT64 *)pmd_tmp & 0x3) );/*lint !e511*/
                return;
            }
            else
            {
                PRINT_DEBUG("pmd_tmp = 0x%lx : 0x%lx\n", pmd_tmp, *(UINT64 *)pmd_tmp); /*lint !e511*/
            }
        }
        set_block_maps(mPara->uwFlag | MMU_PTE_L012_DESCRIPTOR_BLOCK,  mPara->startAddr, mPara->endAddr);
        PRINT_DEBUG("pmd_start = 0x%lx : 0x%lx\n", pmd_start, *(UINT64 *)pmd_start); /*lint !e511*/
        v8_dma_clean_range(pmd_start , pmd_end);
        asm volatile("tlbi    vmalle1");
        return;
    }

    if ((mPara->startAddr & (MMU_4K - 1)) != 0)
    {

        PRINT_ERR("mPara->startAddr[0x%lx] not aligned as 4K\n", mPara->startAddr);
        return;
    }

    if ((mPara->endAddr & (MMU_4K - 1)) != 0)
    {
        mPara->endAddr = (mPara->endAddr + (MMU_4K -1)) & ~ (MMU_4K - 1);
    }

    if (mPara->startAddr >= mPara->endAddr)
    {
        PRINT_ERR("wrong addr input mPara->startAddr[0x%lx] mPara->endAddr[0x%lx]\n", mPara->startAddr, mPara->endAddr);
        return;
    }

    if (mPara->startAddr < mPara->stPage->page_addr || mPara->endAddr > (mPara->stPage->page_length + mPara->stPage->page_addr))
    {
        PRINT_ERR("addr input not belongs to this second page \n");
        PRINT_ERR("mPara->startAddr:0x%lx, mPara->stPage->page_addr:0x%lx\n", mPara->startAddr,mPara->stPage->page_addr);
        PRINT_ERR("mPara->endAddr:0x%lx, (mPara->stPage->page_length + mPara->stPage->page_addr):0x%lx\n", mPara->endAddr, (mPara->stPage->page_length + mPara->stPage->page_addr));
        return;
    }

    creat_pte_table(mPara);
} /*lint +e529*/

void LOS_SecPageEnable(SENCOND_PAGE *stPage, UINT64 uwFlag)
{
    MMU_PARAM mPara;

    if (stPage == NULL)
    {
        PRINT_ERR("second page table(stPage) can't be NULL\n");
        return;
    }

    if (osMMUFlagCheck(uwFlag) == -1)
        return;

    mPara.startAddr = stPage->page_addr;
    mPara.endAddr = stPage->page_addr + stPage->page_length;
    mPara.uwFlag = uwFlag;
    mPara.stPage = stPage;

    creat_pte_table(&mPara);
    creat_pmd_table(stPage->page_descriptor_addr, stPage->page_addr, stPage->page_addr + stPage->page_length - 1);
}

void osSecPageInit(void)
{
    extern unsigned long __ram_data_start;
    stOsPage.page_addr = SYS_MEM_BASE + TEXT_OFFSET;
    stOsPage.page_length = ((((unsigned long)&__ram_data_start - stOsPage.page_addr) + MMU_2M - 1)& ~ (MMU_2M - 1));
    stOsPage.page_descriptor_addr = (AARCHPTR)second_page_table_os; /*lint !e64*/
    if(stOsPage.page_length > sizeof(second_page_table_os) << 9) {
        PRINT_ERR("the mapping size of os second page is 0x%x, sholud be not bigger than 0x%x\n", stOsPage.page_length,sizeof(second_page_table_os) << 9);
        return;
    }
    LOS_SecPageEnable(&stOsPage, MMU_PTE_CACHE_RW_FLAGS);

    stAppPage.page_addr = MMZ_MEM_BASE; /*lint !e569 !e435*/
    stAppPage.page_length = MMZ_MEM_LEN;
    stAppPage.page_descriptor_addr = (AARCHPTR)second_page_table_app; /*lint !e572*/
    if(stAppPage.page_length > sizeof(second_page_table_app) << 9) {
        PRINT_ERR("the mapping size of app second page is 0x%x, sholud be not bigger than 0x%x\n", stAppPage.page_length,sizeof(second_page_table_app) << 9);
        return;
    }
    LOS_SecPageEnable(&stAppPage, MMU_PTE_NONCACHE_RW_FLAGS);
}

VOID osRemapCached(unsigned long phys_addr, unsigned long size)
{
    if(phys_addr < MMZ_MEM_BASE)
        return;
    MMU_PARAM mPara;
    mPara.startAddr = phys_addr;
    mPara.endAddr = phys_addr + size;
    mPara.uwFlag = MMU_PTE_CACHE_RW_FLAGS;
    mPara.stPage = (SENCOND_PAGE *)&stAppPage;
    mPara.uwArea = PTE_AREA;
    LOS_MMUParamSet(&mPara);
}

VOID osRemapNoCached(unsigned long phys_addr, unsigned long size)
{
    if(phys_addr < MMZ_MEM_BASE)
        return;
    MMU_PARAM mPara;
    mPara.startAddr = phys_addr;
    mPara.endAddr = phys_addr + size;
    mPara.uwFlag = MMU_PTE_NONCACHE_RW_FLAGS;
    mPara.stPage = (SENCOND_PAGE *)&stAppPage;
    mPara.uwArea = PTE_AREA;
    LOS_MMUParamSet(&mPara);
}

void code_protect(void)
{
    extern unsigned long __text_start, __ram_data_start;
    MMU_PARAM mPara;

    mPara.startAddr = (unsigned long)&__text_start;
    mPara.endAddr = (unsigned long)&__ram_data_start;
    mPara.uwFlag = MMU_PTE_CACHE_RO_FLAGS;
    mPara.stPage = (SENCOND_PAGE *)&stOsPage;
    mPara.uwArea = PTE_AREA;
    PRINTK("mPara.startAddr = 0x%lx mPara.endAddr = 0x%lx\n", mPara.startAddr, mPara.endAddr);
    LOS_MMUParamSet(&mPara);
}

/*
*   device:
*       access without cache;
*       not support out-of-order execution;
*       not support Instruction pre read;
*       not support non-aligned access
*   normal:
*       access cacheability(cache or non-cache);
*       support out-of-order execution;
*       support Instruction pre read;
*       support non-aligned access
**/
void mmu_init(void)
{
    UINT64 uFlag;
    (void)memset((void *)PGD_ADDR, 0, MMU_4K * 9); /*lint !e419 !e511*/
    //device
    uFlag = (MMU_INITIAL_MAP_DEVICE | MMU_PTE_L012_DESCRIPTOR_BLOCK);
    creat_block_maps(uFlag, 0, DDR_MEM_ADDR + DDR_MEM_SIZE);

    //normal cache
    uFlag = (MMU_PTE_CACHE_RW_FLAGS | MMU_PTE_L012_DESCRIPTOR_BLOCK);
    set_block_maps(uFlag, SYS_MEM_BASE, SYS_MEM_BASE + TEXT_OFFSET + SYS_MEM_SIZE_DEFAULT - 1);

    //normal non-cache
    uFlag = (MMU_PTE_NONCACHE_RW_FLAGS | MMU_PTE_L012_DESCRIPTOR_BLOCK);
    set_block_maps(uFlag,  DDR_MEM_ADDR, SYS_MEM_BASE);
    set_block_maps(uFlag,  SYS_MEM_BASE + TEXT_OFFSET + SYS_MEM_SIZE_DEFAULT, DDR_MEM_ADDR + DDR_MEM_SIZE - 1);
}
/*lint +e648*/
