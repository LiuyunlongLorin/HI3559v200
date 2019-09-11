#ifndef __FSCK_DIR_H__
#define __FSCK_DIR_H__

#include "hi_fstool.h"

#define FSCK_DIR_LN_LEN_MAX        (256)    /* long name maximal length */
#define FSCK_DIR_LNR_1ST_MARK      (0x40)   /* first long name record */
#define FSCK_DIR_LNR_SEQ_NUM_MASK  (0x1f)   /* mask to extract long record sequence number */
#define FSCK_DIR_NAME_LEN_MAX      (13)     /**8+1+3+1*/


/******FAT Dir entry  struct ****/
typedef struct FSCK_DIR_ENTRY_S
{
    struct FSCK_DIR_ENTRY_S
    *parent,                    /* previous tree  */
    *next,                      /* next brother */
    *child;                     /* directory have this*/
    char name[FSCK_DIR_NAME_LEN_MAX];    /* alias name first part */
    char lname[FSCK_DIR_LN_LEN_MAX];     /* real name */
    u_int flags;                /* attributes */
    u_int state;                /* dir entry state*/
    cl_t head;                      /* cluster num */
    u_int size;                 /* the filesize in bytes */
}FSCK_DIR_ENTRY_S;

typedef struct FSCK_DIR_BAK_NODE_S
{
    FSCK_DIR_ENTRY_S *dir;
    struct FSCK_DIR_BAK_NODE_S *next;
}FSCK_DIR_BAK_NODE_S;

/**********************functions ********************/
u_int FSCK_DIR_Reset(FSCK_BOOT_SECTOR_S *pstBoot);//api--->dir

void FSCK_DIR_Free(void);//api--->dir

u_int FSCK_DIR_HandleTree(int fs, FSCK_BOOT_SECTOR_S* pstBoot, HI_FSTOOL_CHECK_MODE_E enMode);//api--->dir

#endif
