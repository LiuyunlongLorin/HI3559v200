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

/** @defgroup mmu Memory management unit
 *  @ingroup mmu
*/

#ifndef __MMU_CONFIG_H
#define __MMU_CONFIG_H

#include "board.h"
#include "los_config.h"
#include "los_memory.h"
#include "mmu.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#define MMU_SET_PAGE_TABLE_PLACE  \
__attribute__((aligned(MMU_16K))) __attribute__((section(".bss.prebss.translation_table"))) UINT8 first_page_table[MMU_16K]; \
__attribute__((aligned(MMU_1K))) UINT8 second_page_table_os[MMU_16K]; \
__attribute__((aligned(MMU_1K))) UINT8 second_page_table_app[DDR_MEM_SIZE/MMU_1K];

/**
 * @ingroup mmu
 * The liteos cache addr & length
 */
#define LITEOS_CACHE_ADDR               SYS_MEM_BASE
#define LITEOS_CACHE_LENGTH           (g_sys_mem_addr_end - LITEOS_CACHE_ADDR)

/**
 * @ingroup mmu
 * The page table storage addr
 * notice: must ensure it has enough free mem for storage page table
 */
extern UINT8 first_page_table[MMU_16K];
#define FIRST_PAGE_DESCRIPTOR_ADDR            ((UINT32)first_page_table)

#define X_MMU_SECOND_TABLE_OS_PAGE_SET() \
do{ \
    extern unsigned long __ram_data_start; \
    stOsPage.page_addr = SYS_MEM_BASE;  \
    stOsPage.page_length = ((((unsigned long)&__ram_data_start - SYS_MEM_BASE) + MMU_1M - 1)& ~ (MMU_1M - 1)); \
    stOsPage.page_descriptor_addr = (UINT32)second_page_table_os;   \
    stOsPage.page_type = MMU_SECOND_LEVEL_SMALL_PAGE_TABLE_ID;  \
    if(stOsPage.page_length > (sizeof(second_page_table_os) << 10)) {  \
        PRINT_ERR("%s,%d\n", __FUNCTION__,__LINE__); \
        PRINT_ERR("the mapping size of os second page is 0x%x, sholud be not bigger than 0x%x\n", stOsPage.page_length,(sizeof(second_page_table_os) << 10));\
        return;\
    }\
    LOS_SecPageEnable(&stOsPage, BUFFER_ENABLE|CACHE_ENABLE|ACCESS_PERM_RW_RW); \
}while(0)

#define X_MMU_SECOND_TABLE_APP_PAGE_SET() \
do{ \
	stAppPage.page_addr = MMZ_MEM_BASE;  \
	stAppPage.page_length = MMZ_MEM_LEN; \
	stAppPage.page_descriptor_addr = ((UINT32)second_page_table_app + stOsPage.page_length/MMU_1M*MMU_1K); \
	stAppPage.page_type = MMU_SECOND_LEVEL_SMALL_PAGE_TABLE_ID;  \
	LOS_SecPageEnable(&stAppPage, BUFFER_DISABLE|CACHE_DISABLE|ACCESS_PERM_RW_RW); \
}while(0)

#define MMU_GET_FIRST_TABLE_ADDR(addr)    (((addr)/MMU_1M)*4 + FIRST_PAGE_DESCRIPTOR_ADDR)  //table start position + offset = 'addr' table item position
#define MMU_GET_FIRST_TABLE_ITEM(addr)    (*(UINT32 *)MMU_GET_FIRST_TABLE_ADDR(addr)) // get item content which storaged by table
#define MMU_GET_SECOND_TABLE_BASE(addr) ((MMU_GET_FIRST_TABLE_ITEM(addr)) & 0xfffffc00) //if the first item ID is MMU_FIRST_LEVEL_PAGE_TABLE_ID, get second table item addr by hi 22bits
#define MMU_GET_SECOND_TABLE_OFFSET(addr) (((addr) % MMU_1M)/MMU_4K * 4) // second table item offset
#define MMU_GET_SECOND_TABLE_ADDR(addr)  (MMU_GET_SECOND_TABLE_BASE(addr) + MMU_GET_SECOND_TABLE_OFFSET(addr))

#define CODE_PROTECT  \
do \
{ \
    extern unsigned long __text_start, __ram_data_start; \
    MMU_PARAM mPara; \
\
    /*note: must confirm that every addr be aglined as 4K(64K)*/ \
    mPara.startAddr = (unsigned long)&__text_start; \
    mPara.endAddr = (unsigned long)&__ram_data_start; \
    mPara.uwFlag = BUFFER_ENABLE|CACHE_ENABLE|ACCESS_PERM_RO_RO; \
    mPara.stPage = (SENCOND_PAGE *)&stOsPage; \
    LOS_MMUParamSet(&mPara); \
}while(0)

/**
 *@ingroup mmu_config
 *@brief Memory Management Unit Cache/Buffer/Access Permission Setting.
 *
 *@par Description:
 *This API is used to set the Cache/Buffer/access permission mode of a section that is specified by a starting address and ending address
 *@attention
 *<ul>
 *<li>The passed-in starting address and ending address must be aligned on a boundary of 4K. The access permission mode can be only set to ACCESS_PERM_RO_RO and ACCESS_PERM_RW_RW.</li>
 *</ul>
 *
 *@param MMU_PARAM  [IN] param for mmu setting, the struct contains below members
 * startAddr:     Starting address of a section.
 * endAddr:      Ending address of a section.
 * uwFlag:        mode set. There are three func could be controlled with three bit.
 *                                   bit0: ACCESS_PERM_RW_RW/ACCESS_PERM_RO_RO(1/0)
 *                                   bit1: CACHE_ENABLE/CACHE_DISABLE(1/0)
 *                                   bit2: BUFFER_ENABLE/BUFFER_DISABLE(1/0)
 *                                   bit3: FIRST_SECTION/SECOND_PAGE(1/0) it need comfire your memory type, be descripted
 *                                   bit4~7: ignore
 * stPage:      the goal object of second page, if uwFlag bit3 is FIRST_SECTION, stPage will be ignored, and you can set this member as NULL
 *
 *@retval None.
 *@par Dependency:
 *<ul><li>mmu.h: the header file that contains the API declaration.</li></ul>
 *@see
 *@since Huawei LiteOS V100R001C00
 */
void LOS_MMUParamSet(MMU_PARAM *mPara);
#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
#endif
