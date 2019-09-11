#ifndef __SPI_DEV_H__
#define __SPI_DEV_H__

#include "hi_type.h"
#include "hi_osal.h"

#ifdef __HuaweiLite__
#include <spi.h>
#include "fcntl.h"
#else
#include <linux/spi/spi.h>
#endif

#ifndef __HuaweiLite__
HI_S32 MotionSersor_SPI_write(struct spi_device* hi_spi, HI_U8 addr, HI_U8* data, HI_U32 u32cnt);
HI_S32 MotionSersor_SPI_read(struct spi_device* hi_spi, HI_U8 addr, HI_U8* data, HI_U32 u32cnt);
HI_S32 MotionSersor_SPI_init(struct spi_device **hi_spi);
HI_S32 MotionSersor_SPI_deinit(struct spi_device* spi_device);
#else
HI_S32 MotionSersor_SPI_write(HI_U8 addr, HI_U8* data, HI_U32 u32cnt, HI_U32 u32SpiNum);
HI_S32 MotionSersor_SPI_read(HI_U8 addr, HI_U8* data, HI_U32 u32cnt, HI_U32 u32SpiNum);
HI_S32 MotionSersor_SPI_init(HI_VOID);
HI_S32 MotionSersor_SPI_deinit(HI_VOID);
#endif

#endif

