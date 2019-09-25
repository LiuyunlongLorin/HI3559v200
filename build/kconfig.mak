ifeq ($(SDK_KCONFIG),)
SDK_KCONFIG = 1

# Check Parameter
ifeq ($(SDK_ROOT),)
    $(error please specify sdk root path)
endif

##########################################################################################
#	Kconfig Common Configure
##########################################################################################
KCONFIG_CONFIG   ?= $(SDK_ROOT)/.config
ifeq ($(KCONFIG_CONFIG), $(wildcard $(KCONFIG_CONFIG)))
include $(KCONFIG_CONFIG)
endif
KCONFIG_CFLAGS   :=

#++++++++++++++++++++++++++++++++++++++++++++++++++++
#	Chip Type
#++++++++++++++++++++++++++++++++++++++++++++++++++++
# AMP_TYPE: linux_liteos        /linux    /liteos
#           AMP_LINUX_HUAWEILITE/AMP_LINUX/AMP_HUAWEILITE
ifeq ($(CONFIG_HI3556AV100), y)
CFG_CHIP_TYPE    := hi3556av100
CFG_SUBCHIP_TYPE := hi3556av100
AMP_TYPE         := linux_liteos
KCONFIG_CFLAGS   += -DHI3556AV100 -D__HI3556AV100__
KCONFIG_CFLAGS   += -DAMP_LINUX_HUAWEILITE
else ifeq ($(CONFIG_HI3519AV100), y)
CFG_CHIP_TYPE    := hi3519av100
CFG_SUBCHIP_TYPE := hi3519av100
AMP_TYPE         := linux_liteos
KCONFIG_CFLAGS   += -DHI3519AV100 -D__HI3519AV100__
KCONFIG_CFLAGS   += -DAMP_LINUX_HUAWEILITE
else ifeq ($(CONFIG_HI3559AV100), y)
CFG_CHIP_TYPE    := hi3559av100
CFG_SUBCHIP_TYPE := hi3559av100
AMP_TYPE         := linux_liteos
KCONFIG_CFLAGS   += -DHI3559AV100 -D__HI3559AV100__
KCONFIG_CFLAGS   += -DAMP_LINUX_HUAWEILITE
else ifeq ($(CONFIG_HI3559V200), y)
CFG_CHIP_TYPE    := hi3559v200
CFG_SUBCHIP_TYPE := hi3559v200
AMP_TYPE         := linux_liteos
KCONFIG_CFLAGS   += -DHI3559V200 -D__HI3559V200__
KCONFIG_CFLAGS   += -DAMP_LINUX_HUAWEILITE
else ifeq ($(CONFIG_HI3556V200), y)
CFG_CHIP_TYPE    := hi3559v200
CFG_SUBCHIP_TYPE := hi3556v200
AMP_TYPE         := linux_liteos
KCONFIG_CFLAGS   += -DHI3559V200 -D__HI3559V200__ -DHI3556V200 -D__HI3556V200__
KCONFIG_CFLAGS   += -DAMP_LINUX_HUAWEILITE
else ifeq ($(CONFIG_HI3518EV300), y)
CFG_CHIP_TYPE    := hi3518ev300
CFG_SUBCHIP_TYPE := hi3518ev300
AMP_TYPE         := liteos
KCONFIG_CFLAGS   += -DHI3518EV300 -D__HI3518EV300__
KCONFIG_CFLAGS   += -DAMP_HUAWEILITE
endif

#++++++++++++++++++++++++++++++++++++++++++++++++++++
#	Cross ToolChain
#++++++++++++++++++++++++++++++++++++++++++++++++++++
ifeq ($(CONFIG_LINUX_CROSS_HIMIX200), y)
export LINUX_CROSS      := arm-himix200-linux
LIBC_TYPE        := glibc
KCONFIG_CFLAGS   += -D__HI_GLIBC__
else ifeq ($(CONFIG_LINUX_CROSS_HIMIX100), y)
export LINUX_CROSS      := arm-himix100-linux
LIBC_TYPE        := uclibc
KCONFIG_CFLAGS   += -D__HI_UCLIBC__
else ifeq ($(CONFIG_LINUX_CROSS_ARCH64_HIMIX100), y)
export LINUX_CROSS      := aarch64-himix100-linux
LIBC_TYPE        := glibc
KCONFIG_CFLAGS   += -D__HI_GLIBC__
LINUX_ARCH64     := y
endif

ifeq ($(CONFIG_HUAWEILITE_CROSS_HIMIX100), y)
HUAWEILITE_CROSS := arm-himix100-linux
LIBC_TYPE        := uclibc
KCONFIG_CFLAGS   += -D__UCLIBC__
else ifeq ($(CONFIG_HUAWEILITE_CROSS_HIMIX200), y)
HUAWEILITE_CROSS := arm-himix200-linux
else ifeq ($(CONFIG_HUAWEILITE_CROSS_ARCH64_HIMIX100), y)
HUAWEILITE_CROSS := aarch64-himix100-linux
HUAWEILITE_ARCH64:= y
endif

#++++++++++++++++++++++++++++++++++++++++++++++++++++
#	Flash Configure
#++++++++++++++++++++++++++++++++++++++++++++++++++++
ifeq ($(CONFIG_SPINOR), y)
FLASH_TYPE       := spinor
BOOT_MEDIA       := spi
KCONFIG_CFLAGS   += -DCFG_SPI_NOR_FLASH
else ifeq ($(CONFIG_SPINAND), y)
FLASH_TYPE       := spinand
ifeq ($(CFG_CHIP_TYPE), hi3559v200)
BOOT_MEDIA       := nand
else
BOOT_MEDIA       := spi
endif
KCONFIG_CFLAGS   += -DCFG_SPI_NAND_FLASH
else ifeq ($(CONFIG_NAND), y)
FLASH_TYPE       := nand
BOOT_MEDIA       := nand
KCONFIG_CFLAGS   += -DCFG_NAND_FLASH
else ifeq ($(CONFIG_EMMC), y)
FLASH_TYPE       := emmc
BOOT_MEDIA       := emmc
KCONFIG_CFLAGS   += -DCFG_EMMC_FLASH
endif

ifeq ($(CONFIG_FLASH_PAGESIZE_512), y)
RAMPARAM_PAGESIZE    := 512
YAFFS_PAGESIZE_PARAM := 0
else ifeq ($(CONFIG_FLASH_PAGESIZE_2K), y)
RAMPARAM_PAGESIZE    := 2048
YAFFS_PAGESIZE_PARAM := 1
else ifeq ($(CONFIG_FLASH_PAGESIZE_4K), y)
RAMPARAM_PAGESIZE    := 4096
YAFFS_PAGESIZE_PARAM := 2
else ifeq ($(CONFIG_FLASH_PAGESIZE_8K), y)
RAMPARAM_PAGESIZE    := 8192
YAFFS_PAGESIZE_PARAM := 3
else ifeq ($(CONFIG_FLASH_PAGESIZE_16K), y)
RAMPARAM_PAGESIZE    := 16384
YAFFS_PAGESIZE_PARAM := 4
endif
ifeq ($(CONFIG_SPINOR), y)
KCONFIG_CFLAGS   += -DCFG_FLASH_PAGESIZE=1
else ifeq ($(CONFIG_EMMC), y)
KCONFIG_CFLAGS   += -DCFG_FLASH_PAGESIZE=1
else
KCONFIG_CFLAGS   += -DCFG_FLASH_PAGESIZE=$(RAMPARAM_PAGESIZE)
endif

ifeq ($(CONFIG_FLASH_ECCTYPE_1BIT), y)
YAFFS_ECCTYPE_PARAM := 1
else ifeq ($(CONFIG_FLASH_ECCTYPE_4BIT), y)
YAFFS_ECCTYPE_PARAM := 2
else ifeq ($(CONFIG_FLASH_ECCTYPE_8BIT), y)
YAFFS_ECCTYPE_PARAM := 3
else ifeq ($(CONFIG_FLASH_ECCTYPE_24BIT), y)
YAFFS_ECCTYPE_PARAM := 4
else ifeq ($(CONFIG_FLASH_ECCTYPE_40BIT), y)
YAFFS_ECCTYPE_PARAM := 5
else ifeq ($(CONFIG_FLASH_ECCTYPE_64BIT), y)
YAFFS_ECCTYPE_PARAM := 6
else
YAFFS_ECCTYPE_PARAM := 0
endif

#++++++++++++++++++++++++++++++++++++++++++++++++++++
#	Product Type
#++++++++++++++++++++++++++++++++++++++++++++++++++++
ifeq ($(CONFIG_ACTIONCAM), y)
CFG_PDT_TYPE     := actioncam
KCONFIG_CFLAGS += -DCFG_LITEOS_NO_PRE_RECORD
else ifeq ($(CONFIG_DASHCAM), y)
CFG_PDT_TYPE     := dashcam
KCONFIG_CFLAGS += -DCONFIG_DASHCAM
else ifeq ($(CONFIG_PANOCAM), y)
CFG_PDT_TYPE     := panocam
KCONFIG_CFLAGS   +=-DSUPPORT_STITCH
else ifeq ($(CONFIG_SAMPLECAM), y)
CFG_PDT_TYPE     := samplecam
endif

#++++++++++++++++++++++++++++++++++++++++++++++++++++
#	DDR Type
#++++++++++++++++++++++++++++++++++++++++++++++++++++
ifeq ($(CONFIG_DDR_BITWIDTH_16), y)
KCONFIG_CFLAGS += -DDDR_BITWIDTH_16
endif

#++++++++++++++++++++++++++++++++++++++++++++++++++++
#	Board Type
#++++++++++++++++++++++++++++++++++++++++++++++++++++
ifeq ($(CONFIG_DEMB), y)
CFG_BOARD_TYPE   := demb
KCONFIG_CFLAGS   += -DBOARD_DEMB
else ifeq ($(CONFIG_REFB_ACTIONCAM), y)
CFG_BOARD_TYPE   := refb
KCONFIG_CFLAGS   += -DBOARD_ACTIONCAM_REFB
else ifeq ($(CONFIG_REFB_T1_ACTIONCAM), y)
CFG_BOARD_TYPE   := refb_t1
KCONFIG_CFLAGS   += -DBOARD_ACTIONCAM_REFB1
else ifeq ($(CONFIG_REFB_DASHCAM), y)
CFG_BOARD_TYPE   := refb
KCONFIG_CFLAGS   += -DBOARD_DASHCAM_REFB
else ifeq ($(CONFIG_REFB_PANOCAM), y)
CFG_BOARD_TYPE   := refb
KCONFIG_CFLAGS   += -DBOARD_PANOCAM_REFB
endif

ifeq ($(CFG_CHIP_TYPE), hi3556av100)
KCONFIG_CFLAGS   += -DPWR_USER_REG0=0x04590048
KCONFIG_CFLAGS   += -DPWR_USER_REG1=0x0459004c
KCONFIG_CFLAGS   += -DPWR_USER_REG2=0x04590050
KCONFIG_CFLAGS   += -DPWR_USER_REG3=0x04590054
KCONFIG_CFLAGS   += -DTIMERX_REG_BASE=0x04500000
KCONFIG_CFLAGS   += -DTIME_STAMP_BASE_ADDR=$(shell awk 'BEGIN{printf("%d",'$(CONFIG_MEM_PARAM_BASE)'+'$(CONFIG_MEM_PARAM_SIZE)'-0x10000)}')
else ifeq ($(CFG_CHIP_TYPE), hi3519av100)
KCONFIG_CFLAGS   += -DPWR_USER_REG0=0x04590048
KCONFIG_CFLAGS   += -DPWR_USER_REG1=0x0459004c
KCONFIG_CFLAGS   += -DPWR_USER_REG2=0x04590050
KCONFIG_CFLAGS   += -DPWR_USER_REG3=0x04590054
KCONFIG_CFLAGS   += -DTIMERX_REG_BASE=0x04500000
KCONFIG_CFLAGS   += -DTIME_STAMP_BASE_ADDR=$(shell awk 'BEGIN{printf("%d",'$(CONFIG_MEM_PARAM_BASE)'+'$(CONFIG_MEM_PARAM_SIZE)'-0x10000)}')
else ifeq ($(CFG_CHIP_TYPE), hi3559av100)
KCONFIG_CFLAGS   += -DPWR_USER_REG0=0x180c0048
KCONFIG_CFLAGS   += -DPWR_USER_REG1=0x180c004c
KCONFIG_CFLAGS   += -DPWR_USER_REG2=0x180c0050
KCONFIG_CFLAGS   += -DPWR_USER_REG3=0x180c0054
KCONFIG_CFLAGS   += -DTIMERX_REG_BASE=0x12002020
KCONFIG_CFLAGS   += -DTIME_STAMP_BASE_ADDR=0x08100000
else ifeq ($(CFG_CHIP_TYPE), hi3559v200)
KCONFIG_CFLAGS   += -DSOFTINT_REG=0x1202001C
KCONFIG_CFLAGS   += -DTIMERX_REG_BASE=0x12000000
KCONFIG_CFLAGS   += -DTIME_STAMP_BASE_ADDR=$(shell awk 'BEGIN{printf("%d",'$(CONFIG_MEM_PARAM_BASE)'+'$(CONFIG_MEM_PARAM_SIZE)'-0x10000)}')
else ifeq ($(CFG_CHIP_TYPE), hi3518ev300)
KCONFIG_CFLAGS   += -DPWR_USER_REG0=0x120f0048
KCONFIG_CFLAGS   += -DPWR_USER_REG1=0x120f004c
KCONFIG_CFLAGS   += -DPWR_USER_REG2=0x120f0050
KCONFIG_CFLAGS   += -DPWR_USER_REG3=0x120f0054
KCONFIG_CFLAGS   += -DTIMERX_REG_BASE=0x12001000
KCONFIG_CFLAGS   += -DTIME_STAMP_BASE_ADDR=$(shell awk 'BEGIN{printf("%d",'$(CONFIG_MEM_PARAM_BASE)'+'$(CONFIG_MEM_PARAM_SIZE)'-0x10000)}')
endif

#++++++++++++++++++++++++++++++++++++++++++++++++++++
#	Sensor Type
#++++++++++++++++++++++++++++++++++++++++++++++++++++
CFG_SENSOR_CNT   := 8
ifeq ($(CONFIG_SNS0_IMX277), y)
CFG_SENSOR_TYPE0 := imx277
KCONFIG_CFLAGS   += -DCONFIG_SNS0_IMX277
else ifeq ($(CONFIG_SNS0_IMX377), y)
CFG_SENSOR_TYPE0 := imx377
KCONFIG_CFLAGS   += -DCONFIG_SNS0_IMX377
else ifeq ($(CONFIG_SNS0_IMX477), y)
CFG_SENSOR_TYPE0 := imx477
KCONFIG_CFLAGS   += -DCONFIG_SNS0_IMX477
else ifeq ($(CONFIG_SNS0_IMX458), y)
CFG_SENSOR_TYPE0 := imx458
KCONFIG_CFLAGS   += -DCONFIG_SNS0_IMX458
else ifeq ($(CONFIG_SNS0_OV12870), y)
CFG_SENSOR_TYPE0 := ov12870
KCONFIG_CFLAGS   += -DCONFIG_SNS0_OV12870
else ifeq ($(CONFIG_SNS0_IMX335), y)
CFG_SENSOR_TYPE0 := imx335
KCONFIG_CFLAGS   += -DCONFIG_SNS0_IMX335
else ifeq ($(CONFIG_SNS0_OS04B10), y)
CFG_SENSOR_TYPE0 := os04b10
KCONFIG_CFLAGS   += -DCONFIG_SNS0_OS04B10
else ifeq ($(CONFIG_SNS0_OS05A), y)
CFG_SENSOR_TYPE0 := os05a
KCONFIG_CFLAGS   += -DCONFIG_SNS0_OS05A
else ifeq ($(CONFIG_SNS0_OS08A10), y)
CFG_SENSOR_TYPE0 := os08a10
KCONFIG_CFLAGS   += -DCONFIG_SNS0_OS08A10
else ifeq ($(CONFIG_SNS0_IMX307), y)
CFG_SENSOR_TYPE0 := imx307
KCONFIG_CFLAGS   += -DCONFIG_SNS0_IMX307
else ifeq ($(CONFIG_SNS0_GC2053), y)
CFG_SENSOR_TYPE0 := gc2053
KCONFIG_CFLAGS   += -DCONFIG_SNS0_GC2053
else
CFG_SENSOR_CNT   := $(shell awk 'BEGIN{printf("%d",'$(CFG_SENSOR_CNT)'-1)}')
endif

ifeq ($(CONFIG_SNS1_IMX477), y)
CFG_SENSOR_TYPE1 := imx477
KCONFIG_CFLAGS   += -DCONFIG_SNS1_IMX477
else ifeq ($(CONFIG_SNS1_IMX307), y)
CFG_SENSOR_TYPE1 := imx307
KCONFIG_CFLAGS   += -DCONFIG_SNS1_IMX307
else ifeq ($(CONFIG_SNS1_BT656_1080P30), y)
CFG_SENSOR_TYPE1 := bt656_1080p30
KCONFIG_CFLAGS   += -DCONFIG_SNS1_BT656
else ifeq ($(CONFIG_SNS1_BT656_1080P25), y)
CFG_SENSOR_TYPE1 := bt656_1080p25
KCONFIG_CFLAGS   += -DCONFIG_SNS1_BT656
KCONFIG_CFLAGS   += -DCONFIG_SNS1_BT656_1080P25
else ifeq ($(CONFIG_SNS1_BT656_720P30), y)
CFG_SENSOR_TYPE1 := bt656_720p30
KCONFIG_CFLAGS   += -DCONFIG_SNS1_BT656
else
CFG_SENSOR_CNT   := $(shell awk 'BEGIN{printf("%d",'$(CFG_SENSOR_CNT)'-1)}')
endif

ifeq ($(CONFIG_SNS2_IMX477), y)
CFG_SENSOR_TYPE2 := imx477
KCONFIG_CFLAGS   += -DCONFIG_SNS2_IMX477
else
CFG_SENSOR_CNT   := $(shell awk 'BEGIN{printf("%d",'$(CFG_SENSOR_CNT)'-1)}')
endif

ifeq ($(CONFIG_SNS3_IMX477), y)
CFG_SENSOR_TYPE3 := imx477
KCONFIG_CFLAGS   += -DCONFIG_SNS3_IMX477
else ifeq ($(CONFIG_SNS3_IMX307), y)
CFG_SENSOR_TYPE3 := imx307
KCONFIG_CFLAGS   += -DCONFIG_SNS3_IMX307
else
CFG_SENSOR_CNT   := $(shell awk 'BEGIN{printf("%d",'$(CFG_SENSOR_CNT)'-1)}')
endif

ifeq ($(CONFIG_SNS4_IMX477), y)
CFG_SENSOR_TYPE4 := imx477
KCONFIG_CFLAGS   += -DCONFIG_SNS4_IMX477
else
CFG_SENSOR_CNT   := $(shell awk 'BEGIN{printf("%d",'$(CFG_SENSOR_CNT)'-1)}')
endif

ifeq ($(CONFIG_SNS5_IMX477), y)
CFG_SENSOR_TYPE5 := imx477
KCONFIG_CFLAGS   += -DCONFIG_SNS5_IMX477
else
CFG_SENSOR_CNT   := $(shell awk 'BEGIN{printf("%d",'$(CFG_SENSOR_CNT)'-1)}')
endif

ifeq ($(CONFIG_SNS6_IMX477), y)
CFG_SENSOR_TYPE6 := imx477
KCONFIG_CFLAGS   += -DCONFIG_SNS6_IMX477
else
CFG_SENSOR_CNT   := $(shell awk 'BEGIN{printf("%d",'$(CFG_SENSOR_CNT)'-1)}')
endif

ifeq ($(CONFIG_SNS7_IMX477), y)
CFG_SENSOR_TYPE7 := imx477
KCONFIG_CFLAGS   += -DCONFIG_SNS7_IMX477
else
CFG_SENSOR_CNT   := $(shell awk 'BEGIN{printf("%d",'$(CFG_SENSOR_CNT)'-1)}')
endif

KCONFIG_CFLAGS   +=-DCONFIG_SENSOR_CNT=$(CFG_SENSOR_CNT)
ifeq ($(CONFIG_DASHCAM), y)
ifeq ($(CFG_SENSOR_TYPE1)$(CFG_SENSOR_TYPE2)$(CFG_SENSOR_TYPE3)$(CFG_SENSOR_TYPE4),)
KCONFIG_CFLAGS +=-DONE_SENSOR_CONNECT
else
KCONFIG_CFLAGS +=-DTWO_SENSOR_CONNECT
endif
endif

#++++++++++++++++++++++++++++++++++++++++++++++++++++
#	Peripheral
#++++++++++++++++++++++++++++++++++++++++++++++++++++
#	Display screen
ifeq ($(CONFIG_SCREEN), y)
KCONFIG_CFLAGS   += -DCONFIG_SCREEN
ifeq ($(CONFIG_SCREEN_OTA5182), y)
KCONFIG_CFLAGS   += -DCONFIG_SCREEN_OTA5182
KCONFIG_CFLAGS   += -DCONFIG_SCREEN_TYPE_LCD8BIT
else ifeq ($(CONFIG_SCREEN_ST7796S), y)
KCONFIG_CFLAGS   += -DCONFIG_SCREEN_ST7796S
KCONFIG_CFLAGS   += -DCONFIG_SCREEN_TYPE_MIPI
else ifeq ($(CONFIG_SCREEN_ST7789), y)
KCONFIG_CFLAGS   += -DCONFIG_SCREEN_ST7789
KCONFIG_CFLAGS   += -DCONFIG_SCREEN_TYPE_LCD6BIT
else ifeq ($(CONFIG_SCREEN_ST7796S_RGB16BIT), y)
KCONFIG_CFLAGS   += -DCONFIG_SCREEN_ST7796S_RGB16BIT
KCONFIG_CFLAGS   += -DCONFIG_SCREEN_TYPE_LCD16BIT
else ifeq ($(CONFIG_SCREEN_OTA7290B), y)
KCONFIG_CFLAGS   += -DCONFIG_SCREEN_OTA7290B
KCONFIG_CFLAGS   += -DCONFIG_SCREEN_TYPE_MIPI
endif
ifeq ($(CONFIG_VERTICAL_SCREEN), y)
KCONFIG_CFLAGS   += -DCONFIG_VERTICAL_SCREEN
endif
ifeq ($(CONFIG_FLIP_TOUCHPAD), y)
KCONFIG_CFLAGS   += -DCONFIG_FLIP_TOUCHPAD
endif
endif

#	touchpad
ifeq ($(CONFIG_TOUCHPAD), y)
KCONFIG_CFLAGS   += -DCFG_LCD_TOUCHPAD_ON
ifeq ($(CFG_TOUCHPAD_FT6356), y)
KCONFIG_CFLAGS   += -DCFG_TOUCHPAD_FT6356
endif
endif

#	gauge
ifeq ($(CONFIG_GAUGE),y)
KCONFIG_CFLAGS   += -DCONFIG_GAUGE_ON
ifeq ($(CONFIG_GAUGE_BQ27421), y)
KCONFIG_CFLAGS   += -DCONFIG_GAUGE_BQ27421
endif
endif

#	wifi
ifeq ($(CONFIG_WIFI),y)
KCONFIG_CFLAGS   += -DCONFIG_WIFI_ON
ifeq ($(CONFIG_WIFI_BCM6225), y)
KCONFIG_CFLAGS   += -DCONFIG_WIFI_BCM6225
else ifeq ($(CONFIG_WIFI_BCM6356S), y)
KCONFIG_CFLAGS   += -DCONFIG_WIFI_BCM6356S
endif
endif

#	gsensor
ifeq ($(CONFIG_GSENSOR),y)
KCONFIG_CFLAGS   += -DCONFIG_GSENSOR_ON
endif

#	gps
ifeq ($(CONFIG_GPS),y)
KCONFIG_CFLAGS   += -DCONFIG_GPS_ON
endif

#	AHD
ifeq ($(CONFIG_AHD),y)
KCONFIG_CFLAGS   += -DCONFIG_AHD_ON
endif
ifeq ($(CONFIG_AHD_N3),y)
KCONFIG_CFLAGS   += -DCONFIG_AHD_N3
endif

#	LED
ifeq ($(CONFIG_LED),y)
KCONFIG_CFLAGS   += -DCONFIG_LED_ON
endif

#	ADC
ifeq ($(CONFIG_ADC),y)
KCONFIG_CFLAGS   += -DCONFIG_ADC_ON
endif

#	WDT
ifeq ($(CONFIG_WATCHDOG),y)
KCONFIG_CFLAGS   += -DCONFIG_WATCHDOG_ON
endif

#       MOTIONSENSOR
ifeq ($(CONFIG_MOTIONSENSOR),y)
KCONFIG_CFLAGS   += -DCONFIG_MOTIONSENSOR
ifeq ($(CONFIG_MOTIONSENSOR_IMC20690), y)
KCONFIG_CFLAGS   += -DCONFIG_MOTIONSENSOR_IMC20690
else ifeq ($(CONFIG_MOTIONSENSOR_LSM6DSM), y)
KCONFIG_CFLAGS   += -DCONFIG_MOTIONSENSOR_LSM6DSM
endif
endif

#   ACC
ifeq ($(CONFIG_ACC),y)
KCONFIG_CFLAGS   += -DCONFIG_ACC_ON
endif
#++++++++++++++++++++++++++++++++++++++++++++++++++++
#	Build Type
#++++++++++++++++++++++++++++++++++++++++++++++++++++
ifeq ($(CONFIG_DEBUG), y)
export CFG_VERSION := debug
KCONFIG_CFLAGS   += -DCONFIG_DEBUG

#Log Level : Debug/Error
ifeq ($(CONFIG_LOG_LEVEL_DEBUG), y)
KCONFIG_CFLAGS   += -DCFG_DEBUG_LOG_ON
endif

#PQTool
ifeq ($(CONFIG_PQT_SUPPORT_ON), y)
ifeq ($(CONFIG_PQT_STREAM_SUPPORT_ON), y)
KCONFIG_CFLAGS   += -DCONFIG_PQT_STREAM_SUPPORT_ON
endif
KCONFIG_CFLAGS   += -DCONFIG_PQT_SUPPORT_ON
endif

ifeq ($(CONFIG_ISP_OSD_SUPPORT_ON), y)
KCONFIG_CFLAGS   += -DCONFIG_ISP_OSD_SUPPORT_ON
endif


#CoreDump
ifeq ($(CONFIG_COREDUMP_ON), y)
KCONFIG_CFLAGS   += -DCONFIG_COREDUMP_ON
endif

else ifeq ($(CONFIG_RELEASE), y)
export CFG_VERSION := release
KCONFIG_CFLAGS   += -DCONFIG_RELEASE
endif

#++++++++++++++++++++++++++++++++++++++++++++++++++++
#	Param Type : rawparam/rootfs
#++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifeq ($(AMP_TYPE), linux_liteos)
KCONFIG_CFLAGS   += -DCFG_RAW_PARAM_ON
#endif

#++++++++++++++++++++++++++++++++++++++++++++++++++++
#	TimeStamp / TimeMeasure
#++++++++++++++++++++++++++++++++++++++++++++++++++++
ifeq ($(CONFIG_TIMESTAMP_ON), y)
KCONFIG_CFLAGS   += -DCFG_TIME_STAMP_ON -DCFG_TIME_MEASURE_ON
endif

#++++++++++++++++++++++++++++++++++++++++++++++++++++
#	BusyPage
#++++++++++++++++++++++++++++++++++++++++++++++++++++
ifeq ($(CONFIG_BUSYPAGE_ON), y)
KCONFIG_CFLAGS   += -DCONFIG_BUSYPAGE_ON
endif

#++++++++++++++++++++++++++++++++++++++++++++++++++++
#	Rootfs Type
#++++++++++++++++++++++++++++++++++++++++++++++++++++
ifeq ($(CONFIG_ROOTFS_JFFS2), y)
KCONFIG_CFLAGS   += -DCONFIG_ROOTFS_JFFS2
CFG_ROOTFS_TYPE  := jffs2
else ifeq ($(CONFIG_ROOTFS_SQUASHFS), y)
KCONFIG_CFLAGS   += -DCONFIG_ROOTFS_SQUASHFS
CFG_ROOTFS_TYPE  := squashfs
else ifeq ($(CONFIG_ROOTFS_YAFFS), y)
KCONFIG_CFLAGS   += -DCONFIG_ROOTFS_YAFFS
CFG_ROOTFS_TYPE  := yaffs
else ifeq ($(CONFIG_ROOTFS_UBIFS), y)
KCONFIG_CFLAGS   += -DCONFIG_ROOTFS_UBIFS
CFG_ROOTFS_TYPE  := ubifs
else ifeq ($(CONFIG_ROOTFS_EXT4), y)
KCONFIG_CFLAGS   += -DCONFIG_ROOTFS_EXT4
CFG_ROOTFS_TYPE  := ext4
endif

#++++++++++++++++++++++++++++++++++++++++++++++++++++
#	Appfs Type
#++++++++++++++++++++++++++++++++++++++++++++++++++++
ifeq ($(CONFIG_APPFS_JFFS2), y)
KCONFIG_CFLAGS   += -DCONFIG_APPFS_JFFS2
CFG_APPFS_TYPE   := jffs2
else ifeq ($(CONFIG_APPFS_YAFFS), y)
KCONFIG_CFLAGS   += -DCONFIG_APPFS_YAFFS
CFG_APPFS_TYPE   := yaffs
else ifeq ($(CONFIG_APPFS_UBIFS), y)
KCONFIG_CFLAGS   += -DCONFIG_APPFS_UBIFS
CFG_APPFS_TYPE   := ubifs
else ifeq ($(CONFIG_APPFS_EXT4), y)
KCONFIG_CFLAGS   += -DCONFIG_APPFS_EXT4
CFG_APPFS_TYPE   := ext4
endif

#++++++++++++++++++++++++++++++++++++++++++++++++++++
#	eMMC Storage Device/Partition No
#++++++++++++++++++++++++++++++++++++++++++++++++++++
ifeq ($(CONFIG_EMMC), y)
KCONFIG_CFLAGS   += -DCONFIG_EMMC_STORAGE_BLK_NO=$(CONFIG_EMMC_STORAGE_BLK_NO)
KCONFIG_CFLAGS   += -DCONFIG_EMMC_STORAGE_PART_NO=$(CONFIG_EMMC_STORAGE_PART_NO)
endif

#++++++++++++++++++++++++++++++++++++++++++++++++++++
#	Memory
#++++++++++++++++++++++++++++++++++++++++++++++++++++
ifneq ($(CONFIG_MEM_TOTAL_SIZE),)
KCONFIG_CFLAGS   += -DCFG_TOTAL_MEN_LEN=$(CONFIG_MEM_TOTAL_SIZE)
endif
ifneq ($(CONFIG_MEM_PARAM_BASE),)
KCONFIG_CFLAGS   += -DCFG_PARAM_PHY_ADDR=$(CONFIG_MEM_PARAM_BASE)
endif
ifneq ($(CONFIG_MEM_HUAWEILITE_MMZ_BASE),)
KCONFIG_CFLAGS   += -DCFG_LITEOS_MMZ_START=$(CONFIG_MEM_HUAWEILITE_MMZ_BASE)
endif
ifneq ($(CONFIG_MEM_HUAWEILITE_MMZ_SIZE),)
KCONFIG_CFLAGS   += -DCFG_LITEOS_MMZ_LEN=$(shell awk 'BEGIN{printf("%d",'$(CONFIG_MEM_HUAWEILITE_MMZ_SIZE)'/0x400)}')
endif
ifneq ($(CONFIG_MEM_LINUX_MMZ_BASE),)
KCONFIG_CFLAGS   += -DCFG_LINUX_MMZ_START=$(CONFIG_MEM_LINUX_MMZ_BASE)
endif
KCONFIG_CFLAGS   += -DCFG_LINUX_MMZ_LEN=$(shell awk 'BEGIN{printf("%d",'$(CONFIG_MEM_LINUX_MMZ_ANONYMOUS_SIZE)'/0x400+'$(CONFIG_MEM_LINUX_MMZ_HIGO_SIZE)'/0x400)}')
ifneq ($(CONFIG_MEM_LINUX_MMZ_ANONYMOUS_SIZE),)
KCONFIG_CFLAGS   += -DCFG_LINUX_MMZ_ANONYMOUS_LEN=$(shell awk 'BEGIN{printf("%d",'$(CONFIG_MEM_LINUX_MMZ_ANONYMOUS_SIZE)'/0x400)}')
endif
ifneq ($(CONFIG_MEM_LINUX_MMZ_HIGO_SIZE),)
KCONFIG_CFLAGS   += -DCFG_LINUX_MMZ_HIGO_LEN=$(shell awk 'BEGIN{printf("%d",'$(CONFIG_MEM_LINUX_MMZ_HIGO_SIZE)'/0x400)}')
endif
ifeq ($(CONFIG_MEM_LINUX_MMZ_HDMI_RELOAD),y)
KCONFIG_CFLAGS   += -DCFG_LINUX_MMZ_HDMI_RELOAD
ifneq ($(CONFIG_MEM_LINUX_MMZ_HDMI_BASE),)
KCONFIG_CFLAGS   += -DCFG_LINUX_MMZ_HDMI_START=$(CONFIG_MEM_LINUX_MMZ_HDMI_BASE)
endif
ifneq ($(CONFIG_MEM_LINUX_MMZ_HDMI_ANONYMOUS_SIZE),)
KCONFIG_CFLAGS   += -DCFG_LINUX_MMZ_HDMI_ANONYMOUS_LEN=$(shell awk 'BEGIN{printf("%d",'$(CONFIG_MEM_LINUX_MMZ_HDMI_ANONYMOUS_SIZE)'/0x400)}')
endif
ifneq ($(CONFIG_MEM_LINUX_MMZ_HDMI_HIGO_SIZE),)
KCONFIG_CFLAGS   += -DCFG_LINUX_MMZ_HDMI_HIGO_LEN=$(shell awk 'BEGIN{printf("%d",'$(CONFIG_MEM_LINUX_MMZ_HDMI_HIGO_SIZE)'/0x400)}')
endif
endif

#++++++++++++++++++++++++++++++++++++++++++++++++++++
#	Media
#++++++++++++++++++++++++++++++++++++++++++++++++++++
ifeq ($(CONFIG_LDC_SUPPORT), y)
KCONFIG_CFLAGS   += -DCONFIG_LDC_SUPPORT
endif

#++++++++++++++++++++++++++++++++++++++++++++++++++++
#	FileMng
#++++++++++++++++++++++++++++++++++++++++++++++++++++
ifeq ($(CONFIG_DCF), y)
KCONFIG_CFLAGS   += -DCONFIG_FILEMNG_DCF
else ifeq ($(CONFIG_DTCF), y)
KCONFIG_CFLAGS   += -DCONFIG_FILEMNG_DTCF
endif

#++++++++++++++++++++++++++++++++++++++++++++++++++++
#	RecordMng
#++++++++++++++++++++++++++++++++++++++++++++++++++++
ifeq ($(CONFIG_MP4), y)
KCONFIG_CFLAGS   += -DCONFIG_RECORDMNG_MP4
endif
ifeq ($(CONFIG_TS), y)
KCONFIG_CFLAGS   += -DCONFIG_RECORDMNG_TS
endif
ifeq ($(CONFIG_SUPPORT_ASYNC_STOP), y)
KCONFIG_CFLAGS   += -DRECMNG_SUPPORT_ASYNC_STOP
endif

#++++++++++++++++++++++++++++++++++++++++++++++++++++
#	Photo
#++++++++++++++++++++++++++++++++++++++++++++++++++++
ifeq ($(CONFIG_POST_PROCESS), y)
KCONFIG_CFLAGS   += -DCFG_POST_PROCESS
KCONFIG_CFLAGS   += -DSUPPORT_PHOTO_POST_PROCESS
ifeq ($(CONFIG_HI3559AV100), y)
KCONFIG_CFLAGS   += -DPHOTO_POSTPROCESS_ARCH_DUAL
endif
endif
ifeq ($(CONFIG_EXIF), y)
KCONFIG_CFLAGS   += -DCFG_EXIF
endif
ifeq ($(CONFIG_DNG), y)
KCONFIG_CFLAGS   += -DCFG_DNG
endif

#++++++++++++++++++++++++++++++++++++++++++++++++++++
#	Player
#++++++++++++++++++++++++++++++++++++++++++++++++++++
ifeq ($(CONFIG_PLAY_BACK),y)
KCONFIG_CFLAGS   += -DSUPPORT_PLAY_BACK
endif
ifeq ($(CONFIG_VOICE_PLAY),y)
KCONFIG_CFLAGS   += -DSUPPORT_VOICE_PLAY
endif

#++++++++++++++++++++++++++++++++++++++++++++++++++++
#	RawCapture
#++++++++++++++++++++++++++++++++++++++++++++++++++++
ifeq ($(CONFIG_RAWDEBUG),y)
KCONFIG_CFLAGS   += -DCONFIG_RAWCAP_ON
endif

#++++++++++++++++++++++++++++++++++++++++++++++++++++
#	VideoDetect
#++++++++++++++++++++++++++++++++++++++++++++++++++++
ifeq ($(CONFIG_VIDEO_DETECT),y)
KCONFIG_CFLAGS   += -DCONFIG_VIDEO_DETECT_ON
ifeq ($(CONFIG_MOTIONDETECT),y)
KCONFIG_CFLAGS   += -DCONFIG_MOTIONDETECT_ON
endif
endif

#++++++++++++++++++++++++++++++++++++++++++++++++++++
#	PowerManage
#++++++++++++++++++++++++++++++++++++++++++++++++++++
ifeq ($(CONFIG_TEMPRATURE_CYCLE),y)
KCONFIG_CFLAGS   += -DCONFIG_TEMPRATURE_CYCLE_ON
endif

#++++++++++++++++++++++++++++++++++++++++++++++++++++
#	SceneAuto
#++++++++++++++++++++++++++++++++++++++++++++++++++++
ifeq ($(CONFIG_HI3559AV100), y)
KCONFIG_CFLAGS   += -DSUPPORT_PQ_HDRMODE
KCONFIG_CFLAGS   += -DCFG_3DNR_IN_VI
else ifeq ($(CONFIG_HI3556AV100), y)
KCONFIG_CFLAGS   += -DCFG_3DNR_IN_VPSS
else ifeq ($(CONFIG_HI3519AV100), y)
KCONFIG_CFLAGS   += -DCFG_3DNR_IN_VPSS
endif

ifeq ($(CONFIG_SCENEAUTO_SUPPORT), y)
KCONFIG_CFLAGS   += -DCONFIG_SCENEAUTO_SUPPORT
endif

#++++++++++++++++++++++++++++++++++++++++++++++++++++
#	KeyType
#++++++++++++++++++++++++++++++++++++++++++++++++++++
ifeq ($(CONFIG_HI3556AV100), y)
ifeq ($(CONFIG_REFB_ACTIONCAM), y)
KCONFIG_CFLAGS   += -DCFG_ONEKEY_CONSOLE
endif
endif

#++++++++++++++++++++++++++++++++++++++++++++++++++++
#	System Feature
#++++++++++++++++++++++++++++++++++++++++++++++++++++
ifeq ($(CONFIG_BOOT_SOUND),y)
KCONFIG_CFLAGS   += -DCFG_BOOT_SOUND_ON
endif
ifeq ($(CONFIG_BOOT_LOGO),y)
KCONFIG_CFLAGS   += -DCFG_BOOT_LOGO_ON
endif

#++++++++++++++++++++++++++++++++++++++++++++++++++++
#	Vcap Standby Feature
#++++++++++++++++++++++++++++++++++++++++++++++++++++
ifeq ($(CONFIG_HI3556AV100), y)
ifneq ($(CONFIG_SNS0_IMX477), y)
KCONFIG_CFLAGS   += -DCFG_MEDIA_VCAP_STANDBY
endif
endif

#++++++++++++++++++++++++++++++++++++++++++++++++++++
#	Uboot Table File
#++++++++++++++++++++++++++++++++++++++++++++++++++++
ifeq ($(CONFIG_HI3556AV100), y)
ifeq ($(CONFIG_REFB_ACTIONCAM) , y)
export REGBIN_XLSM := Hi3556AV100-DVREFB_LPDDR4B-8L_T_LPDDR4_2664M_1GB_32bitx1-A53_1200M.xlsm
endif
endif

ifeq ($(CONFIG_HI3559V200), y)
ifeq ($(CONFIG_DDR_BITWIDTH_16), y)
export REGBIN_XLSM := Hi3559V200-DMEB_6L_T-DDR3_1800M_512MB_16bit-A7_900M-SYSBUS_300M.xlsm
endif
endif

#++++++++++++++++++++++++++++++++++++++++++++++++++++
#	UI Source
#++++++++++++++++++++++++++++++++++++++++++++++++++++
ifeq ($(CONFIG_LCD_UISRC_ADAPT_AUTO), y)
KCONFIG_CFLAGS   += -DCONFIG_LCD_UISRC_ADAPT_AUTO
ifneq ($(CONFIG_LCD_UISRC_BASE_WIDTH),)
KCONFIG_CFLAGS   += -DCFG_LCD_UISRC_BASE_WIDTH=$(CONFIG_LCD_UISRC_BASE_WIDTH)
endif
ifneq ($(CONFIG_LCD_UISRC_BASE_HEIGHT),)
KCONFIG_CFLAGS   += -DCFG_LCD_UISRC_BASE_HEIGHT=$(CONFIG_LCD_UISRC_BASE_HEIGHT)
endif
ifneq ($(CONFIG_LCD_UISRC_DEST_WIDTH),)
KCONFIG_CFLAGS   += -DCFG_LCD_UISRC_DEST_WIDTH=$(CONFIG_LCD_UISRC_DEST_WIDTH)
endif
ifneq ($(CONFIG_LCD_UISRC_DEST_HEIGHT),)
KCONFIG_CFLAGS   += -DCFG_LCD_UISRC_DEST_HEIGHT=$(CONFIG_LCD_UISRC_DEST_HEIGHT)
endif
else ifeq ($(CONFIG_LCD_UISRC_ADAPT_MANUAL), y)
KCONFIG_CFLAGS   += -DCONFIG_LCD_UISRC_ADAPT_MANUAL
endif

ifeq ($(CONFIG_HDMI_UISRC_ADAPT_AUTO), y)
KCONFIG_CFLAGS   += -DCONFIG_HDMI_UISRC_ADAPT_AUTO
ifneq ($(CONFIG_HDMI_UISRC_BASE_WIDTH),)
KCONFIG_CFLAGS   += -DCFG_HDMI_UISRC_BASE_WIDTH=$(CONFIG_HDMI_UISRC_BASE_WIDTH)
endif
ifneq ($(CONFIG_HDMI_UISRC_BASE_HEIGHT),)
KCONFIG_CFLAGS   += -DCFG_HDMI_UISRC_BASE_HEIGHT=$(CONFIG_HDMI_UISRC_BASE_HEIGHT)
endif
ifneq ($(CONFIG_HDMI_UISRC_DEST_WIDTH),)
KCONFIG_CFLAGS   += -DCFG_HDMI_UISRC_DEST_WIDTH=$(CONFIG_HDMI_UISRC_DEST_WIDTH)
endif
ifneq ($(CONFIG_HDMI_UISRC_DEST_HEIGHT),)
KCONFIG_CFLAGS   += -DCFG_HDMI_UISRC_DEST_HEIGHT=$(CONFIG_HDMI_UISRC_DEST_HEIGHT)
endif
else ifeq ($(CONFIG_HDMI_UISRC_ADAPT_MANUAL), y)
KCONFIG_CFLAGS   += -DCONFIG_HDMI_UISRC_ADAPT_MANUAL
endif

endif # ifndef $(SDK_KCONFIG)
