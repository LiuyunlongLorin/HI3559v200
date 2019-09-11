/*-----------------------------------------------------------------------------
 * Copyright (c) <2013-2015>, <Huawei Technologies Co., Ltd>
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation and/
 * or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 ------------------------------------------------------------------------------
 * Notice of Export Control Law
 ==============================================
 * Huawei LiteOS may be subject to applicable export control laws and regulations,
 * which might include those applicable to Huawei LiteOS of U.S. and the country in
 * which you are located.
 * Import, export and usage of Huawei LiteOS in any manner by you shall be in
 * compliance with such applicable export control laws and regulations.
 *---------------------------------------------------------------------------*/

#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"
#include "errno.h"
#include "pthread.h"
#include "inode/inode.h"
#include "disk.h"
#include "sys/mount.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

los_disk _sys_disk[SYS_MAX_DISK];
los_part _sys_part[SYS_MAX_PART];

unsigned int g_uwFatSectorsPerBlock = CONFIG_FS_FAT_SECTOR_PER_BLOCK;
unsigned int g_uwFatBlockNums       = CONFIG_FS_FAT_BLOCK_NUMS;

unsigned int g_uwUsbMode = 0;
extern int sd_sync(int);

#define DISK_LOCK(mux) do { \
    unsigned int mux_ret = 0; \
    mux_ret = pthread_mutex_lock(mux); \
    if (mux_ret != 0) { \
        PRINT_ERR("%s %d, mutex lock failed\n", __FUNCTION__, __LINE__); \
    } \
}while(0);

#define DISK_UNLOCK(mux) do { \
    unsigned int mux_ret = 0; \
    mux_ret = pthread_mutex_unlock(mux); \
    if (mux_ret != 0) { \
        PRINT_ERR("%s %d, mutex unlock failed\n", __FUNCTION__, __LINE__); \
    } \
}while(0);

typedef VOID* (*STORAGE_HOOK_FUNCTION)(VOID*);
static unsigned int osReHookFuncAddDiskRef(STORAGE_HOOK_FUNCTION handler, VOID *param)  __attribute__((weakref("osReHookFuncAdd"))); /*lint -e402*/
static unsigned int osReHookFuncDelDiskRef(STORAGE_HOOK_FUNCTION handler)   __attribute__((weakref("osReHookFuncDel")));

int los_alloc_diskid_byname(const char *disk_name)
{
    int disk_id;
    los_disk   *disk;

    if (NULL == disk_name)
    {
        PRINT_ERR("The paramter disk_name is NULL");
         return -1;
    }

    for (disk_id = 0; disk_id < SYS_MAX_DISK; disk_id++)
    {
        disk = get_disk(disk_id);
        if (disk && disk->disk_status == STAT_UNUSED)
            break;
    }
    if (!disk || SYS_MAX_DISK == disk_id)
    {
        PRINT_ERR("los_alloc_diskid_byname failed %d!\n",disk_id);
        return -1;
    }

    if (strlen(disk_name) <= DISK_NAME)
    {
        strncpy(disk->disk_name, disk_name, sizeof(disk->disk_name));
        disk->disk_name[sizeof(disk->disk_name)-1] = '\0';
    }
    else
    {
        PRINT_ERR("disk_name is too long!\n");
        return -1;
    }
    return disk_id;
}

int los_get_diskid_byname(const char *disk_name)
{
    int disk_id;
    los_disk   *disk;
    int disk_name_len = 0;

    if (NULL == disk_name)
    {
        PRINT_ERR("The paramter disk_name is NULL");
        return -1;
    }

    disk_name_len = strlen(disk_name);

    for (disk_id = 0; disk_id < SYS_MAX_DISK; disk_id++)
    {
        disk = get_disk(disk_id);
        if (disk && disk->disk_status == STAT_UNUSED)
            continue;
        if (disk && (0 == strncmp(disk_name, disk->disk_name, disk_name_len)))
        {
            if (disk_name_len == strlen(disk->disk_name))
                break;
        }
    }
    if (!disk || SYS_MAX_DISK == disk_id)
    {
        PRINT_ERR("los_get_diskid_byname failed!\n");
        return -1;
    }
    return disk_id;
}
void os_set_usb_status(unsigned int disk_id)
{
    if (disk_id < SYS_MAX_DISK)
    {
        g_uwUsbMode |= (1 << disk_id); /*lint !e502*/
    }
}

void os_clear_usb_status(unsigned int disk_id)
{
    if (disk_id < SYS_MAX_DISK)
    {
        g_uwUsbMode &= ~(1 << disk_id); /*lint !e502*/
    }
}

static BOOL get_disk_usb_status(unsigned int disk_id) /*lint -e528*/
{
    return (g_uwUsbMode & (1 << disk_id))?TRUE:FALSE;
}

los_disk *get_disk(int id)
{
    if (id >=0 && id < SYS_MAX_DISK)
    {
        return &_sys_disk[id];
    }
    else
    {
        return (los_disk *)NULL;
    }
}

los_part *get_part(int id)
{
    if (id >=0 && id < SYS_MAX_PART)
    {
        return &_sys_part[id];
    }
    else
    {
        return (los_part *)NULL;
    }
}

static unsigned long long get_first_part_start(los_part * part)
{
    los_part *first_part = NULL;
    los_disk *disk = get_disk(part->disk_id);

    first_part = (disk == NULL) ? NULL : LOS_DL_LIST_ENTRY(disk->head.pstNext, los_part, list);

    return (first_part == NULL) ? 0 : first_part->sector_start;
}

static void disk_part_add_to_disk(los_disk *disk, los_part *part)
{
    part->disk_id      = disk->disk_id;
    part->part_no_disk = disk->part_count;
    LOS_ListTailInsert(&disk->head, &part->list);
    disk->part_count++;
}

static void disk_part_del_from_disk(los_disk *disk, los_part *part)
{
    LOS_ListDelete(&part->list);
    disk->part_count--;
}

static los_part *disk_part_allocate(struct inode *dev, unsigned long long start, unsigned long long count)
{
    int i;
    los_part *part = get_part(0);

    for (i = 0; i < SYS_MAX_PART; i++)
    {
        if (!part->dev)
        {
            part->part_id      = i;
            part->part_no_mbr  = 0;
            part->dev          = dev;
            part->sector_start = start;
            part->sector_count = count;
            part->part_name    = (char *)NULL;
            LOS_ListInit(&part->list);

            return part;
        }
        part ++;
    }

    return (los_part *)NULL;
}

static void disk_part_release(los_part *part)
{
    part->dev          = (struct inode *)NULL;
    part->part_no_disk = 0;
    part->part_no_mbr  = 0;
    if (part->part_name)
    {
        free(part->part_name);
        part->part_name = NULL;
    }
}

static int disk_add_part(los_disk *disk, unsigned long long sector_start, unsigned long long sector_count)
{
    char dev_name[30];
    struct inode *disk_dev, *part_dev;
    los_part *part;

    if ((disk == NULL) || (disk->disk_status == STAT_UNUSED))
        return -1;

    (void)snprintf(dev_name, sizeof(dev_name), "%s%c%d", disk->disk_name, 'p', disk->part_count); /*lint !e534*/
    disk_dev = disk->dev;

    if (register_blockdriver(dev_name, disk_dev->u.i_bops, 0755, disk_dev->i_private))
    {
        PRINT_ERR("disk_add_part : register %s fail!\n", dev_name);
        return -1;
    }
    else
    {
        part_dev = inode_find(dev_name, (const char **)NULL);
        if (part_dev)
        {
            PRINTK("disk_add_part : register %s ok!\n", dev_name);
        }
        else
        {
            PRINT_ERR("disk_add_part : find %s fail!\n", dev_name);
            return -1;
        }
    }

    part = disk_part_allocate(part_dev, sector_start, sector_count);
    inode_release(part_dev);
    if (part == NULL)
    {
        unregister_blockdriver(dev_name); /*lint !e534*/
        return -1;
    }

    disk_part_add_to_disk(disk, part);
    if (disk->type == EMMC)
    {
        part->type = EMMC;
    }
    return (int)part->part_id;
}

static int disk_divide(los_disk *disk, struct disk_divide_info *info)
{
    unsigned int i = 0;
    int ret = 0;

    if(disk == NULL || info == NULL)
    {
        return -1;
    }

    disk->type = info->part[i].type;
    for(i = 0; i < info->part_count; i++)
    {
        if(info->sector_count < info->part[i].sector_start)
        {
            return -1;
        }
        if(info->part[i].sector_count > (info->sector_count - info->part[i].sector_start))
        {
            PRINT_ERR("Part[%d] sector_start:%llu, sector_count:%llu, exceed emmc sector_count:%llu.\n", i, info->part[i].sector_start, info->part[i].sector_count,  (info->sector_count - info->part[i].sector_start));
            info->part[i].sector_count = info->sector_count - info->part[i].sector_start;
            PRINT_ERR("Part[%d] sector_count change to %llu.\n", i,  info->part[i].sector_count);

            ret = disk_add_part(disk, info->part[i].sector_start, info->part[i].sector_count);
            if (ret == -1)
            {
                return -1;
            }
            break;
        }
        ret = disk_add_part(disk, info->part[i].sector_start, info->part[i].sector_count);
        if (ret == -1)
        {
            return -1;
        }
    }

    return 0;
}

static char gpt_partition_type_recognition(char *par_buf)
{
    char *buf = par_buf;

    if ((LD_DWORD_DISK(&buf[BS_FILSYSTEMTYPE32]) & 0xFFFFFF) == 0x544146 || \
        !strncmp(&buf[BS_FILSYSTYPE], "FAT", 3))
    {
        return 0x0B;
    }
    else if (!strncmp(&buf[BS_JMPBOOT], "\xEB\x76\x90" "EXFAT   ", 11) || \
        !strncmp(&buf[BS_JMPBOOT], "\xEB\x52\x90" "NTFS    ", 11))
    {
        return 0x07;
    }
    else
    {
        return 0;
    }
}

static int disk_gpt_partition_recognition(struct inode *blkdrv, struct disk_divide_info *info)
{
    char *gpt_buf = NULL;
    char *partition_buf = NULL;
    char partition_type;
    unsigned int table_num = 0;
    unsigned long long partition_start, partition_end;
    unsigned int partition_count = 0;
    unsigned int i = 0;
    unsigned int j = 0;
    unsigned int index = 0;
    int ret = -1;

    gpt_buf = (char *)memalign(64, info->sector_size);
    partition_buf = (char *)memalign(64, info->sector_size);
    if (!gpt_buf || !partition_buf)
    {
        PRINT_ERR("guid buffer malloc failed!\n");
        goto out_with_mem;
    }
    memset(gpt_buf, 0 , info->sector_size);
    memset(partition_buf, 0 , info->sector_size);

    ret = blkdrv->u.i_bops->read(blkdrv, (unsigned char *)gpt_buf, 1, 1);
    if (ret != 1)
    {
        PRINT_ERR("%s %d\n", __FUNCTION__, __LINE__);
        goto out_with_mem;
    }

    if (!VERIFY_GPT(gpt_buf))
    {
        PRINT_ERR("%s %d\n", __FUNCTION__, __LINE__);
        goto out_with_mem;
    }

    table_num = LD_DWORD_DISK(&gpt_buf[TABLE_NUM_OFFSET]);
    if (table_num > TABLE_MAX_NUM)
    {
        table_num = TABLE_MAX_NUM;
    }

    index = ((table_num % 4) ? (table_num / 4 + 1) : (table_num / 4));

    for (i = 0; i < index; i++)
    {
        memset(gpt_buf, 0, info->sector_size);
        ret = blkdrv->u.i_bops->read(blkdrv, (unsigned char *)gpt_buf, (TABLE_START_SECTOR + i), 1);
        if (ret != 1)
        {
            PRINT_ERR("%s %d\n", __FUNCTION__, __LINE__);
            goto out_with_mem;
        }

        for (j = 0; j < 4; j++)
        {
            if (VERITY_AVAILABLE_PAR(&gpt_buf[j * TABLE_SIZE]))
            {
                if (VERITY_PAR_VALID(&gpt_buf[j * TABLE_SIZE]))
                {
                    partition_start = LD_QWORD_DISK(&gpt_buf[j * TABLE_SIZE + 32]);
                    partition_end = LD_QWORD_DISK(&gpt_buf[j * TABLE_SIZE + 40]);

                    memset(partition_buf, 0, info->sector_size);
                    ret = blkdrv->u.i_bops->read(blkdrv, (unsigned char *)partition_buf, partition_start, 1);
                    if (ret != 1)
                    {
                        PRINT_ERR("%s %d\n", __FUNCTION__, __LINE__);
                        goto out_with_mem;
                    }

                    if (VERIFY_FS(partition_buf))
                    {
                        partition_type = gpt_partition_type_recognition(partition_buf);
                        if (partition_type) {
                            if (partition_count >= MAX_DIVIDE_PART_PER_DISK)
                            {
                                goto out_with_mem;
                            }
                            info->part[partition_count].type = partition_type;
                            info->part[partition_count].sector_start = partition_start;
                            info->part[partition_count].sector_count = (partition_end - partition_start + 1);
                            partition_count++;
                        }
                        else
                        {
                            PRINT_ERR("The partition type is not allowed to use!\n");
                        }
                    }
                    else
                    {
                        PRINT_ERR("Do not support the partition type!\n");
                        continue;
                    }
                }
                else
                {
                    goto out_with_mem;
                }
            }
            else
            {
                PRINTK("The partition type is ESP or MSR!\n");
            }
        }
    }

    ret = partition_count;
out_with_mem:
    free(gpt_buf);
    free(partition_buf);
    return ret;
}

static int disk_partition_recognition(struct inode *blkdrv, struct disk_divide_info *info)
{
    int i;
    int ret = -1;
    int extended_flag = 0;
    int extended_pos = -1;
    int mbr_count = 0;
    int ebr_count = 0;
    unsigned int extended_address = 0;
    unsigned int extended_offset = 0;
    char mbr_partition_type, ebr_partition_type;
    char *mbr_buf = NULL;
    char *ebr_buf = NULL;

    mbr_buf = (char *)memalign(64, info->sector_size);
    ebr_buf = (char *)memalign(64, info->sector_size);
    if (!mbr_buf || !ebr_buf)
    {
        PRINT_ERR("buffer memalign failed!\n");
        goto out_with_mem;
    }

    memset(mbr_buf, 0, info->sector_size);
    memset(ebr_buf, 0, info->sector_size);

    /* Recognize parimary paritions. */
    ret = blkdrv->u.i_bops->read(blkdrv, (unsigned char *)mbr_buf, 0, 1);
    if (ret != 1)
    {
        PRINT_ERR("driver read return error: %d\n", ret);
        goto out_with_mem;
    }

    /* Check boot record signature. */
    if (LD_WORD_DISK(&mbr_buf[BS_SIG55AA]) != 0xAA55)
    {
        ret = -1;
        goto out_with_mem;
    }

    /* The partition type is GPT */
    if (mbr_buf[PARTION_MODE_BTYE] == 0xEE) /*lint !e650*/
    {
        ret = disk_gpt_partition_recognition(blkdrv, info);
        goto out_with_mem;
    }

    if (VERIFY_FS(mbr_buf))
    {
        ret = blkdrv->u.i_bops->read(blkdrv, (unsigned char *)ebr_buf, LD_DWORD_DISK(&mbr_buf[PAR_OFFSET + PAR_START_OFFSET]), 1);
        if (ret != 1 || !VERIFY_FS(ebr_buf))
        {
            ret = 0;
            goto out_with_mem;
        }
    }

    for (i = 0; i < 4; i++)
    {
        mbr_partition_type = mbr_buf[PAR_OFFSET + PAR_TYPE_OFFSET + i * PAR_TABLE_SIZE];
        if (mbr_partition_type)
        {
            info->part[i].type = mbr_partition_type;
            info->part[i].sector_start = LD_DWORD_DISK(&mbr_buf[PAR_OFFSET + PAR_START_OFFSET + i*PAR_TABLE_SIZE]);
            info->part[i].sector_count = LD_DWORD_DISK(&mbr_buf[PAR_OFFSET + PAR_COUNT_OFFSET + i*PAR_TABLE_SIZE]);
            if (mbr_partition_type == EXTENDED_PAR || mbr_partition_type == EXTENDED_8G)
            {
                extended_flag = YES;
                extended_pos = i;
                continue;
            }
            mbr_count++;
        }
    }
    if (extended_flag)
    {
        extended_address = LD_DWORD_DISK(&mbr_buf[PAR_OFFSET + PAR_START_OFFSET + extended_pos*PAR_TABLE_SIZE]);

        do
        {
            memset(ebr_buf, 0, info->sector_size);
            ret = blkdrv->u.i_bops->read(blkdrv, (unsigned char *)ebr_buf,
            extended_address + extended_offset, 1);
            if (ret != 1)
            {
                PRINT_ERR("driver read return error: %d\n", ret);
                goto out_with_mem;
            }
            ebr_partition_type = ebr_buf[PAR_OFFSET + PAR_TYPE_OFFSET];
            if (ebr_partition_type && mbr_count + ebr_count < MAX_DIVIDE_PART_PER_DISK)
            {
                info->part[4 + ebr_count].type = ebr_partition_type;
                info->part[4 + ebr_count].sector_start = extended_address + extended_offset + LD_DWORD_DISK(&ebr_buf[PAR_OFFSET + PAR_START_OFFSET]);
                info->part[4 + ebr_count].sector_count = LD_DWORD_DISK(&ebr_buf[PAR_OFFSET + PAR_COUNT_OFFSET]);
                ebr_count++;
            }
            extended_offset = LD_DWORD_DISK(&ebr_buf[PAR_OFFSET + PAR_START_OFFSET + PAR_TABLE_SIZE]);
        }while (ebr_buf[PAR_OFFSET + PAR_TYPE_OFFSET + PAR_TABLE_SIZE]&& mbr_count + ebr_count < MAX_DIVIDE_PART_PER_DISK);
    }
    ret = mbr_count + ebr_count;

out_with_mem:
    free(ebr_buf);
    free(mbr_buf);
    return ret;
}

int disk_partition_register(los_disk *disk)
{
    int i,count,part_size;
    los_part *part;
    struct disk_divide_info parInfo;

    /* Fill disk_divide_info structure to set partition's infomation. */
    memset(parInfo.part, 0, sizeof(parInfo.part));
    part_size = sizeof(parInfo.part)/sizeof(parInfo.part[0]);

    parInfo.sector_size = disk->sector_size;
    count = disk_partition_recognition(disk->dev, &parInfo);
    if (count < 0)
    {
        return -1;
    }
    parInfo.part_count = count;
    parInfo.sector_count = disk->sector_count;
    if (count == 0)
    {
        part = get_part(disk_add_part(disk, 0, disk->sector_count));
        if (part != NULL)
        {
            part->part_no_mbr = 0;
        }
        else
        {
            return -1;
        }

        PRINTK("No MBR detected.\n");
        return 0;
    }

    for (i = 0; i < part_size; i++)
    {
        /* Read the disk_divide_info structure to get partition's infomation. */
        if (parInfo.part[i].type && parInfo.part[i].type != EXTENDED_PAR && parInfo.part[i].type != EXTENDED_8G)
        {
            part = get_part(disk_add_part(disk, parInfo.part[i].sector_start, parInfo.part[i].sector_count));
            if (part != NULL)
            {
                part->part_no_mbr = i + 1;
                part->filesystem_type = parInfo.part[i].type;
            }
            else
            {
                return -1;
            }
        }
    }

    return 0;
}

int los_disk_read(int pdrv, void *buff, unsigned long long sector, unsigned int count)
{
    unsigned int len    = 0;
    int          result = ENOERR;
    los_disk    *pdisk   = get_disk(pdrv);


    if (!(buff && count)) //buff=NULL or count=0;
        return -1;

    if (pdisk == NULL)
        return -1;

    DISK_LOCK(&pdisk->disk_mutex);

    if (pdisk->disk_status == STAT_UNUSED)
        goto error;

#ifdef LOSCFG_FS_FAT_CACHE
    if (pdisk->bcache)
    {
        len    = pdisk->bcache->sector_size * count;
        result = block_cache_read(pdisk->bcache, buff, &len, 0, sector);
    } else
#endif
    {
        result = pdisk->dev->u.i_bops->read(pdisk->dev, (unsigned char *)buff, sector, count);
        if (result == count)
        {
            result = ENOERR;
        }
    }

    if (result != ENOERR)
        goto error;

    DISK_UNLOCK(&pdisk->disk_mutex);
    return 0;

error:
    DISK_UNLOCK(&pdisk->disk_mutex);
    return -1; /*lint !e438*/
} /*lint !e529*/

int los_disk_write(int pdrv, void *buff, unsigned long long sector, unsigned int count)
{
    unsigned int len    = 0;
    int          result = ENOERR;
    los_disk    *disk   = get_disk(pdrv);

    if(!(buff && count)) //buff=NULL or count=0;
        return -1;

    if (disk == NULL)
        return -1;

    DISK_LOCK(&disk->disk_mutex);

    if(disk->disk_status == STAT_UNUSED)
        goto error;

#ifdef LOSCFG_FS_FAT_CACHE
    if (disk->bcache)
    {
        len    = disk->bcache->sector_size * count;
        result = block_cache_write(disk->bcache, buff, &len, 0, sector);
    } else
#endif
    {
        result = disk->dev->u.i_bops->write(disk->dev, (unsigned char *)buff, sector, count);
        if (result == count)
        {
            result = ENOERR;
        }
    }

    if (result != ENOERR)
        goto error;

    DISK_UNLOCK(&disk->disk_mutex);
    return 0;

error:
    DISK_UNLOCK(&disk->disk_mutex);
    return -1; /*lint !e438*/
}/*lint !e529*/

int los_disk_ioctl(int pdrv, int cmd, void *buff)
{
    struct geometry info;
    los_disk *disk = get_disk(pdrv);

    if ((buff == NULL) || (disk == NULL))
        return -1;

    DISK_LOCK(&disk->disk_mutex);

    if (disk->disk_status == STAT_UNUSED)
        goto error;

    memset(&info, 0, sizeof(info));
    if (disk->dev->u.i_bops->geometry(disk->dev, &info) != 0)
        goto error;

    if (cmd == DISK_GET_SECTOR_COUNT)
    {
        *(unsigned long long *)buff = info.geo_nsectors;
        if (info.geo_nsectors == 0)
            goto error;
    }
    else if (cmd == DISK_GET_SECTOR_SIZE)
    {
        *(size_t *)buff = info.geo_sectorsize;
    }
    else if (cmd == DISK_GET_BLOCK_SIZE) /* Get erase block size in unit of sectors (UINT32) */
    {
        *(size_t *)buff = DISK_MAX_SECTOR_SIZE/info.geo_sectorsize;
        //Block Num SDHC == 512,SD can be set to 512 or other
    }
    else if (cmd == DISK_CTRL_SYNC)
    {
        DISK_UNLOCK(&disk->disk_mutex);
        return 0;//TODO
    }
    else
    {
        goto error;
    }

    DISK_UNLOCK(&disk->disk_mutex);
    return 0;

error:
    DISK_UNLOCK(&disk->disk_mutex);
    return -1;
}

int los_part_read(int pt, void *buff, unsigned long long sector, unsigned int count)
{
    los_part *part = get_part(pt);
    los_disk *disk;
    int ret = 0;

    if (part == NULL)
    {
        return -1;
    }

    disk = get_disk(part->disk_id);
    if (disk == NULL)
    {
        return -1;
    }

    DISK_LOCK(&disk->disk_mutex);
    if (part->dev == NULL || disk->disk_status == STAT_UNUSED)
    {
        goto error;
    }

    if (count > part->sector_count)
    {
        goto error;
    }

    /* Read from absolute sector. */
    if (part->type == EMMC)
    {
        sector += part->sector_start;
    }

    if ((sector >= get_first_part_start(part)) &&
        (sector + count > part->sector_start + part->sector_count || sector < part->sector_start))
    {
        goto error;
    }

    ret = los_disk_read(part->disk_id, buff, sector, count);
    if (ret < 0)
        goto error;

    DISK_UNLOCK(&disk->disk_mutex);
    return 0;

error:
    DISK_UNLOCK(&disk->disk_mutex);
    return -1;
}

int los_part_write(int pt, void *buff, unsigned long long sector, unsigned int count)
{
    los_part *part = get_part(pt);
    los_disk *disk;
    int ret = 0;

    if (part == NULL)
    {
        return -1;
    }

    disk = get_disk(part->disk_id);
    if (disk == NULL)
    {
        return -1;
    }

    DISK_LOCK(&disk->disk_mutex);
    if (part->dev == NULL || disk->disk_status == STAT_UNUSED)
    {
        goto error;
    }

    if (count > part->sector_count)
    {
        goto error;
    }

    /* Write to absolute sector. */
    if (part->type == EMMC)
    {
        sector += part->sector_start;
    }

    if ((sector >= get_first_part_start(part)) &&
        (sector + count > part->sector_start + part->sector_count || sector < part->sector_start))
    {
        goto error;
    }

    ret = los_disk_write(part->disk_id, buff, sector, count);
    if (ret < 0)
        goto error;

    DISK_UNLOCK(&disk->disk_mutex);
    return 0;

error:
    DISK_UNLOCK(&disk->disk_mutex);
    return -1;
}

int los_part_ioctl(int pt, int cmd, void *buff)
{
    struct geometry info;
    los_part *part = get_part(pt);
    los_disk *disk;

    if (part == NULL)
    {
        return -1;
    }

    disk = get_disk(part->disk_id);
    if (disk == NULL)
    {
        return -1;
    }

    DISK_LOCK(&disk->disk_mutex);
    if (part->dev == NULL || disk->disk_status == STAT_UNUSED)
    {
        goto error;
    }

    memset(&info, 0, sizeof(info));
    if (part->dev->u.i_bops->geometry(part->dev, &info) != 0)
        goto error;

    if (cmd == DISK_GET_SECTOR_COUNT)
    {
        *(unsigned long long *)buff = part->sector_count;
        if (*(unsigned long long *)buff == 0)
            goto error;
    }
    else if (cmd == DISK_GET_SECTOR_SIZE)
    {
        *(size_t *)buff = info.geo_sectorsize;
    }
    else if (cmd == DISK_GET_BLOCK_SIZE) /* Get erase block size in unit of sectors (UINT32) */
    {
        if(part->dev->u.i_bops->ioctl(part->dev, 0x2, (unsigned long)buff) != 0)
            goto error;
    }
    else if (cmd == DISK_CTRL_SYNC)
    {
        DISK_UNLOCK(&disk->disk_mutex);
        return 0;//TODO
    }
    else
    {
        goto error;
    }

    DISK_UNLOCK(&disk->disk_mutex);
    return 0;

error:
    DISK_UNLOCK(&disk->disk_mutex);
    return -1;
}

int los_disk_init(const char *disk_name, const struct block_operations *bops,
                void *priv, int disk_id, void *info)
{
    struct geometry disk_info;
    struct inode *blkdriver;
    int ret;
    los_disk     *disk  = get_disk(disk_id);
#ifdef LOSCFG_FS_FAT_CACHE
    los_bcache_t *bc    = (los_bcache_t *)NULL;
    unsigned int sector_per_block;
#endif
    pthread_mutexattr_t attr;

    if (disk_name == NULL || disk == NULL || disk->disk_status == STAT_INUSED)
        return -1;

    if (strlen(disk_name) > DISK_NAME)
        return -1;

    if (register_blockdriver(disk_name, bops, 0755, priv))
    {
        PRINT_ERR("disk_init : register %s fail!\n", disk_name);
        return -1;
    }
    else
    {
        blkdriver = inode_find(disk_name, (const char **)NULL);
        if (blkdriver)
        {
            if (0 == blkdriver->u.i_bops->geometry(blkdriver, &disk_info))
            {
                PRINTK("disk_init : register %s ok!\n", disk_name);
            }
            else
            {
                inode_release(blkdriver);
                unregister_blockdriver(disk_name); /*lint !e534*/
                PRINT_ERR("disk_init : register %s ok buf get disk info fail!\n", disk_name);
                return -1;
            }
        }
        else
        {
            dprintf("disk_init : find %s fail!\n", disk_name);
            unregister_blockdriver(disk_name); /*lint !e534*/
            return -1;
        }
    }

#ifdef LOSCFG_FS_FAT_CACHE
    sector_per_block = g_uwFatSectorsPerBlock / (disk_info.geo_sectorsize / 512);
    bc = block_cache_init((void *)blkdriver, disk_info.geo_sectorsize,
        sector_per_block, g_uwFatBlockNums, disk_info.geo_nsectors / sector_per_block);
    if (bc == NULL)
    {
        PRINT_ERR("disk_init : disk have not init bcache cache!\n");
    }
    else
    {

        bc->preread_fn = (block_cache_preread_fn)NULL; /*lint !e611*/

        if (!get_disk_usb_status(disk_id))
        {
            ret = bcache_async_preread_init(bc);
            if (ret == 0)
            {
                bc->preread_fn = resume_async_preread;
            }
        }

        if (osReHookFuncAddDiskRef) /*lint !e506*/
        {
            (VOID)osReHookFuncAddDiskRef((STORAGE_HOOK_FUNCTION)sd_sync, (void*)0);
            (VOID)osReHookFuncAddDiskRef((STORAGE_HOOK_FUNCTION)sd_sync, (void*)1);
        }

    }
#endif
    (void)pthread_mutexattr_init(&attr);
    attr.type = PTHREAD_MUTEX_RECURSIVE;
    (void)pthread_mutex_init(&disk->disk_mutex, &attr);

    DISK_LOCK(&disk->disk_mutex);
    disk->disk_id      = disk_id;
    disk->dev          = blkdriver;
#ifdef LOSCFG_FS_FAT_CACHE
    disk->bcache         = bc;
#endif
    disk->sector_start = 0;
    disk->sector_size  = disk_info.geo_sectorsize;
    disk->sector_count = disk_info.geo_nsectors;
    strncpy(disk->disk_name, disk_name, sizeof(disk->disk_name));
    disk->disk_name[sizeof(disk->disk_name)-1] = '\0';
    LOS_ListInit(&disk->head);
    disk->disk_status  = STAT_INUSED;

    inode_release(blkdriver);

    if (info != NULL)
    {
        ret = disk_divide(disk, (struct disk_divide_info *)info);
        if(ret != ENOERR)
        {
            goto error;
        }
    }
    else
    {
        ret = disk_partition_register(disk);
        if(ret != ENOERR)
        {
            goto error;
        }
    }

    DISK_UNLOCK(&disk->disk_mutex);
    return ENOERR;

 error:
    DISK_UNLOCK(&disk->disk_mutex);
    los_disk_deinit(disk->disk_id); /*lint !e534*/
    return -1;
}/*lint !e529*/

int los_disk_deinit(int disk_id)
{
    los_disk *pstDisk = get_disk(disk_id);
    los_part *pstPart;
    char dev_name[30];
    UINTPTR uvIntSave;
    unsigned int ret = 0;

    if (!pstDisk)
        return -EINVAL;

    DISK_LOCK(&pstDisk->disk_mutex);

    if (pstDisk->disk_status == STAT_UNUSED)
    {
        DISK_UNLOCK(&pstDisk->disk_mutex);
        return -EINVAL;
    }

    pstDisk->disk_status = STAT_DELETE;

    if (!LOS_ListEmpty(&pstDisk->head))
    {
        pstPart = LOS_DL_LIST_ENTRY(pstDisk->head.pstNext, los_part, list); /*lint !e413*/
        while (&pstPart->list != &pstDisk->head)
        {
            (void)snprintf(dev_name, sizeof(dev_name) ,"%s%c%d", pstDisk->disk_name, 'p', pstDisk->part_count -1); /*lint !e534*/
            disk_part_del_from_disk(pstDisk, pstPart);
            unregister_blockdriver(dev_name); /*lint !e534*/
            disk_part_release(pstPart);

            pstPart = LOS_DL_LIST_ENTRY(pstDisk->head.pstNext, los_part, list); /*lint !e413*/
        }
    }

    uvIntSave = LOS_IntLock();
    pstDisk->disk_status  = STAT_UNUSED;
#ifdef LOSCFG_FS_FAT_CACHE
    if (!get_disk_usb_status(disk_id))
    {
        ret = bcache_async_preread_deinit(pstDisk->bcache);
        if (ret != 0)
        {
            PRINT_ERR("Blib async preread deinit failed in %s, %d\n",__FUNCTION__, __LINE__);
        }
    }
    block_cache_deinit(pstDisk->bcache);
    pstDisk->bcache         = (los_bcache_t *)NULL;
    if(osReHookFuncDelDiskRef) /*lint !e506*/
    {
        (VOID)osReHookFuncDelDiskRef((STORAGE_HOOK_FUNCTION)sd_sync);
    }
#endif

    pstDisk->dev = (struct inode *)NULL;
    LOS_IntRestore(uvIntSave);

    unregister_blockdriver(pstDisk->disk_name); /*lint !e534*/
    DISK_UNLOCK(&pstDisk->disk_mutex);
    ret = pthread_mutex_destroy(&pstDisk->disk_mutex);
    if (ret != 0)
    {
        PRINT_ERR("%s %d, mutex destroy failed\n", __FUNCTION__, __LINE__);
        return -EFAULT;
    }

    return ENOERR;
}

int los_disk_sync(int pdrv)
{
    los_disk *pstDisk = get_disk(pdrv);
    int ret = ENOERR;

    if (pstDisk == NULL)
    {
        return EINVAL;
    }

    DISK_LOCK(&pstDisk->disk_mutex);
    if (pstDisk->disk_status == STAT_UNUSED)
    {
        DISK_UNLOCK(&pstDisk->disk_mutex);
        return EINVAL;
    }

#ifdef LOSCFG_FS_FAT_CACHE
    if (pstDisk->bcache)
    {
        ret = block_cache_sync(pstDisk->bcache);
    }
#endif

    DISK_UNLOCK(&pstDisk->disk_mutex);
    return ret;
}

/*
 * parameter
 * sector_per_block:sectors num in one block, only be 0\32\64\96\128;
 *                  blocksize = sector_per_block * sectorsize;
 * block_num : blocks num in in bcache,memsize = blocknum * blocksize;
 * return value : ENOERR or other errno;
 */
int los_disk_set_bcache(int pdrv, unsigned int sector_per_block, unsigned int block_num)
{
#ifdef LOSCFG_FS_FAT_CACHE
    int ret = ENOERR;
    UINTPTR uvIntSave;
    los_bcache_t *bc, *newbl= (los_bcache_t *)NULL;
    los_disk *pstDisk = get_disk(pdrv);

    if (pstDisk == NULL)
    {
        return EINVAL;
    }

    DISK_LOCK(&pstDisk->disk_mutex);

    if (pstDisk->disk_status == STAT_UNUSED)
        goto error;

    /* because we use int flag[4] in bcache for sectors bitmap tag, so it only
        can be 32 * (0,1,2,3,4)*/
    if (sector_per_block % 32 != 0 ||  sector_per_block / 32  > 4)
        goto error;

    bc = pstDisk->bcache;
    pstDisk->bcache = (los_bcache_t *)NULL;

    ret = los_disk_sync(pdrv);
    if (ret != ENOERR && bc != NULL)
    {
        free(bc->mem_base);
        free(bc->rw_buffer);
        memset(bc, 0, sizeof(los_bcache_t));
        free(bc);
        DISK_UNLOCK(&pstDisk->disk_mutex);
        return ret;
    }

    uvIntSave = LOS_IntLock();
    if (bc)
    {
        free(bc->mem_base);
        free(bc->rw_buffer);
        memset(bc, 0, sizeof(los_bcache_t));
        free(bc);
    }

    newbl = block_cache_init((void *)pstDisk->dev, pstDisk->sector_size,
        sector_per_block, block_num, pstDisk->sector_count / sector_per_block);
    if (newbl == NULL && (sector_per_block * block_num) != 0)
    {
        LOS_IntRestore(uvIntSave);
        DISK_UNLOCK(&pstDisk->disk_mutex);
        return ENOMEM;
    }

    pstDisk->bcache = newbl;
    LOS_IntRestore(uvIntSave);
    DISK_UNLOCK(&pstDisk->disk_mutex);
    return ENOERR;

error:
    DISK_UNLOCK(&pstDisk->disk_mutex);
    return EINVAL;
#endif

    return -1; /*lint !e527*/
}

los_part *los_part_find(struct inode *blkdriver)
{
    int i;
    los_disk *pstDisk;
    los_part *pstPart;

    if(blkdriver == NULL)
    {
        return (los_part *)NULL;
    }

    for (i = 0; i < SYS_MAX_DISK; i++)
    {
        pstDisk = get_disk(i);
        if (pstDisk == NULL)
            continue;

        DISK_LOCK(&pstDisk->disk_mutex);
        if ((pstDisk->disk_status == STAT_INUSED || pstDisk->disk_status == STAT_DELETE) && !LOS_ListEmpty(&pstDisk->head))
        {
            pstPart = LOS_DL_LIST_ENTRY(pstDisk->head.pstNext, los_part, list); /*lint !e413*/
            if (pstDisk->dev == blkdriver)
            {
                DISK_UNLOCK(&pstDisk->disk_mutex);
                return pstPart;
            }

            while (&pstPart->list != &pstDisk->head)
            {
                if (pstPart->dev == blkdriver)
                {
                    DISK_UNLOCK(&pstDisk->disk_mutex);
                    return pstPart;
                }
                pstPart = LOS_DL_LIST_ENTRY(pstPart->list.pstNext, los_part, list); /*lint !e413*/
            }
        }
        else
        {
            DISK_UNLOCK(&pstDisk->disk_mutex);
            continue;
        }
        DISK_UNLOCK(&pstDisk->disk_mutex);
    }

    return (los_part *)NULL;
}

int los_part_access(const char *dev, mode_t mode)
{
    los_part *part = (los_part *)NULL;
    struct inode *node = (struct inode *)NULL;

    node = inode_find(dev, (const char **)NULL);
    if(node == NULL)
    {
        return -1;
    }

    part = los_part_find(node);
    inode_release(node);
    if(part == NULL)
    {
        return -1;
    }

    return 0;
}

int los_part_cpyName(los_part *pstPart,const char *src)
{
    int len;

    len = strlen(src);

    pstPart->part_name = (char *)zalloc(len + 1);
    if(pstPart->part_name == NULL)
    {
        PRINT_ERR("%s[%d] malloc failure\n", __FUNCTION__, __LINE__);/*lint !e40*/
        return -1;
    }

    strncpy(pstPart->part_name,src,len);
    return 0;
}

int add_mmc_partition(struct disk_divide_info *info, size_t sector_start, size_t sector_count)
{
    struct disk_divide_info *pstInfo;
    int index;
    unsigned long long start,end;
    int i;

    if (info == NULL)
    {
        return -1;
    }

    pstInfo = info;
    if (pstInfo->part_count > MAX_DIVIDE_PART_PER_DISK || sector_count == 0)
    {
        return -1;
    }

    start = sector_start;
    end   = sector_start + sector_count;

    index = pstInfo->part_count;
    for (i = 0; i < index; i++)
    {
        if (start < pstInfo->part[i].sector_start + pstInfo->part[i].sector_count)
        {
            return -1;
        }

        if (end > info->sector_count)
        {
            return -1;
        }
    }

    if (index >= MAX_DIVIDE_PART_PER_DISK)
    {
        return -1;
    }
    pstInfo->part[index].sector_start = sector_start;
    pstInfo->part[index].sector_count = sector_count;
    pstInfo->part[index].type = EMMC;
    pstInfo->part_count++;

    return 0;
}

void show_part(los_part *part)
{
    if (part == NULL || part->dev == NULL)
    {
        PRINT_ERR("part is NULL\n");
        return;
    }

    PRINTK("\npart info :\n");
    PRINTK("disk id          : %d\n", part->disk_id);
    PRINTK("part_id in system: %d\n", part->part_id);
    PRINTK("part no in disk  : %d\n", part->part_no_disk);
    PRINTK("part no in mbr   : %d\n", part->part_no_mbr);
    PRINTK("part filesystem  : %02X\n", part->filesystem_type);
    PRINTK("part dev name    : %s\n", part->dev->i_name);
    PRINTK("part sec start   : %llu\n", part->sector_start);
    PRINTK("part sec count   : %llu\n", part->sector_count);

}

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */


