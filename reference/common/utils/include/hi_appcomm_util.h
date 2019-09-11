/**
 * @file    hi_appcomm_util.h
 * @brief   application common misc utils interface.
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/18
 * @version   1.0

 */
#ifndef __HI_APPCOMM_UTIL_H__
#define __HI_APPCOMM_UTIL_H__

#ifdef __HuaweiLite__
#include "asm/io.h"
#endif
#include "hi_appcomm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/** \addtogroup     APPCOMM */
/** @{ */  /** <!-- [APPCOMM] */


/**-------------------------Linux Specified Interface------------------------- */
/**
 * @brief    check specified path is directory or not.
 * @param[in] pszPath:directory path
 * @return 1:directory 0:file -1:not exist.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/19
 */
HI_S32 HI_PathIsDirectory(const HI_CHAR* pszPath);

/**
 * @brief    delete specified directory.
 * @param[in] pszPath:directory path
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/19
 */
HI_S32 HI_rmdir(const HI_CHAR* pszPath);

/**
 * @brief    create specified directory.
 * @param[in] pszPath:directory path
 * @param[in] mode:create mode
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/19
 */
HI_S32 HI_mkdir(const HI_CHAR* pszPath, mode_t mode);

HI_S32 HI_system(const HI_CHAR* pszCmd);

HI_S32 HI_du(const HI_CHAR* pszPath, HI_U64* pu64Size_KB);

HI_VOID* HI_MemMap(HI_U64 u64PhyAddr, HI_U32 u32Size);

HI_S32 HI_MemUnmap(HI_VOID* pvMappedAddr);

HI_S32 HI_insmod(const HI_CHAR* pszPath, const HI_CHAR* pszOptions);

HI_S32 HI_rmmod(const HI_CHAR* pszPath);

HI_S32 HI_async(HI_VOID);

HI_S32 HI_fsync(const HI_CHAR * pszPath);

#ifdef __LITEOS__
#define himm(address, value)  writel(value, address)
#define himd(address, pvalue) do{*(pvalue) = readl(address);}while(0)
#else
HI_S32 himm(HI_U32 u32Addr, HI_U32 u32Value);
HI_S32 himd(HI_U32 u32Addr, HI_U32* pu32Value);
#endif
static inline HI_VOID writereg(HI_U32 value, HI_U32 mask, HI_U32 addr)
{
    HI_U32 t;
    himd(addr,&t);
    t &= ~mask;
    t |= value & mask;
    himm(addr,t);
}

/**-------------------------Common Interface------------------------- */

/** @}*/  /** <!-- ==== APPCOMM End ====*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __HI_APPCOMM_UTIL_H__ */

