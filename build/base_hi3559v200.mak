ifeq ($(SDK_BASECFG_HI3559V200),)
SDK_BASECFG_HI3559V200 = 1

# Check Parameter
ifeq ($(SDK_ROOT),)
    $(error please specify sdk root path)
endif

##########################################################################################
#	OSDRV Configure
##########################################################################################
OSDRV_ROOT            := $(SDK_ROOT)/osdrv
OSDRV_UBOOT_PATH      := $(OSDRV_ROOT)/opensource/uboot
OSDRV_LINUX_PATH      := $(OSDRV_ROOT)/opensource/kernel
export LINUX_ROOT     := $(OSDRV_LINUX_PATH)/linux-4.9.y
OSDRV_BUSYBOX_PATH    := $(OSDRV_ROOT)/opensource/busybox
OSDRV_COMPONENT_PATH  := $(OSDRV_ROOT)/components
OSDRV_COMPONENT_IPCM  := $(OSDRV_COMPONENT_PATH)/ipcm/ipcm
OSDRV_TOOLS_PATH      := $(OSDRV_ROOT)/tools
OSDRV_CROSS           := $(LINUX_CROSS)
OSDRV_LITEOS_PATH     := $(OSDRV_ROOT)/platform/liteos
export LITEOS_ROOT    := $(OSDRV_LITEOS_PATH)
OSDRV_LITEOS_DYNLOAD_TOOLS := $(LITEOS_ROOT)/tools/scripts/dynload_tools
OSDRV_LITEOS_TEXT_OFFSET   := 0x00000000

OSDRV_PUB_ROOT        := $(OSDRV_ROOT)/pub
OSDRV_PUB_IMG_PATH    := $(OSDRV_PUB_ROOT)/$(CFG_SUBCHIP_TYPE)_$(BOOT_MEDIA)_amp_image_$(LIBC_TYPE)
OSDRV_PUB_IMG_BIN     := $(OSDRV_PUB_IMG_PATH)/uImage_$(CFG_SUBCHIP_TYPE)_amp
OSDRV_PUB_IMG_MINIKER := $(OSDRV_PUB_IMG_PATH)/uImage_$(CFG_SUBCHIP_TYPE)_mini
#OSDRV_PUB_LINUX_LIB   := $(OSDRV_PUB_ROOT)/linux/lib
OSDRV_PUB_LINUX_BIN   := $(OSDRV_PUB_ROOT)/bin/board_$(LIBC_TYPE)
OSDRV_PUB_PC_BIN_PATH := $(OSDRV_PUB_ROOT)/bin/pc
OSDRV_ROOTFS_PKT      := $(OSDRV_PUB_ROOT)/rootfs_$(LIBC_TYPE).tgz
OSDRV_MINIROOTFS_PKT  := $(OSDRV_PUB_ROOT)/rootfs_$(LIBC_TYPE).tgz
OSDRV_PUB_GDB_FILE    := $(OSDRV_PUB_LINUX_BIN)/gdb-$(LINUX_CROSS)
OSDRV_LINUX_CROSS_LIB := /opt/hisi-linux/x86-arm/$(LINUX_CROSS)/arm-linux-uclibceabi/lib/a7_softfp_neon-vfpv4/

OSDRV_ARCH            := arm
OSDRV_COMPILE_OPT     := CHIP=$(CFG_SUBCHIP_TYPE) BOOT_MEDIA=$(BOOT_MEDIA) OSDRV_CROSS=$(LINUX_CROSS)

##########################################################################################
# OSAL Configure
##########################################################################################
AMP_ROOT                   := $(SDK_ROOT)/amp
OSAL_ROOT                  := $(SDK_ROOT)/amp
OSAL_LINUX_PATH            := $(OSAL_ROOT)/a7_linux/osal
OSAL_LITEOS_PATH           := $(OSAL_ROOT)/a7_liteos/osal

##########################################################################################
# MPP Configure
##########################################################################################
MPP_ROOT                   := $(SDK_ROOT)/amp
MPP_REL_PATH               := $(MPP_ROOT)
MPP_REL_LINUX_PATH         := $(MPP_REL_PATH)/a7_linux/mpp
MPP_REL_LINUX_INC_PATH     := $(MPP_REL_LINUX_PATH)/include
MPP_REL_LINUX_KO_PATH      := $(MPP_REL_LINUX_PATH)/ko
MPP_REL_LINUX_LIB_PATH     := $(MPP_REL_LINUX_PATH)/lib
MPP_REL_LITEOS_PATH        := $(MPP_REL_PATH)/a7_liteos/mpp
MPP_REL_LITEOS_INC_PATH    := $(MPP_REL_LITEOS_PATH)/include
MPP_REL_LITEOS_LIB_PATH    := $(MPP_REL_LITEOS_PATH)/lib
MPP_REL_LITEOS_EXT_LIB_PATH:= $(MPP_REL_LITEOS_LIB_PATH)/extdrv

# MIPI Rx Lane Distribution Mode
ifeq ($(CONFIG_HI3559V200_LANE_DIVIDE_MODE_0), y)
MIPI_RX_LANE_DIVIDE_MODE   := LANE_DIVIDE_MODE_0
else ifeq ($(CONFIG_HI3559V200_LANE_DIVIDE_MODE_0_AHD), y)
MIPI_RX_LANE_DIVIDE_MODE   := LANE_DIVIDE_MODE_0
else ifeq ($(CONFIG_HI3559V200_LANE_DIVIDE_MODE_1_AHD), y)
MIPI_RX_LANE_DIVIDE_MODE   := LANE_DIVIDE_MODE_1
else ifeq ($(CONFIG_HI3559V200_LANE_DIVIDE_MODE_1), y)
MIPI_RX_LANE_DIVIDE_MODE   := LANE_DIVIDE_MODE_1
endif

##########################################################################################
# Drv Configure
##########################################################################################
DRV_ROOT                   := $(SDK_ROOT)/amp
DRV_INTERDRV_PATH_LINUX    := $(DRV_ROOT)/a7_linux/drv/interdrv
DRV_EXTDRV_PATH_LINUX      := $(DRV_ROOT)/a7_linux/drv/extdrv
DRV_INTERDRV_PATH_LITEOS   := $(DRV_ROOT)/a7_liteos/drv/interdrv
DRV_EXTDRV_PATH_LITEOS     := $(DRV_ROOT)/a7_liteos/drv/extdrv
DRV_REL_PATH               := $(DRV_ROOT)/pub
DRV_REL_PATH_LINUX         := $(DRV_REL_PATH)/linux
DRV_REL_PATH_LITEOS        := $(DRV_REL_PATH)/liteos

##########################################################################################
# HiSyslink Configure
##########################################################################################
HISYSLINK_ROOT             := $(SDK_ROOT)/amp
HISYSLINK_IPCMSG_PATH      := $(HISYSLINK_ROOT)
HISYSLINK_IPCMSG_INC_PATH  := $(HISYSLINK_IPCMSG_PATH)/a7_linux/hisyslink/ipcmsg/include
HISYSLINK_IPCMSG_LIB_LINUX := $(HISYSLINK_IPCMSG_PATH)/a7_linux/hisyslink/ipcmsg/out/a7_linux
HISYSLINK_IPCMSG_LIB_LITEOS:= $(HISYSLINK_IPCMSG_PATH)/a7_liteos/hisyslink/ipcmsg/out/a7_liteos
HISYSLINK_DATAFIFO_PATH      := $(HISYSLINK_ROOT)
HISYSLINK_DATAFIFO_INC_PATH  := $(HISYSLINK_DATAFIFO_PATH)/a7_linux/hisyslink/datafifo/include
HISYSLINK_DATAFIFO_LIB_LINUX := $(HISYSLINK_DATAFIFO_PATH)/a7_linux/hisyslink/datafifo/out/a7_linux
HISYSLINK_DATAFIFO_LIB_LITEOS:= $(HISYSLINK_DATAFIFO_PATH)/a7_liteos/hisyslink/datafifo/out/a7_liteos

##########################################################################################
# PQTool Configure
##########################################################################################
PQT_ROOT              := $(SDK_ROOT)/pqtool
PQT_PATH              := $(PQT_ROOT)/Hi3559V200_PQ_V1.0.1.2

endif # ifndef $(SDK_BASECFG_HI3559V200)
