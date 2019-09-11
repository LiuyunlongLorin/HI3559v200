#include "los_typedef.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#ifndef LOS_HAL_TABLE_WOW_BEGIN
#define LOS_HAL_TABLE_WOW_BEGIN( _label, _name )                                 \
__asm__(".section \".liteos.table." __xstring(_name) ".wow.begin\",\"aw\"\n"       \
    ".globl " __xstring(LOS_LABEL_DEFN(_label)) "\n"                         \
    ".type    " __xstring(LOS_LABEL_DEFN(_label)) ",object\n"                \
    ".p2align " __xstring(LOSARC_P2ALIGNMENT) "\n"                           \
__xstring(LOS_LABEL_DEFN(_label)) ":\n"                                      \
    ".previous\n"                                                            \
       )
#endif

#ifndef LOS_HAL_TABLE_WOW_END
#define LOS_HAL_TABLE_WOW_END( _label, _name )                                   \
__asm__(".section \".liteos.table." __xstring(_name) ".wow.finish\",\"aw\"\n"      \
    ".globl " __xstring(LOS_LABEL_DEFN(_label)) "\n"                         \
    ".type    " __xstring(LOS_LABEL_DEFN(_label)) ",object\n"                \
    ".p2align " __xstring(LOSARC_P2ALIGNMENT) "\n"                           \
__xstring(LOS_LABEL_DEFN(_label)) ":\n"                                      \
    ".previous\n"                                                            \
       )
#endif

#ifndef LOS_HAL_TABLE_SCATTER_BEGIN
#define LOS_HAL_TABLE_SCATTER_BEGIN( _label, _name )                                 \
__asm__(".section \".liteos.table." __xstring(_name) ".scatter.begin\",\"aw\"\n"       \
    ".globl " __xstring(LOS_LABEL_DEFN(_label)) "\n"                         \
    ".type    " __xstring(LOS_LABEL_DEFN(_label)) ",object\n"                \
    ".p2align " __xstring(LOSARC_P2ALIGNMENT) "\n"                           \
__xstring(LOS_LABEL_DEFN(_label)) ":\n"                                      \
    ".previous\n"                                                            \
       )
#endif

#ifndef LOS_HAL_TABLE_SCATTER_END
#define LOS_HAL_TABLE_SCATTER_END( _label, _name )                                   \
__asm__(".section \".liteos.table." __xstring(_name) ".scatter.finish\",\"aw\"\n"      \
    ".globl " __xstring(LOS_LABEL_DEFN(_label)) "\n"                         \
    ".type    " __xstring(LOS_LABEL_DEFN(_label)) ",object\n"                \
    ".p2align " __xstring(LOSARC_P2ALIGNMENT) "\n"                           \
__xstring(LOS_LABEL_DEFN(_label)) ":\n"                                      \
    ".previous\n"                                                            \
       )
#endif

#ifndef LOS_HAL_TABLE_BEGIN
#define LOS_HAL_TABLE_BEGIN( _label, _name )                                 \
__asm__(".section \".liteos.table." __xstring(_name) ".begin\",\"aw\"\n"       \
    ".globl " __xstring(LOS_LABEL_DEFN(_label)) "\n"                         \
    ".type    " __xstring(LOS_LABEL_DEFN(_label)) ",object\n"                \
    ".p2align " __xstring(LOSARC_P2ALIGNMENT) "\n"                           \
__xstring(LOS_LABEL_DEFN(_label)) ":\n"                                      \
    ".previous\n"                                                            \
       )
#endif

#ifndef LOS_HAL_TABLE_END
#define LOS_HAL_TABLE_END( _label, _name )                                   \
__asm__(".section \".liteos.table." __xstring(_name) ".finish\",\"aw\"\n"      \
    ".globl " __xstring(LOS_LABEL_DEFN(_label)) "\n"                         \
    ".type    " __xstring(LOS_LABEL_DEFN(_label)) ",object\n"                \
    ".p2align " __xstring(LOSARC_P2ALIGNMENT) "\n"                           \
__xstring(LOS_LABEL_DEFN(_label)) ":\n"                                      \
    ".previous\n"                                                            \
       )
#endif

// This macro must be applied to any types whose objects are to be placed in
// tables
#ifndef LOS_HAL_TABLE_TYPE
#define LOS_HAL_TABLE_TYPE LOSBLD_ATTRIB_ALIGN( LOSARC_ALIGNMENT )
#endif

#ifndef LOS_HAL_TABLE_EXTRA
#define LOS_HAL_TABLE_EXTRA( _name ) \
        LOSBLD_ATTRIB_SECTION(".liteos.table." __xstring(_name) ".extra")
#endif

#ifndef LOS_HAL_TABLE_WOW_ENTRY
#define LOS_HAL_TABLE_WOW_ENTRY( _name ) \
        LOSBLD_ATTRIB_SECTION(".liteos.table." __xstring(_name) ".wow.data") \
        LOSBLD_ATTRIB_USED
#endif

#ifndef LOS_HAL_TABLE_SCATTER_ENTRY
#define LOS_HAL_TABLE_SCATTER_ENTRY( _name ) \
        LOSBLD_ATTRIB_SECTION(".liteos.table." __xstring(_name) ".scatter.data") \
        LOSBLD_ATTRIB_USED
#endif

#ifndef LOS_HAL_TABLE_ENTRY
#define LOS_HAL_TABLE_ENTRY( _name ) \
        LOSBLD_ATTRIB_SECTION(".liteos.table." __xstring(_name) ".data") \
        LOSBLD_ATTRIB_USED
#endif

#ifndef LOS_HAL_TABLE_QUALIFIED_ENTRY
#define LOS_HAL_TABLE_QUALIFIED_ENTRY( _name, _qual ) \
        LOSBLD_ATTRIB_SECTION(".liteos.table." __xstring(_name) ".data." \
                              __xstring(_qual))                        \
        LOSBLD_ATTRIB_USED
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
