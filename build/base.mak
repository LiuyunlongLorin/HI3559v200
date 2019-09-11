ifeq ($(SDK_BASECFG),)
SDK_BASECFG = 1

# Check Parameter
ifeq ($(SDK_ROOT),)
    $(error please specify sdk root path)
endif

# Create Directory
CreateDir = $(shell [ -d $1 ] || mkdir -p $1 || echo ":mkdir '$1' fail")
# Remove Directory
RemoveDir = $(shell [ -d $1 ] && rm -rf $1 && echo -e "rmdir '$1'\t [ OK ]" || echo ":rm dir '$1' fail")

##########################################################################################
# Kconfig Common Configure
##########################################################################################
include $(SDK_ROOT)/build/kconfig.mak

ifneq ($(CFG_CHIP_TYPE),)
include $(SDK_ROOT)/build/base_$(CFG_CHIP_TYPE).mak
endif

##########################################################################################
# MPP Configure
##########################################################################################
ifneq ($(MPP_REL_LITEOS_PATH),)
MPP_CFG_PATH=$(MPP_REL_LITEOS_PATH)/cfg.mak
else ifneq ($(MPP_REL_LINUX_PATH),)
MPP_CFG_PATH=$(MPP_REL_LINUX_PATH)/cfg.mak
endif

CONFIG_HI_HDMI_SUPPORT=$(shell $(SDK_ROOT)/build/get_mpp_cfg.sh $(MPP_CFG_PATH) CONFIG_HI_HDMI_SUPPORT=y)

##########################################################################################
# NDK Configure
##########################################################################################
NDK_ROOT                   := $(SDK_ROOT)/middleware/ndk
NDK_MAPI_PATH              := $(NDK_ROOT)/code
NDK_MAPI_INC_PATH          := $(NDK_MAPI_PATH)/include
NDK_COMPONENT_PATH         := $(NDK_ROOT)/component
NDK_THIRDPARTY_PATH        := $(NDK_ROOT)/thirdparty
NDK_SAMPLE_PATH            := $(NDK_ROOT)/sample

ifeq ($(AMP_TYPE),linux_liteos)
NDK_MAPI_REL_PATH          := $(NDK_ROOT)/out/$(CFG_CHIP_TYPE)/dual
else
NDK_MAPI_REL_PATH          := $(NDK_ROOT)/out/$(CFG_CHIP_TYPE)/$(AMP_TYPE)
endif
NDK_MAPI_REL_LINUX_PATH    := $(NDK_MAPI_REL_PATH)/host
NDK_MAPI_REL_LINUX_KO      := $(NDK_MAPI_REL_LINUX_PATH)/ko
NDK_MAPI_REL_LINUX_LIB     := $(NDK_MAPI_REL_LINUX_PATH)/lib
NDK_MAPI_REL_LITEOS_PATH   := $(NDK_MAPI_REL_PATH)/slave
NDK_MAPI_REL_LITEOS_LIB    := $(NDK_MAPI_REL_LITEOS_PATH)/lib

NDK_MAPI_ADAPT_PATH        := $(NDK_MAPI_PATH)/mediaserver/adapt
NDK_MAPI_SNSADAPT_PATH     := $(NDK_MAPI_ADAPT_PATH)/sensor/$(CFG_CHIP_TYPE)
NDK_MAPI_CFG_PATH          := $(NDK_MAPI_PATH)/mediaserver/configs
NDK_MAPI_SNSCFG_PATH       := $(NDK_MAPI_CFG_PATH)/sensor/$(CFG_CHIP_TYPE)
NDK_MAPI_DISCFG_PATH       := $(NDK_MAPI_CFG_PATH)/dis/hi3559

# Compile Option: Path
NDK_COMPILE_OPT            := OSDRV_ROOT=$(OSDRV_ROOT)
NDK_COMPILE_OPT            += SDK_ROOT=$(SDK_ROOT)
NDK_COMPILE_OPT            += MPP_OUT_PATH=$(MPP_REL_PATH)
NDK_COMPILE_OPT            += HISYSLINK_PATH=$(HISYSLINK_ROOT)
NDK_COMPILE_OPT            += OSAL_ROOT=$(OSAL_ROOT)

# Compile Option: Sensor
NDK_COMPILE_OPT            += SENSOR_CABLE_TYPE=$(MIPI_RX_LANE_DIVIDE_MODE)
ifeq ($(CONFIG_SNS0_IMX277), y)
NDK_COMPILE_OPT            += CFG_SENSOR_TYPE0=IMX277_SLVS
else ifeq ($(CONFIG_SNS0_IMX377), y)
NDK_COMPILE_OPT            += CFG_SENSOR_TYPE0=IMX377
else ifeq ($(CONFIG_SNS0_IMX477), y)
NDK_COMPILE_OPT            += CFG_SENSOR_TYPE0=IMX477
else ifeq ($(CONFIG_SNS0_IMX458), y)
NDK_COMPILE_OPT            += CFG_SENSOR_TYPE0=IMX458
else ifeq ($(CONFIG_SNS0_OV12870), y)
NDK_COMPILE_OPT            += CFG_SENSOR_TYPE0=OV12870
else ifeq ($(CONFIG_SNS0_IMX335), y)
NDK_COMPILE_OPT            += CFG_SENSOR_TYPE0=IMX335
else ifeq ($(CONFIG_SNS0_IMX307), y)
NDK_COMPILE_OPT            += CFG_SENSOR_TYPE0=IMX307
else ifeq ($(CONFIG_SNS0_OS04B10), y)
NDK_COMPILE_OPT            += CFG_SENSOR_TYPE0=OS04B10
else ifeq ($(CONFIG_SNS0_OS05A), y)
NDK_COMPILE_OPT            += CFG_SENSOR_TYPE0=OS05A
else ifeq ($(CONFIG_SNS0_OS08A10), y)
NDK_COMPILE_OPT            += CFG_SENSOR_TYPE0=OS08A10
else ifeq ($(CONFIG_SNS0_GC2053), y)
NDK_COMPILE_OPT            += CFG_SENSOR_TYPE0=GC2053
else
NDK_COMPILE_OPT            += CFG_SENSOR_TYPE0=UNUSED
endif
ifeq ($(CONFIG_SNS1_IMX477), y)
NDK_COMPILE_OPT            += CFG_SENSOR_TYPE1=IMX477
else ifeq ($(CONFIG_SNS1_IMX307), y)
NDK_COMPILE_OPT            += CFG_SENSOR_TYPE1=IMX307
else ifeq ($(CONFIG_SNS1_BT656_1080P30)$(CONFIG_SNS1_BT656_1080P25)$(CONFIG_SNS1_BT656_720P30)$(CONFIG_SNS1_BT656_720P25), y)
NDK_COMPILE_OPT            += CFG_SENSOR_TYPE1=BT656
else
NDK_COMPILE_OPT            += CFG_SENSOR_TYPE1=UNUSED
endif
ifeq ($(CONFIG_SNS2_IMX477), y)
NDK_COMPILE_OPT            += CFG_SENSOR_TYPE2=IMX477
else
NDK_COMPILE_OPT            += CFG_SENSOR_TYPE2=UNUSED
endif
ifeq ($(CONFIG_SNS3_IMX477), y)
NDK_COMPILE_OPT            += CFG_SENSOR_TYPE3=IMX477
else ifeq ($(CONFIG_SNS3_IMX307), y)
NDK_COMPILE_OPT            += CFG_SENSOR_TYPE3=IMX307
else
NDK_COMPILE_OPT            += CFG_SENSOR_TYPE3=UNUSED
endif
ifeq ($(CONFIG_SNS4_IMX477), y)
NDK_COMPILE_OPT            += CFG_SENSOR_TYPE4=IMX477
else
NDK_COMPILE_OPT            += CFG_SENSOR_TYPE4=UNUSED
endif
ifeq ($(CONFIG_SNS5_IMX477), y)
NDK_COMPILE_OPT            += CFG_SENSOR_TYPE5=IMX477
else
NDK_COMPILE_OPT            += CFG_SENSOR_TYPE5=UNUSED
endif
ifeq ($(CONFIG_SNS6_IMX477), y)
NDK_COMPILE_OPT            += CFG_SENSOR_TYPE6=IMX477
else
NDK_COMPILE_OPT            += CFG_SENSOR_TYPE6=UNUSED
endif
ifeq ($(CONFIG_SNS7_IMX477), y)
NDK_COMPILE_OPT            += CFG_SENSOR_TYPE7=IMX477
else
NDK_COMPILE_OPT            += CFG_SENSOR_TYPE7=UNUSED
endif

# Compile Option: Feature
ifeq ($(CONFIG_POST_PROCESS), y)
NDK_COMPILE_OPT            += CFG_SUPPORT_PHOTO_POST_PROCESS=y
else
NDK_COMPILE_OPT            += CFG_SUPPORT_PHOTO_POST_PROCESS=n
endif
ifeq ($(CONFIG_PANOCAM), y)
NDK_COMPILE_OPT            += CFG_SUPPORT_STITCH=y
else
NDK_COMPILE_OPT            += CFG_SUPPORT_STITCH=n
endif
ifeq ($(CONFIG_MOTIONSENSOR), y)
NDK_COMPILE_OPT            += CFG_SUPPORT_GYRO=y
endif

# HDMI
ifeq ($(CONFIG_CARRECORDER)$(CONFIG_REARVIEW), y)
ifeq ($(CONFIG_SCREEN), y)
NDK_COMPILE_OPT            += CFG_SUPPORT_HDMI=n
endif
endif

# Version: Debug/Release
ifeq ($(CONFIG_RELEASE), y)
NDK_COMPILE_OPT            += CFG_ENABLE_MINI=y
endif

MW_COMPILE_OPT := $(NDK_COMPILE_OPT)

##########################################################################################
# MiddleWare Configure
##########################################################################################
MWARE_ROOT            := $(SDK_ROOT)/middleware
MWARE_COMMON_PATH     := $(MWARE_ROOT)/common
MWARE_COMPONENT_PATH  := $(MWARE_ROOT)/component
MWARE_THIRDPARTY_PATH := $(MWARE_ROOT)/thirdparty
MWARE_SAMPLE_PATH     := $(MWARE_ROOT)/sample
MWARE_REL_PATH        := $(MWARE_ROOT)/out
MWARE_REL_LINUX_PATH  := $(MWARE_REL_PATH)/linux
MWARE_REL_LINUX_INC   := $(MWARE_REL_LINUX_PATH)/include
MWARE_REL_LINUX_LIB   := $(MWARE_REL_LINUX_PATH)/lib
MWARE_REL_LITEOS_PATH := $(MWARE_REL_PATH)/liteos
MWARE_REL_LITEOS_INC  := $(MWARE_REL_LITEOS_PATH)/include
MWARE_REL_LITEOS_LIB  := $(MWARE_REL_LITEOS_PATH)/lib

##########################################################################################
# PQTool Configure
##########################################################################################
PQT_ROOT              := $(SDK_ROOT)/pqtool
PQT_STREAM_PATH       := $(PQT_ROOT)/Hi3559_Stream

ifeq ($(CONFIG_PQT_STREAM_SUPPORT_ON)_$(CONFIG_HI_HDMI_SUPPORT), y_y)
KCONFIG_CFLAGS        += -DHDMI_TEST
endif

##########################################################################################
# Reference Configure
##########################################################################################
REF_ROOT              := $(SDK_ROOT)/reference
REF_APPCOMMON_PATH    := $(REF_ROOT)/common
REF_HAL_PATH          := $(REF_ROOT)/hal
REF_THIRDPARTY_PATH   := $(REF_ROOT)/thirdparty
REF_PDT_PATH          := $(REF_ROOT)/$(CFG_PDT_TYPE)


#++++++++++++++++++++++++++++++++++++++++++++++++++++
#	DashCam Type
#++++++++++++++++++++++++++++++++++++++++++++++++++++
ifeq ($(CONFIG_CARRECORDER), y)
CFG_DASHCAM_TYPE     := carrecorder
ifneq ($(CONFIG_SCREEN), y)
ifeq ($(CONFIG_HI_HDMI_SUPPORT), y)
KCONFIG_CFLAGS       += -DHDMI_TEST
endif
endif
else ifeq ($(CONFIG_REARVIEW), y)
CFG_DASHCAM_TYPE     := rearview
KCONFIG_CFLAGS       += -DREARVIEW
ifneq ($(CONFIG_SCREEN), y)
ifeq ($(CONFIG_HI_HDMI_SUPPORT), y)
KCONFIG_CFLAGS       += -DHDMI_TEST
endif
endif
else ifeq ($(CONFIG_NONESCREEN), y)
CFG_DASHCAM_TYPE     := nonescreen
KCONFIG_CFLAGS       += -DNONESCREEN
endif

ifeq ($(CONFIG_SAMPLECAM), y)
ifeq ($(CONFIG_HI_HDMI_SUPPORT), y)
KCONFIG_CFLAGS       += -DHDMI_SUPPORT
endif
endif

endif # ifndef $(SDK_BASECFG)
