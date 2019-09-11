/******************************************************************************

  Copyright (C), 2001-2011, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : hi_type.h
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


/*
*/
#ifndef __HI_TYPE_H__
#define __HI_TYPE_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#if !defined( OS_VXWORKS) /* there is a same typedef in vxworks for U8 */
typedef unsigned char           U8;
#endif
typedef unsigned short          U16;
typedef unsigned long           U32;
typedef unsigned long long      U64;

typedef char                    S8;
typedef short                   S16;
typedef long                    S32;
typedef long long             S64;

typedef char                    INT8;
typedef short                   INT16;
typedef int                     INT32;

typedef char                    CHAR;
typedef char*                   PCHAR;
#if 0
typedef U8                      B8;
typedef U16                     B16;
typedef U32                     B32;
#endif 

typedef     float              FLOAT;
typedef     double             DOUBLE;
typedef     void               VOID;
typedef     unsigned char      UCHAR;
typedef     unsigned char      UINT8;
typedef     unsigned short     UINT16;
typedef     unsigned long       UINT32;
typedef     unsigned long      ULONG;

typedef     U16                ZWORD;
typedef     U32                ZDWORD;
typedef     U32                ZLENGTH;
typedef     U32                ZSIZE;

#define LEN_Z2C(len) (len+len)

#define LEN_C2Z(len)  (len >> 1)
typedef     unsigned long      PHY_ADDR;
typedef     unsigned long      VADDR;

typedef     void*              PVOID;
typedef     void*              PTR;

#define     INVALID_ADDR       0
/*File descriptor*/
typedef     int                FD;

typedef     unsigned long      LENGTH_T;
typedef     unsigned long      size_type;
typedef     unsigned long      SIZE_T;
typedef     unsigned long      COUNT_T;

#if !defined(OFF_T)
typedef unsigned long OFF_T ;
#endif

typedef long SOFF_T;//by hhm
   
typedef     unsigned long      HI_RET;

#define MAX_FILENAME_LENGTH 128
typedef     char FILENAME_T[MAX_FILENAME_LENGTH];

typedef     char CHECKNAME_T[32];
typedef char IPADDRESS_T[32];	//add by ququ

/*#define     EMPTY_VAR U32 null_var*/
#define     EMPTY_VAR unsigned int xyz

/* Vxworks defined BOOL type */
#if !defined(BOOL) && !defined(OS_VXWORKS)
#define BOOL U8
#endif

#ifdef TRUE
    #undef TRUE
#endif
#define TRUE            0x1

#ifdef FALSE
    #undef FALSE
#endif
#define FALSE           0x0L

#define YES 0x1
#define NO  0x0L
    
#ifndef NULL
#define NULL            (U32)0x0L
#endif


/* *DO NOT* type like : if(HI_OK)*/
#define HI_OK      0 
#define HI_ERROR   (1)

#define HI_SUCCESS 0
#define HI_FAILURE  1

#define HI_RET_NOT_DEFINED 0 



#define AV_MALLOC(sizeLen) malloc(sizeLen)
#define AV_MALLOC_ALIGN(ucAlign, sizeLen) malloc(sizeLen)

typedef void* AV_PTR;
typedef void* VIDEO_PTR;
typedef void* AUDIO_PTR;

#ifndef MAXPATHLEN
#define MAXPATHLEN 1024
#endif

/*typedef char[MAXPATHLEN]  FILENAME_T;*/


#define MAX_POOL_NAME_LEN 16
typedef char POOL_NAME[MAX_POOL_NAME_LEN];

typedef unsigned long long PTS_TIME;

typedef struct PVADDR_STRU
{
    VOID*     pvAddr;/*point of virtual address*/
    PHY_ADDR  phyAddr; /*phy address*/
}PVADDR_T;

typedef struct ADDRINFO_STRU
{
    PHY_ADDR phyAddrStart;
    SIZE_T   sizeAddr;
}ADDRINFO_T;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __HI_TYPE_H__ */
