#ifndef __FSCK_FRAGMENT_H__
#define __FSCK_FRAGMENT_H__

#include "fsck_tree.h"
#include "stddef.h"


typedef struct FSCK_FRAGMENT_S
{
    unsigned int head;
    unsigned int length;
    RBT_ENTRY(FSCK_FRAGMENT_S) rb;
}FSCK_FRAGMENT_S;
FSCK_FRAGMENT_S* FSCK_FRAGMENT_New(void);
FSCK_FRAGMENT_S* FSCK_FRAGMENT_Find(void* pstHead, FSCK_FRAGMENT_S* pstNode);
FSCK_FRAGMENT_S* FSCK_FRAGMENT_Insert(void* pstHead, FSCK_FRAGMENT_S* pstNode);
FSCK_FRAGMENT_S* FSCK_FRAGMENT_Min(void* pstHead);
FSCK_FRAGMENT_S* FSCK_FRAGMENT_Next(FSCK_FRAGMENT_S* pstNode);

void FSCK_FRAGMENT_Remove(void* pstHead, FSCK_FRAGMENT_S* pstNode);
void FSCK_FRAGMENT_Free(void* pstHead);

#endif
