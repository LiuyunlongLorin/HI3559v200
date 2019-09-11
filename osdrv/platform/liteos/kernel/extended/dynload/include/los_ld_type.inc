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

/** @defgroup dynload dynamic loading
 *@ingroup kernel
 */

#ifndef _KERNEL_LOS_LD_TYPE_INC
#define _KERNEL_LOS_LD_TYPE_INC

#include "los_ld_elf.inc"
#include "los_ld_macro.inc"

#include "los_list.ph"
#include "los_typedef.ph"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#ifdef LOSCFG_AARCH64
typedef UINT64 AARCHUINT;
#else
typedef UINT32 AARCHUINT;
#endif

#ifdef LOSCFG_AARCH64
#define UINTMAX UINT64_MAX
#else
#define UINTMAX UINT32_MAX
#endif


/* Maximum number of modules that are allowed to be loaded by Huawei LiteOS. */
#define N_MAX_USER_MODULES          10

#define N_MAX_MODULES               (N_MAX_USER_MODULES + 1)

/* Maximum number of default paths */
#define N_DEFAULT_PATH  10

extern LOS_DL_LIST  stFreeHandleList;
extern VOID         *pDynloadHeap;

/* Symbol types */
#define ELF_STT_NOTYPE      0x0     /* Unknown type */
#define ELF_STT_OBJECT      0x1     /* Data object */
#define ELF_STT_FUNC        0x2     /* Function */
#define ELF_STT_SECTION     0x3     /* Section */
#define ELF_STT_FILE        0x4     /* File name*/

/* Symbol binding information */
#define ELF_STB_LOCAL       0x0     /* Local */
#define ELF_STB_GLOBAL      0x1     /* Global */
#define ELF_STB_WEAK        0x2     /* Weak reference */

/* Information about the section in which a symbol is located. The following are constants. */
#define ELF_SHN_ABS         0xFFF1  /* Absolute symbol value, for example, a file name. */
#define ELF_SHN_COMMON      0xFFF2  /* COMMON block */
#define ELF_SHN_UNDEF       0x0000  /* Undefined symbol */

/* Symbol types */
#define OS_SHN_UNDF         0x0     /* Undefined */
#define OS_SHN_LOCAL        0x0     /* Local */
#define OS_SHN_GLOBAL       0x1     /* Global (external) (ORed) */
#define OS_SHN_ABS          0x2     /* Absolute */
#define OS_SHN_TEXT         0x4     /* Text */
#define OS_SHN_DATA         0x6     /* Data */
#define OS_SHN_BSS          0x8     /* BSS */
#define OS_SHN_COMM         0x10    /* Common symbol */
#define OS_SHN_THUMB        0x80    /* Thumb function */

/* New load flags */
#define OS_NO_SYMBOLS       0x1     /* No symbols added to the target
                                       symbol table
                                       */
#define OS_LOCAL_SYMBOLS    0x2     /* Only internal symbols added to
                                       the target symbol table
                                       */
#define OS_GLOBAL_SYMBOLS   0x4     /* Only external symbols added to
                                       the target symbol table
                                       */
#define OS_ALL_SYMBOLS      (OS_LOCAL_SYMBOLS | OS_GLOBAL_SYMBOLS) /* All symbols added to the target
                                                                      symbol table
                                                                      */

#define ELF_BIND_LOCAL      0
#define ELF_BIND_GLOBAL     1
#define ELF_BIND_WEAK       2
#define ELF_BIND_LOPROC     13
#define ELF_BIND_HIPROC     15

#define ELF_TYPE_NOTYPE     0
#define ELF_TYPE_OBJECT     1
#define ELF_TYPE_FUNC       2
#define ELF_TYPE_SECTION    3
#define ELF_TYPE_FILE       4
#define ELF_TYPE_LOPROC     13
#define ELF_TYPE_HIPROC     15

#define MAX_SEARCH_SCOPE    50


/* 0~15 bits maintain the module temporary flags during loadding, unloadding and searching. */
#define IS_LOADDING(handle)             ((handle)->uwModuleStatus & (1 << 0))
#define IS_UNLOADDING(handle)           ((handle)->uwModuleStatus & (1 << 1))
#define IS_SEARCHING(handle)            ((handle)->uwModuleStatus & (1 << 2))
#define SET_LOADDING(handle)            ((handle)->uwModuleStatus |= (1 << 0))
#define SET_UNLOADDING(handle)          ((handle)->uwModuleStatus |= (1 << 1))
#define SET_SEARCHING(handle)           ((handle)->uwModuleStatus |= (1 << 2))
#define CLEAR_SEARCHING(handle)         ((handle)->uwModuleStatus &= (~(1 << 2)))
#define CLEAR_TMPFLAGS(handle)          ((handle)->uwModuleStatus &= 0xffff0000)
/* 16~31 bits maintain the module status after load. */
#define IS_LOADED(handle)               ((handle)->uwModuleStatus & (1 << 16))
#define IS_TAKEN(handle)                ((handle)->uwModuleStatus & (1 << 17))
#define SET_LOADED(handle)              ((handle)->uwModuleStatus |= (1 << 16))
#define SET_TAKEN(handle)               ((handle)->uwModuleStatus |= (1 << 17))
#define CLEAR_LOADED(handle)            ((handle)->uwModuleStatus &= (~(1 << 16)))
#define CLEAR_TAKEN(handle)             ((handle)->uwModuleStatus &= (~(1 << 17)))
#define CLEAR_MODULESTATUS(handle)      ((handle)->uwModuleStatus = 0)

#define INC_SOHEAD(handle)              ((handle)->swHeadCnt++)
#define DEC_SOHEAD(handle)              ((handle)->swHeadCnt--)
#define IS_SOHEAD(handle)               ((handle)->swHeadCnt > 0)




#define OS_RELOC_COPY_WORD(pDst, pSrc)  \
    do { \
        UINT32 *puwDst = (UINT32*)(pDst); \
        UINT32 *puwSrc = (UINT32*)(pSrc); \
        *puwDst = *puwSrc;  puwDst++; puwSrc++; \
        *puwDst = *puwSrc;  puwDst++; puwSrc++; \
        *puwDst = *puwSrc; \
    } while (0)

/* Relocation Type Definitions */
#define OS_R_ARM_NONE           0   /* No reloc */
#define OS_R_ARM_PC24           1
#define OS_R_ARM_ABS32          2
#define OS_R_ARM_REL32          3
#define OS_R_ARM_LDR_PC_G0      4
#define OS_R_ARM_ABS16          5
#define OS_R_ARM_ABS12          6
#define OS_R_ARM_THM_ABS5       7
#define OS_R_ARM_ABS8           8
#define OS_R_ARM_SBREL32        9
#define OS_R_ARM_THM_CALL       10
#define OS_R_ARM_THM_PC8        11
#define OS_R_ARM_BREL_ADJ       12
#define OS_R_ARM_TLS_DESC       13
#define OS_R_ARM_THM_SWI8       14
#define OS_R_ARM_XPC25          15
#define OS_R_ARM_THM_XPC22      16
#define OS_R_ARM_COPY           20  /* Copy symbol at runtime */
#define OS_R_ARM_GLOB_DAT       21  /* Create GOT entry */
#define OS_R_ARM_JUMP_SLOT      22  /* Create PLT entry */
#define OS_R_ARM_RELATIVE       23  /* Adjust by program base */
#define OS_R_ARM_GOTOFF32       24  /* 32 bit offset to GOT */
#define OS_R_ARM_BASE_PREL      25  /* 32 bit PC relative offset to GOT */
#define OS_R_ARM_GOT_BREL       26  /* 32 bit GOT entry */
#define OS_R_ARM_PLT32          27  /* 32 bit PLT address */
#define OS_R_ARM_CALL           28
#define OS_R_ARM_THM_JUMP24     30
#define OS_R_ARM_MOVW_ABS_NC    43
#define OS_R_ARM_MOVT_ABS       44
#define OS_ARM_THM_MOVW_ABS_NC  47
#define OS_ARM_THM_MOVT_ABS     48

/* These are GNU extensions to enable C++ vtable garbage collection. */
#define OS_R_ARM_GNU_VTENTRY    100
#define OS_R_ARM_GNU_VTINHERIT  101

#define OS_R_ARM_THM_JUMP11     102 /* Thumb unconditional branch */
#define OS_R_ARM_THM_JUMP8      103 /* Thumb conditional branch */
#define OS_R_ARM_RXPC25         249
#define OS_R_ARM_RSBREL32       250
#define OS_R_ARM_THM_RPC22      251
#define OS_R_ARM_RREL32         252
#define OS_R_ARM_RABS32         253
#define OS_R_ARM_RPC24          254
#define OS_R_ARM_RBASE          255

#define OS_R_ARM_AARCH64_ABS64                      257 /* Direct 64 bit. */
#define OS_R_ARM_AARCH64_ADR_PREL_PG_HI21           275
#define OS_R_ARM_AARCH64_ADD_ABS_LO12_NC            277
#define OS_R_ARM_AARCH64_LDST8_ABS_LO12_NC          278
#define OS_R_ARM_AARCH64_CALL26                     283
#define OS_R_ARM_AARCH64_LDST16_ABS_LO12_NC         284
#define OS_R_ARM_AARCH64_LDST32_ABS_LO12_NC         285
#define OS_R_ARM_AARCH64_LDST64_ABS_LO12_NC         286



/* AARCH64 for  Dynamic relocations */
#define OS_R_ARM_AARCH64_COPY                       1024
#define OS_R_ARM_AARCH64_GLOB_DAT                   1025
#define OS_R_ARM_AARCH64_JUMP_SLOT                  1026
#define OS_R_ARM_AARCH64_RELATIVE                   1027
#define OS_R_ARM_AARCH64_TLS_DTPREL64               1028
#define OS_R_ARM_AARCH64_TLS_DTPMOD64               1029
#define OS_R_ARM_AARCH64_TLS_TPREL64                1030
#define OS_R_ARM_AARCH64_TLSDESC                    1031
#define OS_R_ARM_AARCH64_IRELATIVE                  1032

#define OS_BIT_AND(x, y)    ((x) & (y))

/**
* Macro capable of relocating ARM symbols
*/

/* Macro capable of modifying the lower X bits */
#define OS_INSERT(buf, value, mask) ((buf) = (((buf) & ~(mask)) | ((value) & (mask))))

#define OS_LOW24_INSERT(buf, value) OS_INSERT((buf), (value), 0x00ffffff)

#define OS_LOW16_INSERT(buf, value) OS_INSERT((buf), (value), 0x0000ffff)

#define OS_LOW12_INSERT(buf, value) OS_INSERT((buf), (value), 0x00000fff)

#define OS_LOW11_INSERT(buf, value) OS_INSERT((buf), (value), 0x000007ff)

#define OS_LOW8_INSERT(buf, value)  OS_INSERT((buf), (value), 0x000000ff)

#define OS_BITS6_10_INSERT(buf, value)  OS_INSERT((buf), (value) << 6, 0x000007c0)

#define OS_UBIT_INSERT(buf, value)  OS_INSERT((buf), (value) << 23 , 0x0080000)

/* The values of n bits of a binary number are 1 */
#define OS_MASK(n)  (((AARCHUINT)1 << (n)) - 1)

/* The value of bit n of a binary number is 1 */
#define OS_SIGN_BIT_MASK(n) ((AARCHUINT)1 << ((n) - 1))

/* Add an extended bit to the upper bit */
#define OS_SIGN_EXTEND(val, nBits) \
    (((OS_MASK(nBits) & (val)) ^ OS_SIGN_BIT_MASK(nBits)) - OS_SIGN_BIT_MASK(nBits))

#define OS_SIGN_SHRINK(val, nBits) (OS_MASK(nBits) & (val))

/* Check whether the extended bit value falls within the nBits range */
#define OS_CHECK_FITS(val, nBits) \
    ((((val) & ~(((AARCHUINT)1 << ((nBits) - 1)) - 1)) == 0) || \
     (((val) & ~(((AARCHUINT)1 << ((nBits) - 1)) - 1)) == ~(AARCHUINT)(((AARCHUINT)1 << ((nBits) - 1)) - 1)))

#define OS_MEM_READ_32(pRelocAdrs, offset)  ((offset) = *((UINT32 *)(pRelocAdrs)))
#define OS_MEM_READ_16(pRelocAdrs, offset)  ((offset) = *((UINT16 *)(pRelocAdrs)))
#define OS_MEM_READ_8(pRelocAdrs, offset)   ((offset) = *((UINT8 *)(pRelocAdrs)))

#define OS_MEM_WRITE_32(pRelocAdrs, value32) \
    (*((UINT32 *)(pRelocAdrs)) = (value32))
#define OS_MEM_WRITE_16(pRelocAdrs, value16) \
    (*((UINT16 *)(pRelocAdrs)) = (value16))
#define OS_MEM_WRITE_8(pRelocAdrs, value8) \
    (*((UINT8 *)(pRelocAdrs)) = (value8))

#define LD_ELF_RELOC_CopyByWord(pDst, pSrc) \
    do { \
        UINT32 *puwDst = (UINT32 *)(pDst); \
        UINT32 *puwSrc = (UINT32 *)(pSrc); \
        *puwDst = *puwSrc;  puwDst++; puwSrc++; \
        *puwDst = *puwSrc;  puwDst++; puwSrc++; \
        *puwDst = *puwSrc; \
    } while (0)

#define OS_READ_INSTRUCTION_THUMB16(position) \
    (*(UINT16 *)(position))
#define OS_WRITE_INSTRUCTION_THUMB16(position, value16) \
    (*(UINT16 *)(position) = (UINT16)(value16))

#define OS_READ_INSTRUCTION_THUMB32(position) \
    (*(UINT16 *)(position) + ((UINT32)(*(UINT16 *)((UINT32)(position) + 2)) << 16))
#define OS_WRITE_INSTRUCTION_THUMB32(position, value32) \
    do { \
        *(UINT16 *)(position) = (UINT16)(value32); \
        *(UINT16 *)((UINT32)(position) + 2) = (UINT16)((UINT32)(value32) >> 16); \
    } while (0)

#define GET_BITS(BitWide) ((AARCHUINT)(-1) >> ((sizeof(AARCHUINT) * 8) - (BitWide)))

#define CLEAR_BITS_AREA(data, ucStartBit, BitWide)  \
    (data) &= ~(GET_BITS(BitWide) << ucStartBit);

#define GET_BITS_AREA(data, ucStartBit, BitWide) (data & (GET_BITS(BitWide) << ucStartBit))

#define OS_GET_PAGE_ADDR(expr) ((expr) & ~(AARCHUINT)0xfff)


typedef struct LD_RELOC_PARAM {
    UINT32       uwRelocType;
    UINT32       uwShType;              //REL section or RELA section.
    AARCHUINT    uwPosition;            //the address of the place being relocated 
    AARCHUINT    uwSymAdd;              //the address of the symbol
    ssize_t      swAddend;              //the addend for the relocation
    AARCHUINT    uwBaseSegment;         //the addressing origin of the output segment defining the symbol S.
    AARCHUINT    uwGotAddr;             //the address of the Global Offset Table
    UINT8        bThumbFunc;            //T is 1 if the target symbol S has type STT_FUNC and the symbol addresses a Thumb instruction; it is 0 otherwise. 
    UINT8        aucReserved[7];
} LD_RELOC_PARAM;

typedef struct LD_SYMBOL {         /* LD_SYMBOL - entry in symbol table */
    CHAR        *pscName;          /* Pointer to symbol name */
    INT8        *pwAddr;           /* Symbol address */
} LD_SYMBOL;

/* Data structures */
typedef struct LD_IDX_TBLS {
    UINT32 *puwLoadScnHdrIdxs;          /* Loadable sections header index tbl */
    UINT32 *puwSymTabScnHdrIdxs;        /* Sym table sections header idx tbl */
    UINT32 *puwRelScnHdrIdxs;           /* Reloc info sections header idx tbl */
    UINT32 *puwStrTabScnHdrIdxs;        /* Str table sections header idx tbl */
} LD_IDX_TBLS;

typedef struct LD_SYM_INFO {
    VOID *pAddr;    /* Symbol's address */
    INT8 scType;    /* Symbol's type */
} LD_SYM_INFO;

typedef struct DYN_COMSYM_INFO {
    LOS_DL_LIST pstLinkNode;
    AARCHUINT      uwSymAddr;
} DYN_COMSYM_INFO;

enum ELF_TYPE {
    ET_NONE = LD_ET_NONE,
    ET_OBJ  = LD_ET_REL,
    ET_EXEC = LD_ET_EXEC,
    ET_SO   = LD_ET_DYN
};

typedef struct SYMBOL_MANAGE {
    UINT32 uwStrLen;
    UINT32 uwSymNum;
    UINT32 uwStrTotalLen;
    UINT32 uwSymTotalNum;
    LD_SYMBOL *pstSymTbl;
    CHAR      *pstStrTbl;
} SYMBOL_MANAGE;

typedef struct DYN_MODULE_INFO {
    LOS_DL_LIST     stManageList;      /* Must comes first*/
    LOS_DL_LIST     stComSymInfoList;
    AARCHUINT       uwSegmentsAddr;
    AARCHUINT       uwSegmentsSize;
    UINT32          uwModuleStatus;
    INT32           swRefCnt;
    INT32           swHeadCnt;
    enum ELF_TYPE   enType;
    CHAR            *pscModuleFileName;
    SYMBOL_MANAGE   stSymManage;
    struct DYN_MODULE_INFO  *apDlNeeded[N_MAX_MODULES];
} DYN_MODULE_INFO;

enum RELOCATE_TYPE {
    EN_REL,
    EN_RELA,
    EN_JMPREL
};

typedef struct RELOCATION_INFO {
    AARCHUINT   uwRelTable;            //address of relocation table.
    UINT32      uwRelTableSize;        //total size of relocation table.
    UINT32      uwRelEntrySize;        //size of every relocation entry.
} RELOCATION_INFO;

typedef struct RELOCATION_INFO_T {
    RELOCATION_INFO stRel;
    RELOCATION_INFO stRela;
    RELOCATION_INFO stJmpRel;
} RELOCATION_INFO_T;

#define RELOCATION_INFO_COUNT    (sizeof(RELOCATION_INFO_T) / sizeof(RELOCATION_INFO))

typedef struct {
    LOS_DL_LIST     stDlLinkNode;

    AARCHUINT       swLoadStart;
    AARCHUINT       swLoadBias;

    AARCHUINT       uwMinVaddr;    //the minimum virtual address of load-segment.

    DYN_MODULE_INFO *pstModuleHandle;

    LD_ELF_EHDR     *pstElfHdr;

    LD_ELF_SHDR     *pstScnHdrTbl;
    UINT32          uwScnNum;

    LD_ELF_PHDR     *pstPhdrTbl;
    UINT32          uwPhnum;

    LD_ELF_DYN      *pstDynamic;

    CHAR            *pscStrtab;
    AARCHUINT       uwStrtabSize;

    LD_ELF_SYM      *pstSymtab;
    UINT32          uwSymnum;

    union {
        RELOCATION_INFO_T  stRelocations;
        RELOCATION_INFO    stRellocationInfos[RELOCATION_INFO_COUNT];
    } unRelocation;
} DL_INFO;

/* initLib */
extern DYN_MODULE_INFO stSysModuleInfo[N_MAX_MODULES];

typedef INT32 (*LOAD_FUNC)(DL_INFO *pstDlInfo);
typedef INT32 (*RELOC_FUNC)(DL_INFO *pstDlInfo);

/* commLib */
extern BOOL LD_ElfSymIsVisible(LD_ELF_SYM *pstSymbol);
extern INT8 LD_ElfSymTypeGet(LD_ELF_SYM *pstSymbol, LD_ELF_SHDR *pstScnHdrTbl);

/* symtblLib */
extern UINT32 LD_CommonManage(DYN_MODULE_INFO *pstModuleHandle, AARCHUINT uwComAreaSize, AARCHUINT swComAlignment, CHAR *pscSymName,
        VOID **ppSymAddr, INT8 *pscSymType);

/* moduleLib */
extern DYN_MODULE_INFO *LD_ModuleHandleAlloc(CHAR *pscElfFileName);
extern INT32 LD_ModuleMemRelease(DYN_MODULE_INFO *pstModuleHandle);
extern INT32 LD_ModuleDeinit(DYN_MODULE_INFO *pstModuleHandle);
extern INT32 LD_ModuleUnload(VOID *pstHandle);
extern INT32 LD_DlLoadAll(DYN_MODULE_INFO *pstModuleHandle, LOAD_FUNC pfnLoadFunc);
extern INT32 LD_DlRelocAll(RELOC_FUNC pfnRelocFunc);
extern INT32 LD_ObjLoad(DL_INFO *pstDlInfo);

/* symLib */
extern UINT32 LD_SymAdd(DYN_MODULE_INFO *pstModuleHandle, CHAR *pscName, INT8 *pwAddr, INT8 stType);
extern INT32 LD_SymRemoveByModuleHandle(DYN_MODULE_INFO *pstModuleHandle);

extern UINT32 LD_ObjARMReloc(DL_INFO *pstDlInfo, LD_ELF_SHDR *pstRelHdr);
extern INT32 LD_ObjReloc(DL_INFO *pstDlInfo);

extern INT32 LD_DlSegReloc(DL_INFO *pstDlInfo);
extern VOID *LD_FindSymInDepended(VOID *pHandle, CHAR *pscName);

extern UINT32 LD_SymLoadAddrGet(DL_INFO *pstDlInfo, UINT32 uwSymIndex, AARCHUINT *puwLoadAddr);
extern VOID LD_FlushICache(VOID);
extern VOID LD_FlushDCache(AARCHUINT swStart, AARCHUINT swEnd);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* _KERNEL_LOS_LD_TYPE_INC */
