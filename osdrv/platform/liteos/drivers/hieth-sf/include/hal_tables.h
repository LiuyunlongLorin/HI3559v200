#ifndef LOSONCE_HAL_TABLES_H
#define LOSONCE_HAL_TABLES_H

#include "los_typedef.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/*------------------------------------------------------------------------*/

#define HAL_LABEL_DEFN(_label) _label
#define HALARC_P2ALIGNMENT 3

// This macro must be applied to any types whose objects are to be placed in
// tables

#define HALBLD_ATTRIB_ALIGN(__align__) __attribute__((aligned(__align__)))
#define HALBLD_ATTRIB_SECTION(__sect__) __attribute__((section (__sect__)))
#define HALARC_ALIGNMENT 8

#define HALBLD_ATTRIB_USED __attribute__((used))
//#define HALBLD_ATTRIB_USED

#ifndef DEV_HAL_TABLE_TYPE
#define DEV_HAL_TABLE_TYPE HALBLD_ATTRIB_ALIGN( HALARC_ALIGNMENT )
#endif

#ifndef DEV_HAL_TABLE_ENTRY
#define DEV_HAL_TABLE_ENTRY( _name ) \
        HALBLD_ATTRIB_SECTION(".liteos.table." __xstring(_name) ".data") \
        HALBLD_ATTRIB_USED
#endif

/*------------------------------------------------------------------------*/
/* EOF hal_tables.h                                                       */

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
#endif // HALONCE_HAL_TABLES_H
