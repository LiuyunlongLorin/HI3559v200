#ifndef JFFS2_BASETYPE_H
#define JFFS2_BASETYPE_H

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */


#ifdef __ARMEB__
# define JFFS2_BYTEORDER           JFFS2_MSBFIRST    // Big endian
#else
# define JFFS2_BYTEORDER           JFFS2_LSBFIRST    // Little endian
#endif

#if defined(__ARMEL__) && defined(__VFP_FP__)
# define JFFS2_DOUBLE_BYTEORDER    JFFS2_LSBFIRST
#else
# define JFFS2_DOUBLE_BYTEORDER    JFFS2_MSBFIRST    // Big? endian
#endif

//-----------------------------------------------------------------------------
// ARM does not usually use labels with underscores.

#define JFFS2_LABEL_NAME(_name_) _name_
#define JFFS2_LABEL_DEFN(_name_) _name_

//-----------------------------------------------------------------------------
// Override the alignment definitions from jffs2_type.h. ARM only allows 4
// byte alignment whereas the default is 8 byte.

#define JFFS2ARC_ALIGNMENT 4
#define JFFS2ARC_P2ALIGNMENT 2

//-----------------------------------------------------------------------------
// Define the standard variable sizes

// The ARM architecture uses the default definitions of the base types,
// so we do not need to define any here.

//-----------------------------------------------------------------------------

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif // JFFS2ONCE_HAL_BASETYPE_H
// End of basetype.h
