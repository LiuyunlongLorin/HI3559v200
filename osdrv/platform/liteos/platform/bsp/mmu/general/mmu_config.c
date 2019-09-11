/*----------------------------------------------------------------------------
 * Copyright (c) <2013-2015>, <Huawei Technologies Co., Ltd>
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific prior written
 * permission.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *---------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------
 * Notice of Export Control Law
 * ===============================================
 * Huawei LiteOS may be subject to applicable export control laws and regulations, which might
 * include those applicable to Huawei LiteOS of U.S. and the country in which you are located.
 * Import, export and usage of Huawei LiteOS in any manner by you shall be in compliance with such
 * applicable export control laws and regulations.
 *---------------------------------------------------------------------------*/

#include "hisoc/mmu_config.h"
#ifdef LOSCFG_LIB_LIBC
#include "string.h"
#endif /* LOSCFG_LIB_LIBC */
#include "los_printf.h"
#include "los_hwi.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

extern void dma_cache_clean(int swPageStart, int swPageEnd);
extern void *memset(void *dst, int c, size_t n);

/**
* This is opration for page table
*/
MMU_SET_PAGE_TABLE_PLACE

SENCOND_PAGE stOsPage = {0};
SENCOND_PAGE stAppPage = {0};

static void set_mem_page(MMU_PARAM *mPara)
{
    UINT32 page_start_index;
    UINT32 page_end_index;
    UINT32 length, uwCache, uwBuf, uwAP;
    UINT32 endAddr = mPara->endAddr;
    UINT32 page_size = (mPara->stPage->page_type == MMU_SECOND_LEVEL_BIG_PAGE_TABLE_ID) ? MMU_64K : MMU_4K;
    UINT32 stt_base = mPara->stPage->page_descriptor_addr;
    UINT32 right_shift_bit = (page_size == MMU_64K) ? SHIFT_64K : SHIFT_4K;

    if ((mPara->startAddr & (page_size - 1)) != 0)
    {
        return;
    }

    if ((endAddr & (page_size - 1)) != 0)
    {
        endAddr = (endAddr + (page_size -1)) & ~ (page_size - 1);
    }

    uwAP = MMU_AP_STATE(mPara->uwFlag);
    uwCache = MMU_CACHE_STATE(mPara->uwFlag);
    uwBuf = MMU_BUFFER_STATE(mPara->uwFlag);

    page_start_index = (mPara->startAddr - mPara->stPage->page_addr) >> right_shift_bit;
    page_end_index = (endAddr - mPara->stPage->page_addr) >> right_shift_bit;
    length = page_end_index - page_start_index;

    if (page_size == MMU_64K)
    {
        X_MMU_TWO_LEVEL_PAGE64K((mPara->startAddr >> SHIFT_64K), page_start_index, length, uwCache, uwBuf, uwAP); /*lint !e440 !e442 !e443 !e572*/
    }
    else
    {
        X_MMU_TWO_LEVEL_PAGE((mPara->startAddr >> SHIFT_4K), page_start_index, length, uwCache, uwBuf, uwAP); /*lint !e572*/
    }
}

VOID LOS_SecPageEnable(SENCOND_PAGE *stPage, UINT32 uwFlag)
{
    UINT32 uwPageStart;
    UINT32 uwPageEnd;
    UINT32 ttb_base = FIRST_PAGE_DESCRIPTOR_ADDR;
    MMU_PARAM mPara;

    if (stPage == NULL)
    {
        PRINT_ERR("second page table(stPage) can't be NULL\n");
        return;
    }
    mPara.startAddr = stPage->page_addr;
    mPara.endAddr = stPage->page_addr + stPage->page_length;
    mPara.uwFlag = uwFlag;
    mPara.stPage = stPage;

    uwPageStart = stPage->page_descriptor_addr;
    uwPageEnd = stPage->page_descriptor_addr + (stPage->page_length >>12 << 2); /*lint !e504*//* page size = 2^12, 4K*/

    disable_ap_check();

    set_mem_page(&mPara); /*lint !e522 */
    dma_cache_clean((int)uwPageStart, (int)uwPageEnd);

    X_MMU_ONE_LEVEL_PAGE((stPage->page_descriptor_addr >> SHIFT_1K), (stPage->page_addr >> SHIFT_1M), (stPage->page_length >> SHIFT_1M)); /*lint !e681 */
    dma_cache_clean((int)(ttb_base + (mPara.startAddr >> SHIFT_1M) * 4), (int)(ttb_base + (mPara.endAddr >> SHIFT_1M) * 4));

    clean_tlb();

    enable_ap_check();
}

void LOS_MMUParamSet(MMU_PARAM *mPara)
{
    UINT32 uwPageStart;
    UINT32 uwPageEnd;
    UINT32 uwItemStart;
    UINT32 uwItemEnd;
    UINT32 uwItemTemp;
    UINT32 uwCache, uwBuf, uwAP,uwTableType;
    UINTPTR uvIntSave;
    if (mPara == NULL)
    {
        PRINT_ERR("input is null\n");
        return;
    }
    uwItemStart = MMU_GET_FIRST_TABLE_ADDR(mPara->startAddr);
    uwItemEnd = MMU_GET_FIRST_TABLE_ADDR(mPara->endAddr - 1);
    uwItemTemp = uwItemStart;
    if (uwItemStart > uwItemEnd)
    {
        PRINT_ERR("wrong addr input, uwItemStart:0x%x, uwItemEnd:0x%x\n", uwItemStart, uwItemEnd);
        return;
    }

    uwAP = MMU_AP_STATE(mPara->uwFlag);
    uwCache = MMU_CACHE_STATE(mPara->uwFlag);
    uwBuf = MMU_BUFFER_STATE(mPara->uwFlag);
    uwTableType = MMU_GET_AREA(mPara->uwFlag);

    if (uwTableType == SECOND_PAGE)
    {
        if (mPara->startAddr < mPara->stPage->page_addr || mPara->endAddr > (mPara->stPage->page_length + mPara->stPage->page_addr))
        {
            PRINT_ERR("addr input not belongs to this second page \n");
            PRINT_ERR("mPara->startAddr:0x%x, mPara->stPage->page_addr:0x%x\n", mPara->startAddr,mPara->stPage->page_addr);
            PRINT_ERR("mPara->endAddr:0x%x, (mPara->stPage->page_length + mPara->stPage->page_addr):0x%x\n", mPara->endAddr, (mPara->stPage->page_length + mPara->stPage->page_addr));
            return;
        }
        while (uwItemTemp <= uwItemEnd)
        {
            if ((*(UINT32 *)uwItemTemp & 0x3) != MMU_FIRST_LEVEL_PAGE_TABLE_ID)
            {
                PRINT_ERR("not all mem belongs to second page(4K or 64K every item), mmu table ID:%d \n", (*(UINT32 *)uwItemTemp & 0x3));
                return;
            }
            uwItemTemp += 4;
        }

        uvIntSave = LOS_IntLock();
        disable_ap_check();
        set_mem_page(mPara); /*lint !e522*/
        enable_ap_check();
        LOS_IntRestore(uvIntSave);
        uwPageStart = MMU_GET_SECOND_TABLE_ADDR(mPara->startAddr);
        uwPageEnd = MMU_GET_SECOND_TABLE_ADDR(mPara->endAddr - 1);
        dma_cache_clean((int)uwPageStart, (int)uwPageEnd);
    }
    else  if (uwTableType == FIRST_SECTION)
    {
        while (uwItemTemp <= uwItemEnd)
        {
            if ((*(UINT32 *)uwItemTemp & 0x3) != MMU_FIRST_LEVEL_SECTION_ID)
            {
                PRINT_ERR("not all mem belongs to first section(1M every item), mmu table ID:%d\n", (*(UINT32 *)uwItemTemp & 0x3));
                return;
            }
            uwItemTemp += 4;
        }

        uwItemTemp = uwItemStart;
        uvIntSave = LOS_IntLock();
        disable_ap_check();
        while (uwItemTemp <= uwItemEnd)
        {
            SECTION_CHANGE(uwItemTemp, uwCache, uwBuf, uwAP);
            uwItemTemp += 4;
        }
        enable_ap_check();
        LOS_IntRestore(uvIntSave);
        dma_cache_clean((int)uwItemStart, (int)uwItemEnd);
    }

    clean_tlb();
}/*lint !e438*/

VOID osSecPageInit(VOID)
{
    X_MMU_SECOND_TABLE_OS_PAGE_SET();
    X_MMU_SECOND_TABLE_APP_PAGE_SET();

}
VOID osRemapCached(unsigned long phys_addr, unsigned long size)
{
    if(phys_addr < MMZ_MEM_BASE)
        return;
    MMU_PARAM mPara;

    mPara.startAddr = phys_addr;
    mPara.endAddr = phys_addr + size;
    mPara.uwFlag = BUFFER_ENABLE|CACHE_ENABLE|ACCESS_PERM_RW_RW;
    mPara.stPage = (SENCOND_PAGE *)&stAppPage;
    LOS_MMUParamSet(&mPara);
}

VOID osRemapNoCached(unsigned long phys_addr, unsigned long size)
{
    if(phys_addr < MMZ_MEM_BASE)
        return;
    MMU_PARAM mPara;

    mPara.startAddr = phys_addr;
    mPara.endAddr = phys_addr + size;
    mPara.uwFlag = BUFFER_DISABLE|CACHE_DISABLE|ACCESS_PERM_RW_RW;
    mPara.stPage = (SENCOND_PAGE *)&stAppPage;
    LOS_MMUParamSet(&mPara);
}

void code_protect(void)
{
    CODE_PROTECT;
}

INT32 osMemNoAccessSet(AARCHPTR startaddr, size_t length)
{
    UINT32 ttb_base = FIRST_PAGE_DESCRIPTOR_ADDR;
    AARCHPTR endAddr = 0;
    endAddr = startaddr + length;
    if (startaddr >= endAddr)
    {
        PRINT_ERR("The Input param invalid ,length equal 0 or the configuration scope overflow.startaddr:0x%x, length:0x%x\n", startaddr, length);
        return -1;
    }

    if ((startaddr >= SYS_MEM_BASE) && (startaddr < g_sys_mem_addr_end))  /*lint !e685  !e568*/
    {
        PRINT_ERR("The no access permission area sholud not contain os system mem,startaddr:0x%x\n", startaddr);
        return -1;
    }
    if ((endAddr > SYS_MEM_BASE) && (endAddr <= g_sys_mem_addr_end))  /*lint !e685  !e568*/
    {
        PRINT_ERR("The no access permission area sholud not contain os system mem,endAddr:0x%x\n", endAddr);
        return -1;
    }
    if (((startaddr & ( MMU_1M - 1)) != 0) || ((length & ( MMU_1M - 1)) != 0))
    {
        PRINT_ERR("The start address or the length is not aligned as 1M, startaddr:0x%x, length:0x%x\n", startaddr, length);
        return -1;
    }

    PRINTK("osMemNoAccessSet:startaddr:%x,length:%x\n", startaddr, length);

    //disable_ap_check();
    X_MMU_SECTION((startaddr >> SHIFT_1M), (startaddr >> SHIFT_1M), (length >> SHIFT_1M), 0, 0, 0, 2); /*lint !e572*/
    dma_cache_clean((UINT32)(ttb_base + (startaddr >> SHIFT_1M) * 4), (UINT32)(ttb_base + (endAddr >> SHIFT_1M) * 4));

    //clean_tlb();
    //enable_ap_check();
    return 0;
}
void osPrintPageItem (MMU_PARAM *mPara)
{
    UINT32 uwTmp;
    UINT32 start_addr;
    UINT32 page_len;

    if (mPara == NULL)
        return;

    if (MMU_GET_AREA(mPara->uwFlag) == SECOND_PAGE)
    {
        start_addr = mPara->stPage->page_descriptor_addr + ((mPara->startAddr - mPara->stPage->page_addr) >> SHIFT_4K) * 4;
        page_len = ((mPara->endAddr - mPara->startAddr) >> SHIFT_4K) * 4;
        if ((mPara->endAddr & (MMU_4K - 1)) != 0)
        {
            page_len += 4;
        }
        PRINTK("SECOND_PAGE:\n");
    }
    else if (MMU_GET_AREA(mPara->uwFlag) == FIRST_SECTION)
    {
        start_addr = FIRST_PAGE_DESCRIPTOR_ADDR + (mPara->startAddr >> SHIFT_1M) * 4;
        page_len = ((mPara->endAddr - mPara->startAddr) >> SHIFT_1M) * 4;
        if ((mPara->endAddr & (MMU_1M - 1)) != 0)
        {
            page_len += 4;
        }
        PRINTK("FIRST_SECTION:\n");
    }
    else
        return;

    PRINTK("mPara->endAddr = 0x%x mPara->startAddr = 0x%x page_len = %d * 4\n", mPara->endAddr, mPara->startAddr, page_len/4);

    for (uwTmp = 0; uwTmp < page_len; uwTmp += 4)
    {
        if (uwTmp % 20 == 0)
            PRINTK("\n");
        PRINTK("%x  ", *(UINT32 *)(start_addr + uwTmp));
    }
    PRINTK("\n");
}

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
