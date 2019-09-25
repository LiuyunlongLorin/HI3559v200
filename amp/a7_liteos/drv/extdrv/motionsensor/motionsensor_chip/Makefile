ifeq ($(EXTDRV_PARAM_FILE), ) 
    EXTDRV_PARAM_FILE:=../Makefile.param
    include $(EXTDRV_PARAM_FILE)
endif

# Sensor Type: ICM20690
SENSOR_TYPE ?= ICM20690

# Transfer Type: I2C SPI
export TRANSFER_TYPE := SPI

# ICM20690 proc info function : y or n
export ICM20690_PROC = n


#SRCS = $(wildcard *.c)
SRCS = ./Main/motionsensor.c
##SRCS += ./proc/motionsensor_proc.c


INC := -I$(REL_INC)
INC += -I$(OSAL_ROOT)/include
INC += -I$(CBB_ROOT)/include
#debug
INC += -I$(CBB_ROOT)/vi/ext_inc
ifeq ($(OSTYPE), liteos)
INC += -I$(LITEOSTOPDIR)/drivers/gpio/include
endif
ifeq ($(MPP_BUILD),y)
    INC += -I$(DRV_ROOT)/extdrv/$(HIARCH)/motionsensor/motionsensor_chip/Main/
    ##INC += -I$(DRV_ROOT)/extdrv/$(HIARCH)/motionsensor/motionsensor_chip/proc/
    INC += -I$(DRV_ROOT)/extdrv/$(HIARCH)/motionsensor/motionsensor_chip/pub/
    INC += -I$(DRV_ROOT)/extdrv/$(HIARCH)/motionsensor/motionsensor_mng/include/
    INC += -I$(DRV_ROOT)/extdrv/$(HIARCH)/motionsensor/include/
else
    INC += -I$(DRV_ROOT)/extdrv/motionsensor/motionsensor_chip/Main/
    ##INC += -I$(DRV_ROOT)/extdrv/motionsensor/motionsensor_chip/proc/
    INC += -I$(DRV_ROOT)/extdrv/motionsensor/motionsensor_chip/pub/
    INC += -I$(DRV_ROOT)/extdrv/motionsensor/motionsensor_mng/include/
    INC += -I$(DRV_ROOT)/extdrv/motionsensor/include/
endif

ifeq ($(ICM20690_PROC),y)
	ccflags-y += -DICM20690_PARAM_PROC
	SRCS += ./proc/motionsensor_proc.c
ifeq ($(MPP_BUILD),y)
	INC += -I$(DRV_ROOT)/extdrv/$(HIARCH)/motionsensor/motionsensor_chip/proc/
else
	INC += -I$(DRV_ROOT)/extdrv/motionsensor/motionsensor_chip/proc/
endif
endif

ifeq ($(SENSOR_TYPE), ICM20690)
    SRCS += ./SensorDev/ICM20690/icm20690.c
	SRCS += ./SensorDev/ICM20690/motionsensorgpio.c
ifeq ($(MPP_BUILD),y)
    INC += -I$(DRV_ROOT)/extdrv/$(HIARCH)/motionsensor/motionsensor_chip/SensorDev/ICM20690/
else
    INC += -I$(DRV_ROOT)/extdrv/motionsensor/motionsensor_chip/SensorDev/ICM20690/
endif
endif

ifeq ($(SENSOR_TYPE), LSM6dsm)
    SRCS += ./SensorDev/LSM6dsm/lsm6dsm.c
	SRCS += ./SensorDev/LSM6dsm/motionsensorgpio.c
ifeq ($(MPP_BUILD),y)
    INC += -I$(DRV_ROOT)/extdrv/$(HIARCH)/motionsensor/motionsensor_chip/SensorDev/LSM6dsm/
else
    INC += -I$(DRV_ROOT)/extdrv/motionsensor/motionsensor_chip/SensorDev/LSM6dsm/
endif
endif



ifeq ($(TRANSFER_TYPE), I2C)
    SRCS += ./TransferType/i2c/i2c_dev.c
	ccflags-y += -DTRANSFER_I2C
ifeq ($(MPP_BUILD),y)
    INC += -I$(DRV_ROOT)/extdrv/$(HIARCH)/motionsensor/motionsensor_chip/TransferType/i2c/
else
    INC += -I$(DRV_ROOT)/extdrv/motionsensor/motionsensor_chip/TransferType/i2c/
endif
    INC += -I$(TRANSFER_DIR)/i2c
endif

ifeq ($(TRANSFER_TYPE), SPI)
    SRCS += ./TransferType/spi/spi_dev.c
	ccflags-y += -DTRANSFER_SPI
ifeq ($(MPP_BUILD),y)
    INC += -I$(DRV_ROOT)/extdrv/$(HIARCH)/motionsensor/motionsensor_chip/TransferType/spi/
else
    INC += -I$(DRV_ROOT)/extdrv/motionsensor/motionsensor_chip/TransferType/spi/
endif
    INC += -I$(TRANSFER_DIR)/spi
endif

ifeq ($(MPP_BUILD),y)
        INC += -I$(OSAL_ROOT)/$(OSTYPE)/kernel/himedia
else
        INC += -I$(OSAL_ROOT)/source/kernel/himedia
endif
EXTDRV_CFLAGS += $(INC)

#*************************************************************************
TARGET := motionsensor_chip

#*************************************************************************
# compile linux or HuaweiLite
include $(PWD)/../../Make.$(OSTYPE)
