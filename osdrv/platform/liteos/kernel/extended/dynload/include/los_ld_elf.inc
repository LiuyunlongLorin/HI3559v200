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

#ifndef _KERNEL_LOS_LD_ELH_INC
#define _KERNEL_LOS_LD_ELH_INC

#include "los_typedef.ph"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */


/*
 * Elf header
 */
#define LD_EI_NIDENT    16

typedef struct
{
    UINT8       ucEIdent[LD_EI_NIDENT];     /* Magic number and other info */
    UINT16      usEType;                    /* Object file type */
    UINT16      usEMachine;                 /* Architecture */
    UINT32      uwEVersion;                 /* Object file version */
    UINT32      uwEEntry;                   /* Entry point virtual address */
    UINT32      uwEPhoff;                   /* Program header table file offset */
    UINT32      uwEShoff;                   /* Section header table file offset */
    UINT32      uwEFlags;                   /* Processor-specific flags */
    UINT16      usEEhsize;                  /* ELF header size in bytes */
    UINT16      usEPhentsize;               /* Program header table entry size */
    UINT16      usEPhnum;                   /* Program header table entry count */
    UINT16      usEShentsize;               /* Section header table entry size */
    UINT16      usEShnum;                   /* Section header table entry count */
    UINT16      usEShstrndx;                /* Section header string table index */
} LD_ELF32_EHDR;

typedef struct
{
    UINT8       ucEIdent[LD_EI_NIDENT];     /* Magic number and other info */
    UINT16      usEType;                    /* Object file type */
    UINT16      usEMachine;                 /* Architecture */
    UINT32      uwEVersion;                 /* Object file version */
    UINT64      uwEEntry;                   /* Entry point virtual address */
    UINT64      uwEPhoff;                   /* Program header table file offset */
    UINT64      uwEShoff;                   /* Section header table file offset */
    UINT32      uwEFlags;                   /* Processor-specific flags */
    UINT16      usEEhsize;                  /* ELF header size in bytes */
    UINT16      usEPhentsize;               /* Program header table entry size */
    UINT16      usEPhnum;                   /* Program header table entry count */
    UINT16      usEShentsize;               /* Section header table entry size */
    UINT16      usEShnum;                   /* Section header table entry count */
    UINT16      usEShstrndx;                /* Section header string table index */
} LD_ELF64_EHDR;

#ifdef LOSCFG_AARCH64
#define LD_ELF_EHDR LD_ELF64_EHDR
#else
#define LD_ELF_EHDR LD_ELF32_EHDR
#endif

/*
 * e_ident[] values
 */
#define LD_EI_MAG0      0
#define LD_EI_MAG1      1
#define LD_EI_MAG2      2
#define LD_EI_MAG3      3
#define LD_EI_CLASS     4
#define LD_EI_DATA      5
#define LD_EI_VERSION   6
#define LD_EI_PAD       7

#define LD_ELFMAG0      0x7f
#define LD_ELFMAG1      'E'
#define LD_ELFMAG2      'L'
#define LD_ELFMAG3      'F'
#define LD_ELFMAG       "\177ELF"
#define LD_SELFMAG      4

/*
 * EI_CLASS
 */
#define LD_ELF_CLASS_NONE   0
#define LD_ELF_CLASS32      1
#define LD_ELF_CLASS64      2

/*
 * EI_DATA
 */
#define LD_ELF_DATA_NONE    0
#define LD_ELF_DATA2LSB     1
#define LD_ELF_DATA2MSB     2

/*
 * e_type
 */
#define LD_ET_NONE      0
#define LD_ET_REL       1
#define LD_ET_EXEC      2
#define LD_ET_DYN       3
#define LD_ET_CORE      4
#define LD_ET_LOPROC    0xff00
#define LD_ET_HIPROC    0xffff

/*
 * e_machine
 */
#define LD_EM_NONE          0       /* No machine */
#define LD_EM_M32           1       /* AT&T WE 32100 */
#define LD_EM_SPARC         2       /* SPARC */
#define LD_EM_386           3       /* Intel 80386 */
#define LD_EM_68K           4       /* Motorola 68000 */
#define LD_EM_88K           5       /* Motorola 88000 */
#define LD_EM_486           6       /* Intel 80486 */
#define LD_EM_860           7       /* Intel 80860 */
#define LD_EM_MIPS          8       /* MIPS RS3000 Big-Endian */
#define LD_EM_MIPS_RS4_BE   10      /* MIPS RS4000 Big-Endian */
#define LD_EM_PPC_OLD       17      /* PowerPC - old */
#define LD_EM_PPC           20      /* PowerPC */
#define LD_EM_RCE_OLD       25      /* RCE - old */
#define LD_EM_RCE           39      /* RCE */
#define LD_EM_MCORE         39      /* MCORE */
#define LD_EM_SH            42      /* SH */
#define LD_EM_M32R          36929   /* M32R */
#define LD_EM_NEC           36992   /* NEC 850 series */
#define LD_EM_NEC_830       36      /* NEC 830 series */
#define LD_EM_SC            58      /* SC */
#define LD_EM_ARM           40      /* ARM  */
#define LD_EM_XTENSA        0x5E    /* XTENSA */
#define LD_EM_AARCH64       183     /* ARM AARCH64 */

/*
 * e_flags
 */
#define LD_EF_PPC_EMB           0x80000000

#define LD_EF_MIPS_NOREORDER    0x00000001
#define LD_EF_MIPS_PIC          0x00000002
#define LD_EF_MIPS_CPIC         0x00000004
#define LD_EF_MIPS_ARCH         0xf0000000
#define LD_EF_MIPS_ARCH_MIPS_2  0x10000000
#define LD_EF_MIPS_ARCH_MIPS_3  0x20000000

#define LD_PT_NULL      0
#define LD_PT_LOAD      1
#define LD_PT_DYNAMIC   2
#define LD_PT_INTERP    3
#define LD_PT_NOTE      4
#define LD_PT_SHLIB     5
#define LD_PT_PHDR      6

/*
 * e_version and EI_VERSION
 */
#define LD_EV_NONE          0
#define LD_EV_CURRENT       1

/*
 * Program Header
 */
typedef struct
{
    UINT32  uwType;                 /* Segment type */
    UINT32  uwOffset;               /* Segment file offset */
    UINT32  uwVaddr;                /* Segment virtual address */
    UINT32  uwPaddr;                /* Segment physical address */
    UINT32  uwFilesz;               /* Segment size in file */
    UINT32  uwMemsz;                /* Segment size in memory */
    UINT32  uwFlags;                /* Segment flags */
    UINT32  uwAlign;                /* Segment alignment */
} LD_ELF32_PHDR;

typedef struct
{
    UINT32  uwType;                 /* Segment type */
    UINT32  uwFlags;                /* Segment flags */
    UINT64  uwOffset;               /* Segment file offset */
    UINT64  uwVaddr;                /* Segment virtual address */
    UINT64  uwPaddr;                /* Segment physical address */
    UINT64  uwFilesz;               /* Segment size in file */
    UINT64  uwMemsz;                /* Segment size in memory */
    UINT64  uwAlign;                /* Segment alignment */
} LD_ELF64_PHDR;

#ifdef LOSCFG_AARCH64
#define LD_ELF_PHDR LD_ELF64_PHDR
#else
#define LD_ELF_PHDR LD_ELF32_PHDR
#endif

/*
 * Special section indexes
 */
#define LD_SHN_UNDEF        0
#define LD_SHN_LORESERVE    0xff00
#define LD_SHN_LOPROC       0xff00
#define LD_SHN_HIPROC       0xff1f
#define LD_SHN_ABS          0xfff1
#define LD_SHN_COMMON       0xfff2
#define LD_SHN_HIRESERVE    0xffff
#define LD_SHN_GHCOMMON     0xff00

/*
 * Section header
 */
typedef struct
{
  UINT32         uwShName;              /* Section name (string tbl index) */
  UINT32         uwShType;              /* Section type */
  UINT32         uwShFlags;             /* Section flags */
  UINT32         uwShAddr;              /* Section virtual addr at execution */
  UINT32         uwShOffset;            /* Section file offset */
  UINT32         uwShSize;              /* Section size in bytes */
  UINT32         uwShLink;              /* Link to another section */
  UINT32         uwShInfo;              /* Additional section information */
  UINT32         uwShAddrAlign;         /* Section alignment */
  UINT32         uwShEntSize;           /* Entry size if section holds table */
} LD_ELF32_SHDR;

typedef struct
{
  UINT32         uwShName;              /* Section name (string tbl index) */
  UINT32         uwShType;              /* Section type */
  UINT64         uwShFlags;             /* Section flags */
  UINT64         uwShAddr;              /* Section virtual addr at execution */
  UINT64         uwShOffset;            /* Section file offset */
  UINT64         uwShSize;              /* Section size in bytes */
  UINT32         uwShLink;              /* Link to another section */
  UINT32         uwShInfo;              /* Additional section information */
  UINT64         uwShAddrAlign;         /* Section alignment */
  UINT64         uwShEntSize;           /* Entry size if section holds table */
} LD_ELF64_SHDR;

#ifdef LOSCFG_AARCH64
#define LD_ELF_SHDR LD_ELF64_SHDR
#else
#define LD_ELF_SHDR LD_ELF32_SHDR
#endif

/*
 * sh_type
 */
#define LD_SHT_NULL     0
#define LD_SHT_PROGBITS 1
#define LD_SHT_SYMTAB   2
#define LD_SHT_STRTAB   3
#define LD_SHT_RELA     4
#define LD_SHT_HASH     5
#define LD_SHT_DYNAMIC  6
#define LD_SHT_NOTE     7
#define LD_SHT_NOBITS   8
#define LD_SHT_REL      9
#define LD_SHT_SHLIB    10
#define LD_SHT_DYNSYM   11
#define LD_SHT_COMDAT   12
#define LD_SHT_LOPROC   0x70000000
#define LD_SHT_HIPROC   0x7fffffff
#define LD_SHT_LOUSER   0x80000000
#define LD_SHT_HIUSER   0xffffffff

/*
 * sh_flags
 */
#define LD_SHF_WRITE        0x1
#define LD_SHF_ALLOC        0x2
#define LD_SHF_EXECINSTR    0x4
#define LD_SHF_MASKPROC     0xf0000000

/*
 * Symbol table
 */
typedef struct
{
    UINT32         uwStName;                /* Symbol name (string tbl index) */
    UINT32         uwStValue;               /* Symbol value */
    UINT32         uwStSize;                /* Symbol size */
    UINT8          ucStInfo;                /* Symbol type and binding */
    UINT8          ucStOther;               /* Symbol visibility */
    UINT16         usStShndx;               /* Section index */
} LD_ELF32_SYM;

typedef struct
{
    UINT32         uwStName;                /* Symbol name (string tbl index) */
    UINT8          ucStInfo;                /* Symbol type and binding */
    UINT8          ucStOther;               /* Symbol visibility */
    UINT16         usStShndx;               /* Section index */
    UINT64         uwStValue;               /* Symbol value */
    UINT64         uwStSize;                /* Symbol size */
} LD_ELF64_SYM;

#ifdef LOSCFG_AARCH64
#define LD_ELF_SYM LD_ELF64_SYM
#else
#define LD_ELF_SYM LD_ELF32_SYM
#endif

#define LD_STN_UNDEF    0

#define LD_STB_LOCAL    0
#define LD_STB_GLOBAL   1
#define LD_STB_WEAK     2
#define LD_STB_LOPROC   13
#define LD_STB_HIPROC   15

#define LD_STT_NOTYPE   0
#define LD_STT_OBJECT   1
#define LD_STT_FUNC     2
#define LD_STT_SECTION  3
#define LD_STT_FILE     4
#define LD_STT_LOPROC   13
#define LD_STT_HIPROC   15
#define LD_STT_THUMB    0x80

#define LD_ELF_ST_BIND(info)          ((info) >> 4)     /* Obtain the binding information of the symbol table */
#define LD_ELF_ST_TYPE(info)          ((info) & 0xf)    /* Obtain the type of the symbol table */
#define LD_ELF_ST_INFO(bind, scType)  (((bind) << 4) + ((scType) & 0xf))

/*
 * Dynamic
 */
typedef struct
{
    UINT32  uwDynTag;                 /* Dynamic entry type */
    union
    {
        UINT32 uwVal;                 /* Integer value */
        UINT32 uwPtr;                 /* Address value */
    } unDyn;
} LD_ELF32_DYN;

typedef struct
{
    UINT64  uwDynTag;                 /* Dynamic entry type */
    union
    {
        UINT64 uwVal;                 /* Integer value */
        UINT64 uwPtr;                 /* Address value */
    } unDyn;
} LD_ELF64_DYN;

#ifdef LOSCFG_AARCH64
#define LD_ELF_DYN LD_ELF64_DYN
#else
#define LD_ELF_DYN LD_ELF32_DYN
#endif

/* This is the info that is needed to parse the dynamic section of the file */
#define LD_DT_NULL         0
#define LD_DT_NEEDED       1
#define LD_DT_PLTRELSZ     2
#define LD_DT_PLTGOT       3
#define LD_DT_HASH         4
#define LD_DT_STRTAB       5
#define LD_DT_SYMTAB       6
#define LD_DT_RELA         7
#define LD_DT_RELASZ       8
#define LD_DT_RELAENT      9
#define LD_DT_STRSZ        10
#define LD_DT_SYMENT       11
#define LD_DT_INIT         12
#define LD_DT_FINI         13
#define LD_DT_SONAME       14
#define LD_DT_RPATH        15
#define LD_DT_SYMBOLIC     16
#define LD_DT_REL          17
#define LD_DT_RELSZ        18
#define LD_DT_RELENT       19
#define LD_DT_PLTREL       20
#define LD_DT_DEBUG        21
#define LD_DT_TEXTREL      22
#define LD_DT_JMPREL       23
#define LD_DT_ENCODING     32

/*
 * Relocation
 */
typedef struct
{
    UINT32        uwROffset;               /* Address */
    UINT32        uwRInfo;                 /* Relocation type and symbol index */
} LD_ELF32_REL;

typedef struct
{
    UINT32        uwROffset;               /* Address */
    UINT32        uwRInfo;                 /* Relocation type and symbol index */
    INT32         swRAddend;               /* Addend */
} LD_ELF32_RELA;

typedef struct
{
    UINT64        uwROffset;               /* Address */
    UINT64        uwRInfo;                 /* Relocation type and symbol index */
} LD_ELF64_REL;

typedef struct
{
    UINT64        uwROffset;               /* Address */
    UINT64        uwRInfo;                 /* Relocation type and symbol index */
    INT64         swRAddend;               /* Addend */
} LD_ELF64_RELA;

#ifdef LOSCFG_AARCH64
#define LD_ELF_REL  LD_ELF64_REL
#define LD_ELF_RELA LD_ELF64_RELA
#else
#define LD_ELF_REL  LD_ELF32_REL
#define LD_ELF_RELA LD_ELF32_RELA
#endif


#ifdef LOSCFG_AARCH64
#define LD_ELF_R_SYM(info)            ((info) >> 32)
#define LD_ELF_R_TYPE(info)           ((info) & 0xffffffff)
#define LD_ELF_R_INFO(sym,type)       ((((UINT64) (sym)) << 32) + (type))
#else
#define LD_ELF_R_SYM(info)            ((info) >> 8)
#define LD_ELF_R_TYPE(info)           (info & 0xff)
#define LD_ELF_R_INFO(sym, scType)    (((sym) << 8) + (UINT8)(scType))
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* _KERNEL_LOS_LD_ELH_INC */
