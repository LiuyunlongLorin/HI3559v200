/******************************************************************************

  Copyright (C), 2001-2011, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : hi_os.h
  Version       : Initial Draft
  Author        : Hisilicon multimedia software group
  Created       : 2005/4/23
  Last Modified :
  Function List :
  History       :
  1.Date        : 2005/4/20
    Author      : T41030
    Modification: Created file

******************************************************************************/


#ifndef __HI_OS_H__
#define __HI_OS_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#if defined(OS_LINUX)
    
#endif

#if defined(OS_WINCE)

#endif

#if defined(OS_WIN32)

#endif

#if defined(OS_LINUX)
#define  _UNUSED_FUNC __attribute__ ((unused))
#define  _UNUSED_VAR  __attribute__ ((unused))

/*

int xxx() _NOWARN_UNUSED;

int _NOWARN_UNUSED YYY()
{
    return 0;
}

*/
#define _NOWARN_UNUSED _UNUSED_FUNC
#endif

#if defined(OS_VXWORKS)

#endif


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __HI_OS_H__ */
