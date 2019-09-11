/*----------------------------------------------------------------------------
 * Copyright (c) <2013-2015>, <Huawei Technologies Co., Ltd>
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific prior written
 * permission.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *---------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------
 * Notice of Export Control Law
 * ===============================================
 * Huawei LiteOS may be subject to applicable export control laws and regulations, which might
 * include those applicable to Huawei LiteOS of U.S. and the country in which you are located.
 * Import, export and usage of Huawei LiteOS in any manner by you shall be in compliance with such
 * applicable export control laws and regulations.
 *---------------------------------------------------------------------------*/
#include "linux/module.h"
#include "linux/device.h"
#include "linux/platform_device.h"
#include "stdio.h"
#include "inode/inode.h"
#ifdef LOSCFG_FS_VFS
#include "fs/fs.h"
#include "spi.h"
#include "spi_dev.h"

#include "los_mux.h"

/*
 * spidev_open: open spi devide
 */
static int spidev_open(struct file *filep)
{
    struct inode *inode = filep->f_inode ;
    int cs_index;

    cs_index = ((struct spi_dev *)(inode->i_private))->cs_index;
    ((struct spi_dev *)(inode->i_private))->cur_sdd->cur_cs = cs_index;

    return ENOERR;
}

/*
 * spidev_ioctrl: ioctrl spi device interface
 */
static int spidev_ioctl(struct file * filep, int cmd, unsigned long arg)
{
    int ret = ENOERR;
    unsigned int tmp = 0;
    struct spi_ioc_transfer * transfer;
    struct inode * inode = filep -> f_inode ;
    struct spi_driver_data *sdd =
            ((struct spi_dev *)(inode->i_private))->cur_sdd;

    (void)LOS_MuxPend(sdd->lock, LOS_WAIT_FOREVER);
    switch(cmd) {
        /* read requests */
        case SPI_IOC_RD_MODE:
            *(unsigned int *)arg = sdd->cur_mode;
            break;
        case SPI_IOC_RD_LSB_FIRST:
            spi_err("Not support cmd(%0d)!!!\n", cmd);
            ret = -EINVAL;
            break;
        case SPI_IOC_RD_BITS_PER_WORD:
            *(unsigned int *)arg = sdd->cur_bits_per_word;
            break;
        case SPI_IOC_RD_MAX_SPEED_HZ:
            *(unsigned long *)arg = sdd->max_speed;
            break;
        /* write requests */
        case SPI_IOC_WR_MODE:
            tmp = *(unsigned int *) arg;
            sdd->cur_mode = tmp;
            break;
        case SPI_IOC_WR_LSB_FIRST:
            spi_err("Not support cmd(%0d)!!!\n", cmd);
            ret = -EINVAL;
            break;
        case SPI_IOC_WR_BITS_PER_WORD:
            tmp = *(unsigned int *) arg;
            sdd->cur_bits_per_word = tmp;
            break;
        case SPI_IOC_WR_MAX_SPEED_HZ:
            tmp = *(unsigned int *) arg;
            sdd->max_speed = tmp;
            break;
        case SPI_IOC_MESSAGE(N):
            transfer = (struct spi_ioc_transfer *) arg;
            sdd->cur_cs = ((struct spi_dev *)(inode->i_private))->cs_index;
            ret = spi_transfer(sdd, transfer);
            if (ret != transfer -> len) {
                spi_err("fail to ioctl! \n");
                ret = -EIO;
            }
            break;
        default:
            spi_err("Not support cmd(%0d)!!!\n", cmd);
            ret = -EINVAL;
            break;
    }
    (void)LOS_MuxPost(sdd->lock);

    return ret;
}

struct file_operations_vfs spidev_ops =
{
    spidev_open,
    NULL,
    NULL,
    NULL,
    NULL,
    spidev_ioctl,
#ifndef CONFIG_DISABLE_POLL
    NULL,
#endif
    NULL
};

#endif /*end of LOSCFG_FS_VFS*/
#ifdef LOSCFG_FS_VFS
extern struct file_operations_vfs spidev_ops;
#endif
static struct spi_dev *pstSpiDev;
extern int spi_intf_init(struct spi_driver_data *sdd);
extern int spi_set_platdata(struct spi_platform_data *spd, int id);
extern int spi_init_cfg(struct spi_driver_data *sdd);
static int hispi_probe(struct platform_device *pdev)
{
    char dev_path[50] = {0};
    int i;
    int ret = -1;
    struct spi_platform_data *spd = (struct spi_platform_data *)dev_get_platdata(&pdev->dev);
    struct spi_driver_data *sdd = (struct spi_driver_data *)dev_get_drvdata(&pdev->dev);
    if(!spd){
        spi_err("get platdata fail!");
        return ret;
    }
    if(!sdd){
        spi_err("get drvdata fail!");
        return ret;
    }
    if(spi_intf_init(sdd)){
        spi_err("register spi operation fail!");
        goto err;
    }
    if (spi_set_platdata(spd, sdd->bus_num)){
        spi_err("set platform data fail for host[%0d]!!!", sdd->bus_num);
        goto err;
    }

    sdd->spd = spd;
    sdd->max_speed = (spd->clk_rate) / ((SCR_MIN + 1) * CPSDVSR_MIN);
    sdd->min_speed = (spd->clk_rate) / ((SCR_MAX + 1) * CPSDVSR_MAX);
    if (spi_init_cfg(sdd)) {    /* Setup Deufult Mode */
        spi_err("spi init fail for host[%0d]!!!", sdd->bus_num);
        return ret;
    }

    for(i = 0;i < spd->num_cs;i++)
    {
        pstSpiDev = zalloc(sizeof(struct spi_dev));
        if(!pstSpiDev) {
            spi_err("no mem for spidev!");
            return ret;
        }
        pstSpiDev->cs_index = i;
        pstSpiDev->cur_sdd = sdd;
#ifdef LOSCFG_FS_VFS
        memset(dev_path,0,sizeof(dev_path));
        snprintf(dev_path, sizeof(dev_path), "/dev/spidev%u.%u", sdd->bus_num, i);
        ret =  register_driver(dev_path, &spidev_ops, 0755, pstSpiDev);
        if(ret){
            spi_err("gen %s fail!",dev_path);
            goto err;
        }
        if(i==0){
            ret = LOS_MuxCreate((UINT32 *)(&pstSpiDev->cur_sdd->lock));
            if(ret){
                spi_err("create lock for spi host[%d] fail!",i);
                ret = unregister_driver(dev_path);
                if(!ret)
                {
                     spi_err("unregister_driver fail, ret= %d", ret);
                }
                goto err;
            }
        }
#endif
    }
    return 0;
err:
   free(pstSpiDev);
   pstSpiDev = NULL;
   return -1;
}
static int hispi_suspend(struct device *dev)
{
    return 0;
}
static int hispi_resume(struct device *dev)
{
    return 0;
}
static const struct dev_pm_op hispi_dev_pm_ops = {
    .suspend = hispi_suspend,
    .resume = hispi_resume,
};
static int hispi_remove(struct platform_device *dev)
{
    char dev_path[50] = {0};
    struct spi_platform_data *spd = (struct spi_platform_data *)dev_get_platdata(&dev->dev);
    struct spi_driver_data *sdd = (struct spi_driver_data *)dev_get_drvdata(&dev->dev);
    struct inode *spi_inode;
    struct spi_dev * spiDev;
    int i,ret;
    for(i = 0;i < spd->num_cs;i++)
    {
        memset(dev_path,0,sizeof(dev_path));
        snprintf(dev_path, sizeof(dev_path), "/dev/spidev%u.%u", sdd->bus_num, i);
        spi_inode = inode_find(dev_path,NULL);
        if(!spi_inode){
            spi_err("unregister %s  fail!",dev_path);
            goto err;
        }
        spiDev = (struct spi_dev *)(spi_inode->i_private);
        if(!spiDev){
            spi_err("unregister %s fail!",dev_path);
            goto err;
        }
        ret =  unregister_driver(dev_path);
        if(ret){
            spi_err("unregister %s fail!",dev_path);
            goto err;
        }
        free(spiDev);
    }
    return 0;
err:
   return -1;
}
static struct platform_driver hispi_driver = {
    .probe      = hispi_probe,
    .remove     = hispi_remove,
    .driver     = {
        .name   = "spidev",
        .pm = &hispi_dev_pm_ops,
    },
};
int hispi_init(void)
{
    return platform_driver_register(&hispi_driver);
}

void hispi_exit(void)
{
    platform_driver_unregister(&hispi_driver);
}

module_init(hispi_init);
module_exit(hispi_exit);
int spi_dev_init(void)
{
  return hispi_init();
}

