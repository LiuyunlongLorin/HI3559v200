#ifndef JFFS2ONCE_TYPE_H
#define JFFS2ONCE_TYPE_H
//=============================================================================
//
//      fileio.h
//
//      Fileio header
//
//=============================================================================
//*   Copyright (c) <2014-2017>, <Huawei Technologies Co., Ltd>
//*   All rights reserved.
// ####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Free Software Foundation, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later
// version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License
// along with eCos; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
// As a special exception, if other files instantiate templates or use
// macros or inline functions from this file, or you compile this file
// and link it with other works to produce a work based on this file,
// this file does not by itself cause the resulting work to be covered by
// the GNU General Public License. However the source code for this file
// must still be made available in accordance with section (3) of the GNU
// General Public License v2.
//
// This exception does not invalidate any other reasons why a work based
// on this file might be covered by the GNU General Public License.
// -------------------------------------------
// ####ECOSGPLCOPYRIGHTEND####
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     nickg
// Contributors:  nickg
// Date:          2000-05-25
// Purpose:       Fileio header
// Description:   This header contains the external definitions of the general file
//                IO subsystem for POSIX and EL/IX compatability.
//
// Usage:
//              #include <fileio.h>
//              ...
//
//
//####DESCRIPTIONEND####
/****************************************************************************
 * Notice of Export Control Law
 * ===============================================
 * Huawei LiteOS may be subject to applicable export control laws and regulations,
 * which might include those applicable to Huawei LiteOS of U.S. and the country in
 * which you are located.
 * Import, export and usage of Huawei LiteOS in any manner by you shall be in
 * compliance with such applicable export control laws and regulations.
 ****************************************************************************/
//=============================================================================

#include <stddef.h>           // Definition of NULL from the compiler

// -------------------------------------------------------------------------
// Some useful macros. These are defined here by default.

// __externC is used in mixed C/C++ headers to force C linkage on an external
// definition. It avoids having to put all sorts of ifdefs in.

#ifdef __cplusplus
# define __externC extern "C"
#else
# define __externC extern
#endif
// Also define externC for now - but it is deprecated
#ifdef externC
#undef externC
#endif
#define externC __externC

// Compiler version.
#ifdef __GNUC__
# if defined(__GNU_PATCHLEVEL__)
#  define __GNUC_VERSION__ (__GNUC__ * 10000 \
                             + __GNUC_MINOR__ * 100 \
                             + __GNUC_PATCHLEVEL__)
# else
#  define __GNUC_VERSION__ (__GNUC__ * 10000 \
                             + __GNUC_MINOR__ * 100)
# endif
#endif

// -------------------------------------------------------------------------
// The header <basetype.h> defines the base types used here. It is
// supplied either by the target architecture HAL, or by the host
// porting kit. They are all defined as macros, and only those that
// make choices other than the defaults given below need be defined.

#define JFFS2_LSBFIRST 1234
#define JFFS2_MSBFIRST 4321

#include "basetype.h"

#if (JFFS2_BYTEORDER != JFFS2_LSBFIRST) && (JFFS2_BYTEORDER != JFFS2_MSBFIRST)
# error You must define JFFS2_BYTEORDER to equal JFFS2_LSBFIRST or JFFS2_MSBFIRST
#endif

#ifndef JFFS2_DOUBLE_BYTEORDER
#define JFFS2_DOUBLE_BYTEORDER JFFS2_BYTEORDER
#endif

#ifndef jffs2_halint8
# define jffs2_halint8 char
#endif
#ifndef jffs2_halint16
# define jffs2_halint16 short
#endif
#ifndef jffs2_halint32
# define jffs2_halint32 int
#endif
#ifndef jffs2_halint64
# define jffs2_halint64 long long
#endif

#ifndef jffs2_halcount8
# define jffs2_halcount8 int
#endif
#ifndef jffs2_halcount16
# define jffs2_halcount16 int
#endif
#ifndef jffs2_halcount32
# define jffs2_halcount32 int
#endif
#ifndef jffs2_halcount64
# define jffs2_halcount64 long long
#endif

#ifndef jffs2_haladdress
# define jffs2_haladdress AARCHPTR
#endif
#ifndef jffs2_haladdrword
# define jffs2_haladdrword AARCHPTR
#endif

#ifndef jffs2_halbool
# define jffs2_halbool int
#endif

#ifndef jffs2_halatomic
# define jffs2_halatomic jffs2_halint8
#endif

// -------------------------------------------------------------------------
// Provide a default architecture alignment
// This may be overridden in basetype.h if necessary.
// These should be straightforward numbers to allow use in assembly.

#ifndef JFFS2ARC_ALIGNMENT
# define JFFS2ARC_ALIGNMENT 8
#endif
// And corresponding power of two alignment
#ifndef JFFS2ARC_P2ALIGNMENT
# define JFFS2ARC_P2ALIGNMENT 3
#endif
#if (JFFS2ARC_ALIGNMENT) != (1 << JFFS2ARC_P2ALIGNMENT)
# error "Inconsistent JFFS2ARC_ALIGNMENT and JFFS2ARC_P2ALIGNMENT values"
#endif

// -------------------------------------------------------------------------
// The obvious few that compilers may define for you.
// But in case they don't:

#ifndef NULL
# define NULL 0
#endif

// -------------------------------------------------------------------------
// Allow creation of procedure-like macros that are a single statement,
// and must be followed by a semi-colon

#define JFFS2_MACRO_START do {
#define JFFS2_MACRO_END   } while (0)

#define JFFS2_EMPTY_STATEMENT JFFS2_MACRO_START JFFS2_MACRO_END

#define JFFS2_UNUSED_PARAM( _type_, _name_ ) JFFS2_MACRO_START      \
  _type_ __tmp1 = (_name_);                                     \
  _type_ __tmp2 = __tmp1;                                       \
  __tmp1 = __tmp2;                                              \
JFFS2_MACRO_END


//----------------------------------------------------------------------------
// The unused attribute stops the compiler warning about the variable
// not being used.
// The used attribute prevents the compiler from optimizing it away.

#define JFFS2_REFERENCE_OBJECT(__object__)                            \
    JFFS2_MACRO_START                                                 \
    static const void*  __jffs2var_discard_me__                       \
    __attribute__ ((unused, used)) = (const void*)&(__object__);    \
    JFFS2_MACRO_END

// -------------------------------------------------------------------------
// Define basic types for using integers in memory and structures;
// depends on compiler defaults and CPU type.

typedef unsigned jffs2_halint8    jffs2_uint8  ;
typedef   signed jffs2_halint8    jffs2_int8   ;

typedef unsigned jffs2_halint16   jffs2_uint16 ;
typedef   signed jffs2_halint16   jffs2_int16  ;

typedef unsigned jffs2_halint32   jffs2_uint32 ;
typedef   signed jffs2_halint32   jffs2_int32  ;

typedef unsigned jffs2_halint64   jffs2_uint64 ;
typedef   signed jffs2_halint64   jffs2_int64  ;

//typedef  jffs2_halbool            jffs2_bool   ;

// -------------------------------------------------------------------------
// Define types for using integers in registers for looping and the like;
// depends on CPU type, choose what it is most comfortable with, with at
// least the range required.

typedef unsigned jffs2_halcount8  jffs2_ucount8  ;
typedef   signed jffs2_halcount8  jffs2_count8   ;

typedef unsigned jffs2_halcount16 jffs2_ucount16 ;
typedef   signed jffs2_halcount16 jffs2_count16  ;

typedef unsigned jffs2_halcount32 jffs2_ucount32 ;
typedef   signed jffs2_halcount32 jffs2_count32  ;

typedef unsigned jffs2_halcount64 jffs2_ucount64 ;
typedef   signed jffs2_halcount64 jffs2_count64  ;

// -------------------------------------------------------------------------
// Define a type to be used for atomic accesses. This type is guaranteed
// to be read or written in a single uninterruptible operation. This type
// is at least a single byte.

typedef volatile unsigned jffs2_halatomic  jffs2_atomic;
typedef volatile unsigned jffs2_halatomic  JFFS2_ATOMIC;

// -------------------------------------------------------------------------
// Define types for access plain, on-the-metal memory or devices.

typedef jffs2_uint32  JFFS2_WORD;
typedef jffs2_uint8   JFFS2_BYTE;
typedef jffs2_uint16  JFFS2_WORD16;
typedef jffs2_uint32  JFFS2_WORD32;
typedef jffs2_uint64  JFFS2_WORD64;

typedef jffs2_haladdress  JFFS2_ADDRESS;
typedef jffs2_haladdrword JFFS2_ADDRWORD;

// -------------------------------------------------------------------------
// Number of elements in a (statically allocated) array.

#define JFFS2_NELEM(a) (sizeof(a) / sizeof((a)[0]))

// -------------------------------------------------------------------------
// Constructor ordering macros.  These are added as annotations to all
// static objects to order the constuctors appropriately.

#if defined(__cplusplus) && defined(__GNUC__) && \
    !defined(JFFS2BLD_ATTRIB_INIT_PRI)
# define JFFS2BLD_ATTRIB_INIT_PRI( _pri_ ) __attribute__((init_priority(_pri_)))
#elif !defined(JFFS2BLD_ATTRIB_INIT_PRI)
// FIXME: should maybe just bomb out if this is attempted anywhere else?
// Not sure
# define JFFS2BLD_ATTRIB_INIT_PRI( _pri_ )
#endif

// The following will be removed eventually as it doesn't allow the use of
// e.g. pri+5 format
#define JFFS2_INIT_PRIORITY( _pri_ ) JFFS2BLD_ATTRIB_INIT_PRI( JFFS2_INIT_##_pri_ )

#define JFFS2BLD_ATTRIB_INIT_BEFORE( _pri_ ) JFFS2BLD_ATTRIB_INIT_PRI(_pri_-100)
#define JFFS2BLD_ATTRIB_INIT_AFTER( _pri_ )  JFFS2BLD_ATTRIB_INIT_PRI(_pri_+100)

#if defined(__GNUC__) && !defined(__cplusplus) && (__GNUC_VERSION__ >= 40300) /*lint !e553*/
// Equivalents of the above for C functions, available from gcc 4.3 onwards.
# define JFFS2BLD_ATTRIB_C_INIT_PRI( _pri_)       __attribute__((constructor (_pri_)))
# define JFFS2BLD_ATTRIB_C_INIT_BEFORE( _pri_ )   __attribute__((constructor (_pri_-100)))
# define JFFS2BLD_ATTRIB_C_INIT_AFTER( _pri_ )    __attribute__((constructor (_pri_+100)))
#endif

// Start with initializing everything inside the cpu and the main memory.
#define JFFS2_INIT_HAL                    10000
#define JFFS2_INIT_SCHEDULER              11000
#define JFFS2_INIT_IDLE_THREAD            11100
#define JFFS2_INIT_INTERRUPTS             12000
#define JFFS2_INIT_CLOCK                  14000
#define JFFS2_INIT_THREADS                16000
#define JFFS2_INIT_KERNEL                 19000
#define JFFS2_INIT_MEMALLOC               20000
// Now move on to I/O subsystems and device drivers. These can make use of
// kernel and HAL functionality, and can dynamically allocate memory if
// absolutely needed. For now they can also assume that diag_printf()
// functionality is available, but that may change in future.
//
// Primary buses are ones very closely tied to the processor, e.g. PCI.
#define JFFS2_INIT_BUS_PRIMARY            30000
// Not yet: on some targets jffs2_pci_init() has to be called very early
// on for HAL diagnostics to work.
// #define JFFS2_INIT_BUS_PCI                JFFS2_INIT_BUS_PRIMARY
//
// Secondary buses may hang off primary buses, e.g. USB host.
#define JFFS2_INIT_BUS_SECONDARY          31000
// Tertiary buses are everything else.
#define JFFS2_INIT_BUS_TERTIARY           32000
#define JFFS2_INIT_BUS_I2C                JFFS2_INIT_BUS_TERTIARY
#define JFFS2_INIT_BUS_SPI                JFFS2_INIT_BUS_TERTIARY
//
// In future HAL diag initialization may happen at this point.
//
// Watchdogs and wallclocks often hang off a tertiary bus but
// have no dependencies
#define JFFS2_INIT_DEV_WATCHDOG           35000
#define JFFS2_INIT_DEV_WALLCLOCK          36000
// A primary block configuration can be initialized with no need
// for per-unit configuration information.
#define JFFS2_INIT_DEV_BLOCK_PRIMARY      37000
#define JFFS2_INIT_DEV_FLASH              JFFS2_INIT_DEV_BLOCK_PRIMARY
// Per-unit configuration data extracted from primary storage.
// NOTE: for future use, not implemented yet.
#define JFFS2_INIT_CONFIG                 38000
// Secondary block devices may use per-unit configuration data
// for e.g. interpreting partition layout. Few devices are expected
// to fall into this category. Note that these devices, as well as
// some char devices, may not actually be usable until interrupts
// are enabled.
#define JFFS2_INIT_DEV_BLOCK_SECONDARY    40000
// Char devices are everything else: serial, ethernet, CAN, ...
#define JFFS2_INIT_DEV_CHAR               41000
// For backwards compatibility. Subject to change in future so
// a JFFS2_INIT_DEV_ priority should be used instead.
#define JFFS2_INIT_DRIVERS                48000
// JFFS2_INIT_IO and JFFS2_INIT_IO_FS are poorly defined at present,
// and may get reorganized in future.
#define JFFS2_INIT_IO                     49000
#define JFFS2_INIT_IO_FS                  50000
// The I/O subsystems and device drivers have been initialized.
#define JFFS2_INIT_LIBC                   56000
#define JFFS2_INIT_COMPAT                 58000
#define JFFS2_INIT_APPLICATION            60000
#define JFFS2_INIT_PREDEFAULT             65534
#define JFFS2_INIT_DEFAULT                65535

// -------------------------------------------------------------------------
// Label name macros. Some toolsets generate labels with initial
// underscores and others don't. JFFS2_LABEL_NAME should be used on
// labels in C/C++ code that are defined in assembly code or linker
// scripts. JFFS2_LABEL_DEFN is for use in assembly code and linker
// scripts where we need to manufacture labels that can be used from
// C/C++.
// These are default implementations that should work for most targets.
// They may be overridden in basetype.h if necessary.

#ifndef JFFS2_LABEL_NAME

#define JFFS2_LABEL_NAME(_name_) _name_

#endif

#ifndef JFFS2_LABEL_DEFN

#define JFFS2_LABEL_DEFN(_label) _label

#endif

// -------------------------------------------------------------------------
// COMPILER-SPECIFIC STUFF

#ifdef __GNUC__
// Force a 'C' routine to be called like a 'C++' contructor
# if !defined(JFFS2BLD_ATTRIB_CONSTRUCTOR)
#  define JFFS2BLD_ATTRIB_CONSTRUCTOR __attribute__((constructor))
# endif

// Define a compiler-specific rune for saying a function doesn't return
# if !defined(JFFS2BLD_ATTRIB_NORET)
#  define JFFS2BLD_ATTRIB_NORET __attribute__((noreturn))
# endif

// How to define weak symbols - this is only relevant for ELF and a.out
# if !defined(JFFS2BLD_ATTRIB_WEAK)
#  define JFFS2BLD_ATTRIB_WEAK __attribute__ ((weak))
# endif

// How to define alias to symbols. Just pass in the symbol itself, not
// the string name of the symbol
# if !defined(JFFS2BLD_ATTRIB_ALIAS)
#  define JFFS2BLD_ATTRIB_ALIAS(__symbol__) \
        __attribute__ ((alias (#__symbol__)))
# endif

// This effectively does the reverse of the previous macro. It defines
// a name that the attributed variable or function will actually have
// in assembler.
# if !defined(JFFS2BLD_ATTRIB_ASM_ALIAS)
#  define __Str(x) #x
#  define __Xstr(x) __Str(x)
#  define JFFS2BLD_ATTRIB_ASM_ALIAS(__symbol__) \
             __asm__ ( __Xstr( JFFS2_LABEL_DEFN( __symbol__ ) ) )
# endif

// Shows that a function returns the same value when given the same args, but
// note this can't be used if there are pointer args
# if !defined(JFFS2BLD_ATTRIB_CONST)
#  define JFFS2BLD_ATTRIB_CONST __attribute__((const))
#endif

// Assign a defined variable to a specific section
# if !defined(JFFS2BLD_ATTRIB_SECTION)
#  define JFFS2BLD_ATTRIB_SECTION(__sect__) __attribute__((section (__sect__)))
# endif

// Give a type or object explicit minimum alignment
# if !defined(JFFS2BLD_ATTRIB_ALIGN)
#  define JFFS2BLD_ATTRIB_ALIGN(__align__) __attribute__((aligned(__align__)))
# endif

# if !defined(JFFS2BLD_ATTRIB_ALIGN_MAX)
#  define JFFS2BLD_ATTRIB_ALIGN_MAX __attribute__((aligned))
# endif

# if !defined(JFFS2BLD_ATTRIB_ALIGNOFTYPE)
#  define JFFS2BLD_ATTRIB_ALIGNOFTYPE( _type_ ) \
     __attribute__((aligned(__alignof__( _type_ ))))
# endif

// Teach compiler how to check format of printf-like functions
# define JFFS2BLD_ATTRIB_PRINTF_FORMAT(__format__, __args__) \
        __attribute__((format (printf, __format__, __args__)))

// Teach compiler how to check format of scanf-like functions
# define JFFS2BLD_ATTRIB_SCANF_FORMAT(__format__, __args__) \
        __attribute__((format (scanf, __format__, __args__)))

// Teach compiler how to check format of strftime-like functions
# define JFFS2BLD_ATTRIB_STRFTIME_FORMAT(__format__, __args__) \
        __attribute__((format (strftime, __format__, __args__)))

// Tell the compiler not to throw away a variable or function. Only known
// available on 3.3.2 or above. Old version's didn't throw them away,
// but using the unused attribute should stop warnings.
# if !defined(JFFS2BLD_ATTRIB_USED)
#  if __GNUC_VERSION__ >= 30302/*lint !e553*/
#   define JFFS2BLD_ATTRIB_USED __attribute__((used))
#  else
#   define JFFS2BLD_ATTRIB_USED __attribute__((unused))
#  endif
# endif
#else // non-GNU

# define JFFS2BLD_ATTRIB_CONSTRUCTOR

# define JFFS2BLD_ATTRIB_NORET
    // This intentionally gives an error only if we actually try to
    // use it.  #error would give an error if we simply can't.
// FIXME: Had to disarm the bomb - the JFFS2BLD_ATTRIB_WEAK macro is now
//        (indirectly) used in host tools.
# define JFFS2BLD_ATTRIB_WEAK /* !!!-- Attribute weak not defined --!!! */

# define JFFS2BLD_ATTRIB_ALIAS(__x__) !!!-- Attribute alias not defined --!!!

# define JFFS2BLD_ATTRIB_ASM_ALIAS(__symbol__) !!!-- Asm alias not defined --!!!

# define JFFS2BLD_ATTRIB_CONST

# define JFFS2BLD_ATTRIB_ALIGN(__align__) !!!-- Alignment alias not defined --!!!

# define JFFS2BLD_ATTRIB_ALIGN_MAX !!!-- Alignment alias not defined --!!!

# define JFFS2BLD_ATTRIB_ALIGNOFTYPE( _type_ ) !!!-- Alignment alias not defined --!!!

# define JFFS2BLD_ATTRIB_PRINTF_FORMAT(__format__, __args__)

# define JFFS2BLD_ATTRIB_SCANF_FORMAT(__format__, __args__)

# define JFFS2BLD_ATTRIB_STRFTIME_FORMAT(__format__, __args__)


#endif

// How to define weak aliases. Currently this is simply a mixture of the
// above

# define JFFS2BLD_ATTRIB_WEAK_ALIAS(__symbol__) \
        JFFS2BLD_ATTRIB_WEAK JFFS2BLD_ATTRIB_ALIAS(__symbol__)

#ifdef __cplusplus
# define __THROW throw()
#else
# define __THROW
#endif

// -------------------------------------------------------------------------
// Variable annotations
// These annotations may be added to various static variables in the
// HAL and kernel to indicate which component they belong to. These
// are used by some targets to optimize memory placement of these
// variables.

#ifndef JFFS2BLD_ANNOTATE_VARIABLE_HAL
#define JFFS2BLD_ANNOTATE_VARIABLE_HAL
#endif
#ifndef JFFS2BLD_ANNOTATE_VARIABLE_SCHED
#define JFFS2BLD_ANNOTATE_VARIABLE_SCHED
#endif
#ifndef JFFS2BLD_ANNOTATE_VARIABLE_CLOCK
#define JFFS2BLD_ANNOTATE_VARIABLE_CLOCK
#endif
#ifndef JFFS2BLD_ANNOTATE_VARIABLE_INTR
#define JFFS2BLD_ANNOTATE_VARIABLE_INTR
#endif

// -------------------------------------------------------------------------
// Various "flavours" of memory regions that can be described by the
// Memory Layout Tool (MLT).

#define JFFS2MEM_REGION_ATTR_R  0x01  // Region can be read
#define JFFS2MEM_REGION_ATTR_W  0x02  // Region can be written

// -------------------------------------------------------------------------
#endif // JFFS2ONCE_TYPE_H multiple inclusion protection
// EOF jffs2_type.h
