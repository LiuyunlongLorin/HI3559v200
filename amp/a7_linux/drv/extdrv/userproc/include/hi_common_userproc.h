/******************************************************************************

  Copyright (C), 2001-2011, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : hi_common_userproc.h
  Version       : Initial Draft
  Author        : Hisilicon multimedia software group
  Created       : 2017/04/07
  Description   : Common apis for user proc system.
  History       :
  1.Date        : 2017/04/07
    Author      : xxx
    Modification: Created file

*******************************************************************************/

#ifndef __HI_COMMON_USERPROC_H__
#define __HI_COMMON_USERPROC_H__

#include "hi_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/*******************************Structure declaration *****************************/
/** @addtogroup     COMMON */
/** @{ */ /** <!--  [COMMON] */

/** Global config structure */
#define HI_INVALID_MODULE_ID        (0xffffffff)
#define HI_MAX_USER_MODULE_NUMBER   (256)

/** Module ID flags */
typedef enum hiMOD_ID_E
{
    /**<system common. */ 
    HI_ID_PROC       =1,

    /**<reserve, user definition. */ 
    HI_ID_USR_START   = 0xC0,
    HI_ID_USR_END     = 0xFE,

    HI_ID_BUTT        = 0xFF
} HI_MOD_ID_E;


/** Defines user mode proc show buffer */
typedef struct hiPROC_SHOW_BUFFER_S
{
    HI_U8* pu8Buf;                  /**<Buffer address*/  
    HI_U32 u32Size;                 /**<Buffer size*/    
    HI_U32 u32Offset;               /**<Print Offset*/       
}HI_PROC_SHOW_BUFFER_S;

/** Proc show function */
typedef HI_S32 (* HI_PROC_SHOW_FN)(HI_PROC_SHOW_BUFFER_S * pstBuf, HI_VOID *pPrivData);

/** Proc command function */
typedef HI_S32 (* HI_PROC_CMD_FN)(HI_PROC_SHOW_BUFFER_S * pstBuf, HI_U32 u32Argc, HI_U8 *pu8Argv[], HI_VOID *pPrivData);

/** Defines user mode proc entry */
typedef struct hiPROC_ENTRY_S
{
    HI_CHAR *pszEntryName;          /**<Entry name*/           
    HI_CHAR *pszDirectory;          /**<Directory name. If null, the entry will be added to /proc/hisi directory*/
                                   
    HI_PROC_SHOW_FN pfnShowProc;    /**<Proc show function*/   
    HI_PROC_CMD_FN pfnCmdProc;      /**<Proc command function*/ 
    HI_VOID *pPrivData;             /**<Private data*/        
}HI_PROC_ENTRY_S;


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __HI_COMMON_USERPROC_H__ */

