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
#ifndef _MMU_H
#define _MMU_H

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#define UNCACHEABLE                 0
#define CACHEABLE                   1
#define UNBUFFERABLE                0
#define BUFFERABLE                  1
#define ACCESS_RW                   3 // ap = 0  ap1 = 0b11
#define ACCESS_RO                   7  // ap = 1 ap1 = 0b11
#define ACCESS_NA                   0  // ap = 0 ap1 = 0

#define MMU_AP_STATE(flag)          (((flag) & 0x1)?ACCESS_RW:ACCESS_RO)
#define MMU_CACHE_STATE(flag)       (((flag) >> 1) & 0x1)
#define MMU_BUFFER_STATE(flag)      (((flag) >> 2) & 0x1)
#define MMU_GET_AREA(flag)          ((flag)&(0x1<<3))

/**
 * @ingroup mmu
 * The access permission mode is read-only.
 */
#define ACCESS_PERM_RO_RO                   0

/**
 * @ingroup mmu
 * The access permission mode is read and write.
 */
#define ACCESS_PERM_RW_RW                   (1<<0)

/**
 * @ingroup mmu
 * The cache enabled.
 */
#define CACHE_ENABLE                   (1<<1)

/**
 * @ingroup mmu
 * The cache disabled.
 */
#define CACHE_DISABLE                   0

/**
 * @ingroup mmu
 * The buffer enabled.
 */
#define BUFFER_ENABLE                   (1<<2)

/**
 * @ingroup mmu
 * The buffer disabled.
 */
#define BUFFER_DISABLE                   0

/**
 * @ingroup mmu
 * The first section(1M/item).
 */
#define FIRST_SECTION                  (1<<3)

/**
 * @ingroup mmu
 * The second page(4K/item).
 */
#define SECOND_PAGE             0

#define MMU_1K    0x400
#define MMU_4K    0x1000
#define MMU_16K    0x4000
#define MMU_64K    0x10000
#define MMU_1M    0x100000
#define MMU_4G     0x100000000ull

#define SHIFT_1K        (10)
#define SHIFT_4K        (12)
#define SHIFT_16K        (14)
#define SHIFT_64K        (16)
#define SHIFT_1M        (20)
#define SHIFT_2M        (21)
#define SHIFT_1G        (30)

/**
 * @ingroup mmu
 * mmu second page information structure.
 *
 */
typedef struct
{
    UINT32 page_addr;           /**< The second page start addr         */
    UINT32 page_length;         /**< The second page length             */
    UINT32 page_descriptor_addr;    /**< The second page page table storage addr, diff second page table can't be coincided   */
    UINT32 page_type;           /**< The second page type, it can be set small page ID(4K):MMU_SECOND_LEVEL_SMALL_PAGE_TABLE_ID
                                                                                            or big page ID(64K):MMU_SECOND_LEVEL_BIG_PAGE_TABLE_ID        */
}SENCOND_PAGE;

/**
 * @ingroup mmu
 * mmu param setting information structure.
 *
 */
typedef struct
{
    UINT32 startAddr;      /**< Starting address of a section.     */
    UINT32 endAddr;       /**< Ending address of a section.     */
    UINT32 uwFlag;         /*<   mode set. There are three func could be controlled with three bit.
                                                bit0: ACCESS_PERM_RW_RW/ACCESS_PERM_RO_RO(1/0)
                                                bit1: CACHE_ENABLE/CACHE_DISABLE(1/0)
                                                bit2: BUFFER_ENABLE/BUFFER_DISABLE(1/0)
                                                bit3: FIRST_SECTION/SECOND_PAGE(1/0) it need comfire your memory type, be descripted
                                                bit4~7: ignore*/
    SENCOND_PAGE *stPage;    /*<  the goal object of second page, if uwFlag bit3 is FIRST_SECTION, stPage will be ignored, and you can set this member as NULL */
}MMU_PARAM;

// ARM Domain Access Control Bit Masks
#define ACCESS_TYPE_NO_ACCESS(domain_num)    (0x0 << ((domain_num)*2))
#define ACCESS_TYPE_CLIENT(domain_num)       (0x1 << ((domain_num)*2))
#define ACCESS_TYPE_MANAGER(domain_num)      (0x3 << ((domain_num)*2))

#define MMU_FIRST_LEVEL_FAULT_ID 0x0
#define MMU_FIRST_LEVEL_PAGE_TABLE_ID 0x1
#define MMU_FIRST_LEVEL_SECTION_ID 0x2
#define MMU_FIRST_LEVEL_RESERVED_ID 0x3
/**
 * @ingroup mmu
 * The second page type select 64K
 */
#define MMU_SECOND_LEVEL_BIG_PAGE_TABLE_ID 0x1

/**
 * @ingroup mmu
 * The second page type select 4K
 */
#define MMU_SECOND_LEVEL_SMALL_PAGE_TABLE_ID 0x2

struct MMU_FIRST_LEVEL_FAULT {
    UINT32 id : 2;
    UINT32 sbz : 30;
};

struct MMU_FIRST_LEVEL_PAGE_TABLE {
    UINT32 id : 2;
    UINT32 imp : 3;
    UINT32 domain : 4;
    UINT32 sbz : 1;
    UINT32 base_address : 22;
};

struct MMU_SECOND_LEVEL_BIG_PAGE_TABLE {
    UINT32 id : 2;
    UINT32 b : 1;
    UINT32 c : 1;
    UINT32 ap1 : 2;
    UINT32 sbz : 3;
    UINT32 ap : 1;
    UINT32 s : 1;
    UINT32 ng : 1;
    UINT32 tex : 3;
    UINT32 xn : 1;
    UINT32 base_address : 16;
};

struct MMU_SECOND_LEVEL_SMALL_PAGE_TABLE {
    UINT32 xn : 1;
    UINT32 id : 1;
    UINT32 b : 1;
    UINT32 c : 1;
    UINT32 ap1 : 2;
    UINT32 tex : 3;
    UINT32 ap : 1;
    UINT32 s : 1;
    UINT32 ng : 1;
    UINT32 base_address : 20;
};

struct MMU_FIRST_LEVEL_SECTION {
    UINT32 id : 2;
    UINT32 b : 1;
    UINT32 c : 1;
    UINT32 xn : 1;
    UINT32 domain : 4;
    UINT32 imp : 1;
    UINT32 ap1 : 2;
    UINT32 tex : 3;
    UINT32 ap : 1;
    UINT32 s : 1;
    UINT32 ng : 1;
    UINT32 revs : 1;
    UINT32 ns : 1;
    UINT32 base_address : 12;
};

struct MMU_FIRST_LEVEL_RESERVED {
    UINT32 id : 2;
    UINT32 sbz : 30;
};

#define SECTION_CHANGE(Item, cache, buff, access)   \
{ \
        union MMU_FIRST_LEVEL_DESCRIPTOR desc;  \
        desc.word = (*(UINT32 *)Item);      \
        desc.section.c = (cache);       \
        desc.section.b = (buff);        \
        desc.section.ap1 = (access) & 0x3;      \
        desc.section.ap = (access) >> 2;        \
        (*(UINT32 *)Item)= desc.word;   \
}

#define X_MMU_SECTION(abase,vbase,size,cache,buff,access,sdomain)      \
{ \
    UINT32 i; UINT32 j = abase; UINT32 k = vbase;                         \
    union MMU_FIRST_LEVEL_DESCRIPTOR desc;               \
    desc.word = 0;                                                    \
    desc.section.id = MMU_FIRST_LEVEL_SECTION_ID;                 \
    desc.section.b = (buff);                                    \
    desc.section.c = (cache);                                     \
    desc.section.xn = 0;                                             \
    desc.section.domain = (sdomain);                                          \
    desc.section.ap1 = (access) & 0x3;                                         \
    desc.section.ap = (access) >> 2;                                         \
    k <<= 2;                        \
    k += ttb_base;      \
    for (i = 0; i < size; ++i, ++j, k+=4)                          \
    {                                                            \
        desc.section.base_address = (j);                        \
        *(UINT32 *)k = desc.word;                                            \
    }                                                            \
}

#define X_MMU_ONE_LEVEL_PAGE(abase, vbase, size)      \
{ \
    UINT32 i; UINT32 j = abase; UINT32 k = vbase;                         \
    union MMU_FIRST_LEVEL_DESCRIPTOR desc;               \
    desc.word = 0;                                                    \
    desc.page_table.id = MMU_FIRST_LEVEL_PAGE_TABLE_ID;                 \
    desc.page_table.imp = 0;                                             \
    desc.page_table.domain = 1;                                          \
    k <<= 2;                        \
    k += ttb_base;      \
    for (i = 0; i < size; ++i, ++j, k+=4)                          \
    {                                                            \
        desc.page_table.base_address = (j);                        \
        *(UINT32 *)k = desc.word;                                            \
    }                                                            \
}

#define X_MMU_TWO_LEVEL_PAGE(abase, vbase, size, cache, buff, access)      \
{  \
    UINT32 i; UINT32 j = abase; UINT32 k = vbase;                         \
    union MMU_FIRST_LEVEL_DESCRIPTOR desc;               \
    desc.word = 0;                                                                       \
    desc.small_page_table.xn = 0;                 \
    desc.small_page_table.id = MMU_SECOND_LEVEL_SMALL_PAGE_TABLE_ID >> 1;                 \
    desc.small_page_table.b = (buff);                                             \
    desc.small_page_table.c = (cache);                                         \
    desc.small_page_table.ap1 = (access) & 0x3;;                                    \
    desc.small_page_table.ap = (access) >> 2;                                          \
    k <<= 2;                        \
    k += stt_base;      \
    for (i = 0; i < size; ++i, ++j, k+=4)                          \
    {                                                            \
        desc.small_page_table.base_address = (j);                        \
        *(UINT32 *)k = desc.word;                                            \
    }                                                            \
}

#define X_MMU_TWO_LEVEL_PAGE64K(abase, vbase, size, cache, buff, access)      \
{  \
    UINT32 i; UINT32 j = abase; UINT32 k = vbase; UINT32 n;                         \
    union MMU_FIRST_LEVEL_DESCRIPTOR desc;               \
    desc.word = 0;                                                    \
    desc.big_page_table.id = MMU_SECOND_LEVEL_BIG_PAGE_TABLE_ID;                 \
    desc.big_page_table.b = (buff);                                             \
    desc.big_page_table.c = (cache);                                         \
    desc.big_page_table.ap1 = (access) & 0x3;                                     \
    desc.big_page_table.ap = (access) >> 2;                                       \
    desc.big_page_table.xn = 0;                                          \
    k <<= 2;                        \
    k += stt_base;      \
    for (i = 0; i < size; ++i, ++j)                          \
    {                                                            \
        desc.big_page_table.base_address = (j);                        \
        for (n = 0; n < 16; ++n, k += 4)   \
            *(UINT32 *)k = desc.word;                                            \
    }                                                            \
}

union MMU_FIRST_LEVEL_DESCRIPTOR {
    unsigned long word;
    struct MMU_FIRST_LEVEL_FAULT fault;
    struct MMU_FIRST_LEVEL_PAGE_TABLE page_table;
    struct MMU_FIRST_LEVEL_SECTION section;
    struct MMU_FIRST_LEVEL_RESERVED reserved;
    struct MMU_SECOND_LEVEL_SMALL_PAGE_TABLE small_page_table;
    struct MMU_SECOND_LEVEL_BIG_PAGE_TABLE big_page_table;
};

static inline void enable_ap_check(void)
{
    UINT32 uwReg =
        ACCESS_TYPE_MANAGER(0)    |
        ACCESS_TYPE_CLIENT(1);
    __asm volatile ("mcr  p15,0,%0,c3,c0,0" : : "r"(uwReg));
} /*lint !e529*/

static inline void disable_ap_check(void)
{
    UINT32 uwReg =
        ACCESS_TYPE_MANAGER(0)    |
        ACCESS_TYPE_MANAGER(1);
    __asm volatile ("mcr  p15,0,%0,c3,c0,0" : : "r"(uwReg));
} /*lint !e529*/

static inline void clean_tlb(void)
{
    __asm volatile ( "mov    %0, #0\n" "mcr    p15, 0, %0, c8, c7, 0\n" : : "r" (0));
}

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */


#endif /* _MMU_H*/
