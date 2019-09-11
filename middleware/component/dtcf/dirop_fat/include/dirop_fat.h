/***********************************************************************************
*              Copyright 2004 - 2050, Hisilicon Tech. Co., Ltd.
*                           ALL RIGHTS RESERVED
* FileName: dirop_fat.h
* Description: filemng core srdk interface for nameassign obj
*
* History:
* Version   Date        DefectNum    Description
* main\1    2015-02-02  NULL         Create this file.
***********************************************************************************/

#ifndef __DIROP_FAT_H__
#define __DIROP_FAT_H__

#include <dirent.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/***************************************************************************************************
  目录项结构体start
*****************************************************************************************************/

struct fat_direntall
{
    unsigned long   d_ino;
    unsigned long   d_off;
    unsigned char   d_type;
    unsigned long long  d_size;
    char    d_createtime[8];
    unsigned short  d_reclen;
    char        d_name[1];
};

struct fat_direntall_buf
{
    int d_count;
    int d_usecount;
    struct fat_direntall direntall;
};

struct dirstream_fat
{
    DIR* dp;

    /* offset of the next dir entry in buffer */
    unsigned int dd_nextloc;

    /* bytes of valid entries in buffer */
    unsigned int dd_size;

    pthread_mutex_t dd_lock;
};

typedef struct hiDIR_FAT
{
    struct dirstream_fat stDirStream;
    struct fat_direntall_buf stBuf;
} DIR_FAT;


/***************************************************************************************************
  目录项结构体end
*****************************************************************************************************/

/***************************************************************************************************
 接口start  用法与opendir/readdir/scandir/closedir一致, 目录项由dirent变为fat_direntall增加了d_createtime和d_size属性
*****************************************************************************************************/
DIR_FAT* opendir_fat(const char* name);
int closedir_fat(DIR_FAT* dir_fat);
struct fat_direntall* readdir_fat(DIR_FAT* dir_fat);
int scandir_fat(const char* dir, struct fat_direntall** *namelist,
                int (*selector) (const struct fat_direntall*),
                int (*compar) (const struct fat_direntall**, const struct fat_direntall**));
/***************************************************************************************************
 接口end
*****************************************************************************************************/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /*__DIROP_FAT_H__*/
