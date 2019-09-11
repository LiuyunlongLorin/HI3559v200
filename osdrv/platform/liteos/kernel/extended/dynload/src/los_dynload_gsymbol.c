#include "los_elf_symbol.inc"


#define SYMBOL_ENTRY(_l, _symbol) \
extern void _symbol(void); \
GDSYMBOL _l LOS_HAL_TABLE_ENTRY(dynload_gsymbol) = \
{ \
    #_symbol, \
    (AARCHPTR)_symbol \
};

SYMBOL_ENTRY(AACDecodeFindSyncHeader_symbol, AACDecodeFindSyncHeader)
SYMBOL_ENTRY(AACDecodeFrame_symbol, AACDecodeFrame)
SYMBOL_ENTRY(AACFreeDecoder_symbol, AACFreeDecoder)
SYMBOL_ENTRY(AACGetLastFrameInfo_symbol, AACGetLastFrameInfo)
SYMBOL_ENTRY(AACInitDecoder_symbol, AACInitDecoder)
SYMBOL_ENTRY(LOS_FindSymByName_symbol, LOS_FindSymByName)
SYMBOL_ENTRY(LOS_ModuleUnload_symbol, LOS_ModuleUnload)
SYMBOL_ENTRY(LOS_SoLoad_symbol, LOS_SoLoad)
SYMBOL_ENTRY(__aeabi_idivmod_symbol, __aeabi_idivmod)
SYMBOL_ENTRY(__aeabi_lasr_symbol, __aeabi_lasr)
SYMBOL_ENTRY(__aeabi_ldivmod_symbol, __aeabi_ldivmod)
SYMBOL_ENTRY(__aeabi_memclr4_symbol, __aeabi_memclr4)
SYMBOL_ENTRY(__aeabi_unwind_cpp_pr0_symbol, __aeabi_unwind_cpp_pr0)
SYMBOL_ENTRY(__stack_chk_fail_symbol, __stack_chk_fail)
SYMBOL_ENTRY(__stack_chk_guard_symbol, __stack_chk_guard)
SYMBOL_ENTRY(calloc_symbol, calloc)
SYMBOL_ENTRY(close_symbol, close)
SYMBOL_ENTRY(fprintf_symbol, fprintf)
SYMBOL_ENTRY(free_symbol, free)
SYMBOL_ENTRY(malloc_symbol, malloc)
SYMBOL_ENTRY(memcpy_s_symbol, memcpy_s)
SYMBOL_ENTRY(memmove_s_symbol, memmove_s)
SYMBOL_ENTRY(memset_symbol, memset)
SYMBOL_ENTRY(memset_s_symbol, memset_s)
SYMBOL_ENTRY(mmap_symbol, mmap)
SYMBOL_ENTRY(munmap_symbol, munmap)
SYMBOL_ENTRY(open_symbol, open)
SYMBOL_ENTRY(printf_symbol, printf)
SYMBOL_ENTRY(pthread_mutex_lock_symbol, pthread_mutex_lock)
SYMBOL_ENTRY(pthread_mutex_unlock_symbol, pthread_mutex_unlock)
SYMBOL_ENTRY(stderr_symbol, stderr)
