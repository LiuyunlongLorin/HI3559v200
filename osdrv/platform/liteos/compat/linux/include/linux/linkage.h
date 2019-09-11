#ifndef _COMPAT_LINUX_LINKAGE_H
#define _COMPAT_LINUX_LINKAGE_H

#include <linux/compiler.h>
#include <linux/stringify.h>
#include <asm/linkage.h>

#ifdef __cplusplus
#define COMPAT_CPP_ASMLINKAGE    extern "C"
#else
#define COMPAT_CPP_ASMLINKAGE
#endif

#ifndef asmlinkage
#define asmlinkage    COMPAT_CPP_ASMLINKAGE
#endif
#define CPP_ASMLINKAGE  COMPAT_CPP_ASMLINKAGE

#ifndef __ALIGN
#define __COMPAT_ALIGN_STR    ".align 4,0x90"
#define __COMPAT_ALIGN    .align 4,0x90
#define __ALIGN_STR   __COMPAT_ALIGN_STR
#define __ALIGN  __COMPAT_ALIGN
#endif

#ifdef __ASSEMBLY__

#define ALIGN_STR    __ALIGN_STR
#define ALIGN    __ALIGN

#ifndef END
#define END(var)    .size var, .-var
#endif

#ifndef ENDPROC
#define ENDPROC(var) \
  .type var, @function; \
  END(var)
#endif

#ifndef ENTRY
#define ENTRY(var) \
  .globl var; \
  ALIGN; \
  var:
#endif

#endif
#endif
