#ifndef _FATCACHE_H_
#define _FATCACHE_H_
#include "fsck_tree.h"
#include "stddef.h"
#include "fsck_boot.h"
/******FAT cache struct ****/

typedef struct FSCK_CACHE_S
{
    struct FSCK_CACHE_S *next;
    cl_t head;
    u_int length;
}FSCK_CACHE_S;

typedef struct FSCK_CACHE_CLUSTER_CHAIN_S
{
    struct FSCK_CACHE_S* child;
    cl_t    head;                   /* pointer to start of chain */
    u_int length;               /* how many cluster this file contains*/
    RBT_ENTRY(FSCK_CACHE_CLUSTER_CHAIN_S) rb;
    u_int flag;
    u_int segments;
}FSCK_CACHE_CLUSTER_CHAIN_S;

#define FSCK_BIT(x,n)    (((x)>>(n)) & 0x1)
#define SET_BIT(x,n)    do{ \
    x |= 1<<n;}while (0)
#define CLEAR_BIT(x,n)  do{ \
    x &= ~(1<<n);}while (0)

#ifndef offsetof
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#endif

#define container_of(ptr, type, member) \
    ((type *)((unsigned long)(ptr) - offsetof(type, member)))

FSCK_CACHE_CLUSTER_CHAIN_S* FSCK_CACHE_NewChain(void);
FSCK_CACHE_CLUSTER_CHAIN_S* FSCK_CACHE_FindChain(void* pstHead, FSCK_CACHE_CLUSTER_CHAIN_S* pstChain);
FSCK_CACHE_CLUSTER_CHAIN_S* FSCK_CACHE_NextChain(FSCK_CACHE_CLUSTER_CHAIN_S* pstChain);
FSCK_CACHE_CLUSTER_CHAIN_S* FSCK_CACHE_InsertChain(void* pstHead, FSCK_CACHE_CLUSTER_CHAIN_S* pstChain);
FSCK_CACHE_CLUSTER_CHAIN_S* FSCK_CACHE_MinChain(void* pstHead);
void FSCK_CACHE_RemoveChain(void* pstHead, FSCK_CACHE_CLUSTER_CHAIN_S* pstChain);
void FSCK_CACHE_DumpChain(FSCK_CACHE_CLUSTER_CHAIN_S *fat);

FSCK_CACHE_S* FSCK_CACHE_New(void);
FSCK_CACHE_S* FSCK_CACHE_Next(FSCK_CACHE_CLUSTER_CHAIN_S* fat,unsigned int clus, unsigned int* cl);
void FSCK_CACHE_Add(FSCK_CACHE_CLUSTER_CHAIN_S *fatentry , FSCK_CACHE_S *new);
int FSCK_CACHE_AddToTail(FSCK_CACHE_CLUSTER_CHAIN_S *fatentry , FSCK_CACHE_S *new);
void FSCK_CACHE_SetNext(u_char* fat ,unsigned int cl ,unsigned int  next);
void FSCK_CACHE_Trunc(FSCK_BOOT_SECTOR_S * pstBoot, FSCK_CACHE_CLUSTER_CHAIN_S* fat, unsigned int cl);
void FSCK_CACHE_Free(void* pTree);

#endif
