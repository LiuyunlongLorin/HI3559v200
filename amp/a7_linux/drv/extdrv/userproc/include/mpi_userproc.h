/******************************************************************************

  Copyright (C), 2016-2017, Hisilicon Tech. Co., Ltd.

******************************************************************************
  File Name     : mpi_userproc.h
  Version       : Initial Draft
  Author        : Hisilicon multimedia software group
  Created       : 2017/04/07
  Description   : Support user proc function.
  History       :
  1.Date        : 2017/04/07
    Author      : xxx
    Modification: Created file


******************************************************************************/

#ifndef __MPI_USERPROC_H__
#define __MPI_USERPROC_H__

/******************************* Include Files *******************************/

/* add include here */
#include "hi_type.h"
#include "hi_common_userproc.h"


#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif /* __cplusplus */

/***************************** Macro Definition ******************************/

/*************************** Structure Definition ****************************/

/********************** Global Variable declaration **************************/

/******************************* API declaration *****************************/

/**
@brief User mode proc is initialized. 
@attention You need insert module before calling this API. 
@param[in] None 
@retval ::HI_SUCCESS Success 
@retval ::HI_FAILURE Failure 
@see None
*/
HI_S32 HI_PROC_Init(HI_VOID);

/**
@brief User mode proc is deinitialized. 
@attention None. 
@param[in] None 
@retval ::HI_SUCCESS Success 
@retval ::HI_FAILURE Failure 
@see None
*/
HI_S32 HI_PROC_DeInit(HI_VOID);

/**
@brief User mode proc cretea directory.
@attention You need register module before calling this API. Only support create one level directory. 
@param[in] pszName The directory name. 
@param[out] None 
@retval ::HI_SUCCESS Success 
@retval ::HI_FAILURE Failure 
@see None
*/
HI_S32 HI_PROC_AddDir(const HI_CHAR *pszName);

/**
@brief User mode proc remove directory.
@attention It will return fail if there are entries in the directory.
@param[in] pszName The directory name. 
@retval ::HI_SUCCESS Success
@retval ::HI_FAILURE Failure 
@see None
*/
HI_S32 HI_PROC_RemoveDir(const HI_CHAR *pszName);

/**
@brief User mode proc add entry.
@attention None
@param[in] u32ModuleID Module ID. 
@param[in] pstEntry Parameter of entry.
@retval ::HI_SUCCESS Success
@retval ::HI_FAILURE Failure
@see None
*/
HI_S32 HI_PROC_AddEntry(HI_U32 u32ModuleID, const HI_PROC_ENTRY_S* pstEntry);

/**
@brief User mode proc remove entry. 
@attention None
@param[in] u32ModuleID Module ID. 
@param[in] pstEntry Parameter of entry. 
@retval ::HI_SUCCESS Success  
@retval ::HI_FAILURE Failure 
@see None
*/
HI_S32 HI_PROC_RemoveEntry(HI_U32 u32ModuleID, const HI_PROC_ENTRY_S* pstEntry);

/**
@brief User mode proc print function.
@attention None
@param[in] pstBuf Output buffer parameter.
@param[in] pFmt   Format parameter.
@retval ::HI_SUCCESS Success
@retval ::HI_FAILURE Failure
@see None
*/
HI_S32 HI_PROC_Printf(HI_PROC_SHOW_BUFFER_S *pstBuf, const HI_CHAR *pFmt, ...);




#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* __cplusplus */

#endif /* __MPI_USERPROC_H__ */

