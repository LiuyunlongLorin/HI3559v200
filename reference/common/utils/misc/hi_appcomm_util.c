/**
 * @file    hi_appcomm_util.c
 * @brief   common utils functions.
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/18
 * @version   1.0

 */
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include "hi_appcomm_log.h"
#include "hi_appcomm_util.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#ifndef __HuaweiLite__
#define DEFAULT_MD_LEN (128)

typedef struct tagMMAP_NODE_S
{
    HI_U64 u64Start_P;
    HI_U64 u64Start_V;
    HI_U32 u32Length;
    HI_U32 u32RefCount;
    struct tagMMAP_NODE_S* pstNext;
} MMAP_NODE_S;

/**static init mutex */
static pthread_mutex_t s_MmapMutex = PTHREAD_MUTEX_INITIALIZER;
static MMAP_NODE_S* s_pstMMAPNode = NULL;
#endif

/**
 * @brief    check specified path is directory or not.
 * @param[in] pszPath:directory path
 * @return 1:directory 0:file -1:not exist.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/19
 */
HI_S32 HI_PathIsDirectory(const HI_CHAR* pszPath)
{
    HI_APPCOMM_CHECK_POINTER(pszPath, HI_FAILURE);
    if(0==strlen(pszPath))
    {
        return HI_FAILURE;
    }
    struct stat stStatBuf;
    HI_S32 s32Ret = stat(pszPath, &stStatBuf);

    if (0 == s32Ret)
    {
        if (stStatBuf.st_mode & S_IFDIR)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }

    return HI_FAILURE;
}

/**
 * @brief    delete specified directory.
 * @param[in] pszPath:directory path
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/19
 */
HI_S32 HI_rmdir(const HI_CHAR* pszPath)
{
    HI_APPCOMM_CHECK_POINTER(pszPath, HI_FAILURE);
    struct dirent* pDirent = NULL;
    DIR* pDir = NULL;
    HI_CHAR aszSubDir[HI_APPCOMM_MAX_PATH_LEN] = {0,};
    HI_S32 s32Ret = HI_SUCCESS;
    pDir = opendir(pszPath);

    if (!pDir)
    {
        MLOGE("opendir[%s] failed\n", pszPath);
        return HI_FAILURE;
    }

    while ((pDirent = readdir(pDir)) != NULL)
    {
        if ((0 == strcmp(pDirent->d_name, ".")) || (0 == strcmp(pDirent->d_name, "..")))
        {
            continue;
        }

        snprintf(aszSubDir, sizeof(aszSubDir), "%s/%s", pszPath, pDirent->d_name);
        s32Ret = HI_PathIsDirectory(aszSubDir);

        if (1 == s32Ret)
        {
            s32Ret = HI_rmdir(aszSubDir);

            if (HI_SUCCESS != s32Ret)
            {
                MLOGD("Delete SubDir[%s] failed\n", aszSubDir);
                closedir(pDir);
                return HI_FAILURE;
            }
        }
        else if (0 == s32Ret)
        {
            s32Ret = remove(aszSubDir);

            if (HI_SUCCESS != s32Ret)
            {
                MLOGD("Delete File[%s] failed\n", aszSubDir);
                closedir(pDir);
                return HI_FAILURE;
            }
        }
    }

    closedir(pDir);
    s32Ret = remove(pszPath);

    if (HI_SUCCESS != s32Ret)
    {
        MLOGD("remove[%s] failed\n", pszPath);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

/**
 * @brief    create specified directory.
 * @param[in] pszPath:directory path
 * @param[in] mode:create mode
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/19
 */
HI_S32 HI_mkdir(const HI_CHAR* pszPath, mode_t mode)
{
    HI_CHAR DirName[HI_APPCOMM_MAX_PATH_LEN] = {0};
    HI_CHAR DirNameBak[HI_APPCOMM_MAX_PATH_LEN] = {0};
    struct stat stStat;
    HI_S32 s32Fd = -1;
    HI_S32 s32Len = 0;
    HI_S32 s32Idx = 0;
    HI_S32 s32Ret = 0;

    /* Check Dir Path */
    HI_APPCOMM_CHECK_POINTER(pszPath, HI_FAILURE);

    if ((s32Len = strlen(pszPath)) == 0 )
    {
        MLOGE("Empty Dir path\n");
        return HI_FAILURE;
    }

    /* Add tail '/' if not exist */
    if (pszPath[s32Len - 1] != '/')
    {
        snprintf(DirName, sizeof(DirName), "%s/", pszPath);
    }
    else
    {
        snprintf(DirName, sizeof(DirName), "%s", pszPath);
    }

    s32Len = strlen(DirName);

    /* Create Directory */
    for (s32Idx = 1; s32Idx < s32Len; s32Idx++)
    {
        if (DirName[s32Idx] == '/')
        {
            DirName[s32Idx] = 0; /* Split DirName temporary */
            /* open & fsat way to avoid the TOCTOU coverity problem */
            s32Fd = open(DirName, O_RDONLY);

            if (0 > s32Fd)
            {
                if (ENOENT == errno )/* dir not exist */
                {
                    if (mkdir(DirName, mode) < 0)
                    {
                        MLOGE("!!!Failed to mkdir %s\n", DirName);
                        return HI_FAILURE;
                    }
                }
            }
            else
            {
                s32Ret = fstat(s32Fd, &stStat);
                close(s32Fd);

                if ((0 == s32Ret) && (!S_ISDIR(stStat.st_mode)))
                {
                    /* it is not a dir,Rename it */
                    snprintf(DirNameBak, sizeof(DirName), "%s%s", DirName, ".BAK");
                    s32Ret = rename(DirName, DirNameBak) ;

                    if (HI_SUCCESS != s32Ret)
                    {
                        MLOGE("!!!Rename FIle %s to %s failed\n", DirName, DirNameBak);
                    }

                    /* create dir */
                    if (mkdir(DirName, mode) < 0)
                    {
                        MLOGE("!!!Failed to mkdir %s\n", DirName);
                        return HI_FAILURE;
                    }
                }
            }

            DirName[s32Idx] = '/';
        }
    }

    return HI_SUCCESS;
}

HI_S32 HI_system(const HI_CHAR* pszCmd)
{
#ifndef __HuaweiLite__
    pid_t pid;
    HI_S32 status;
    if(pszCmd == NULL)
    {
        return (1); /**< if cmdstring is NULL£¬return no zero*/
    }
    if((pid = vfork())<0) /**<vfork,child pid share resource with parrent,not copy */
    {
        status = -1; /**<vfork fail */
    }
    else if(pid == 0)
    {
        execl("/bin/sh", "sh", "-c", pszCmd, (char *)0);
        _exit(127); /**< return 127 only exec fail;the chid procee is not exist normore if exec success */
    }
    else /** parrent pid*/
    {
        while(waitpid(pid, &status, 0) < 0)
        {
            if(errno != EINTR)
            {
                status = -1; /**< return -1 when interrupted by signal except EINTR*/
                break;
            }
        }
    }
    return status; /**< return the state of child progress if waitpid success*/
#else
    MLOGW("HuaweiLiteOS is NOT SUPPORT system(%s).\n",pszCmd);
    return HI_SUCCESS;
#endif
}

#ifndef __HuaweiLite__
/* no need considering page_size of 4K */
HI_VOID* HI_MemMap(HI_U64 u64PhyAddr, HI_U32 u32Size)
{
#ifndef PAGE_SIZE
#define PAGE_SIZE 0x1000
#endif
#define PAGE_SIZE_MASK 0xfffffffffffff000UL

    HI_U64 phy_addr_in_page;
    HI_U64 page_diff;
    HI_U64 size_in_page;
    MMAP_NODE_S* pTmp;
    MMAP_NODE_S* pNew;
    HI_VOID* addr = NULL;

    if (0 == u64PhyAddr || 0 == u32Size)
    {
        MLOGE("memmap():size can't be zero!\n");
        return NULL;
    }

    /* check if the physical memory space have been mmaped */
    pTmp = s_pstMMAPNode;
    HI_MUTEX_LOCK(s_MmapMutex);

    while (pTmp != NULL)
    {
        if ( (u64PhyAddr >= pTmp->u64Start_P) &&
             ( (u64PhyAddr + u32Size) <= (pTmp->u64Start_P + pTmp->u32Length) ) )
        {
            pTmp->u32RefCount++;
            HI_MUTEX_UNLOCK(s_MmapMutex);
            return (HI_VOID*)(HI_UL)(pTmp->u64Start_V + u64PhyAddr - pTmp->u64Start_P);
        }

        pTmp = pTmp->pstNext;
    }

    HI_MUTEX_UNLOCK(s_MmapMutex);
    static HI_S32 fd = -1;

    /* not mmaped yet */
    if (fd < 0)
    {
#if __WORDSIZE==64
        const char dev[] = "/dev/mmz_userdev";
#else
        const char dev[] = "/dev/mem";
#endif
        /* dev not opened yet, so open it */
        fd = open (dev, O_RDWR | O_SYNC);

        if (fd < 0)
        {
            MLOGE("memmap():open %s error!\n", dev);
            return NULL;
        }
    }

    /* addr align in page_size(4K) */
    phy_addr_in_page = u64PhyAddr & PAGE_SIZE_MASK;
    page_diff = u64PhyAddr - phy_addr_in_page;
    /* size in page_size */
    size_in_page = ((u32Size + page_diff - 1) & PAGE_SIZE_MASK) + PAGE_SIZE;
#if __WORDSIZE==64
    addr = mmap ((HI_VOID*)0, size_in_page, PROT_READ | PROT_WRITE, MAP_SHARED, fd, phy_addr_in_page);
#else
    addr = mmap64 ((HI_VOID*)0, size_in_page, PROT_READ | PROT_WRITE, MAP_SHARED, fd, phy_addr_in_page);
#endif
    if (addr == MAP_FAILED)
    {
        MLOGE("memmap():mmap @ 0x%llx error!%s\n", phy_addr_in_page,strerror(errno));
        return NULL;
    }

    /* add this mmap to MMAP Node */
    pNew = (MMAP_NODE_S*)malloc(sizeof(MMAP_NODE_S));

    if (NULL == pNew)
    {
        munmap(addr, size_in_page);
        MLOGE("memmap():malloc new node failed!\n");
        return NULL;
    }

    HI_MUTEX_LOCK(s_MmapMutex);
    pNew->u64Start_P = phy_addr_in_page;
    pNew->u64Start_V = (HI_UL)addr;
    pNew->u32Length = size_in_page;
    pNew->u32RefCount = 1;
    pNew->pstNext = NULL;

    if (s_pstMMAPNode == NULL)
    {
        s_pstMMAPNode = pNew;
    }
    else
    {
        pTmp = s_pstMMAPNode;

        while (pTmp->pstNext != NULL)
        {
            pTmp = pTmp->pstNext;
        }

        pTmp->pstNext = pNew;
    }

    HI_MUTEX_UNLOCK(s_MmapMutex);
    return (HI_VOID*)((HI_CHAR*)addr + page_diff);
}

HI_S32 HI_MemUnmap(HI_VOID* pvMappedAddr)
{
    MMAP_NODE_S* pPre;
    MMAP_NODE_S* pTmp;

    if (s_pstMMAPNode == NULL)
    {
        MLOGE("memunmap(): address have not been mmaped!\n");
        return HI_FAILURE;
    }

    /* check if the physical memory space have been mmaped */
    pTmp = s_pstMMAPNode;
    pPre = s_pstMMAPNode;
    HI_MUTEX_LOCK(s_MmapMutex);

    do
    {
        if ( ((HI_UL)pvMappedAddr >= pTmp->u64Start_V) &&
             ((HI_UL)pvMappedAddr <= (pTmp->u64Start_V + pTmp->u32Length)) )
        {
            pTmp->u32RefCount--;

            if (0 == pTmp->u32RefCount)
            {

                /* delete this map node from pMMAPNode */
                if (pTmp == s_pstMMAPNode)
                {
                    s_pstMMAPNode = pTmp->pstNext;
                }
                else
                {
                    pPre->pstNext = pTmp->pstNext;
                }

                /* munmap */
                if (munmap((HI_VOID*)(HI_UL)pTmp->u64Start_V, pTmp->u32Length) != 0 )
                {
                    MLOGE("memunmap(): munmap failed!\n");
                }

                HI_APPCOMM_SAFE_FREE(pTmp);
            }

            HI_MUTEX_UNLOCK(s_MmapMutex);
            return HI_SUCCESS;
        }

        pPre = pTmp;
        pTmp = pTmp->pstNext;
    }
    while (pTmp != NULL);

    HI_MUTEX_UNLOCK(s_MmapMutex);
    MLOGE("memunmap(): address have not been mmaped!\n");
    return HI_FAILURE;
}

HI_S32 himm(HI_U32 u32Addr, HI_U32 u32Value)
{
    HI_VOID* pmem = HI_MemMap(u32Addr, DEFAULT_MD_LEN);

    if (pmem == NULL)
    {
        return HI_FAILURE;
    }

    *(HI_U32*)pmem = u32Value;
    return HI_MemUnmap(pmem);
}

HI_S32 himd(HI_U32 u32Addr, HI_U32* pu32Value)
{
    HI_VOID* pmem = HI_MemMap(u32Addr, DEFAULT_MD_LEN);

    if (pmem == NULL)
    {
        return HI_FAILURE;
    }

    *pu32Value = *(HI_U32*)pmem ;
    return HI_MemUnmap(pmem);
}
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
