#
# Automatically generated file; DO NOT EDIT.
# BVT SDK Platform Configuration
#

#
# General Setup
#
# CONFIG_HI3559AV100ES is not set
# CONFIG_HI3559AV100 is not set
# CONFIG_HI3519AV100 is not set
export CONFIG_HI3516CV500=y
# CONFIG_HI3516EV200 is not set
export CONFIG_HI_CHIP_TYPE=0x3516C500
export CONFIG_HI_ARCH=hi3516cv500
export CONFIG_HI_SUBARCH=hi3559v200
export CONFIG_HI_SUBCHIP_TYPE=0x35590200
# CONFIG_SUBCHIP_HI3516CV500 is not set
export CONFIG_SUBCHIP_HI3559V200=y
# CONFIG_SMP is not set
export CONFIG_AMP=y
export CONFIG_ARM_ARCH_TYPE=amp
export CONFIG_A7=y
export CONFIG_CPU_TYPE=a7
# CONFIG_VERSION_FPGA is not set
export CONFIG_VERSION_ASIC=y
export CONFIG_HI_FPGA=n
export CONFIG_LINUX_OS=y
# CONFIG_HUAWEI_LITEOS_OS is not set
export CONFIG_OS_TYPE=linux
export CONFIG_LINUX_4_9_y=y
export CONFIG_KERNEL_VERSION=linux-4.9.y
export CONFIG_KERNEL_ARM_HIMIX100_LINUX=y
# CONFIG_KERNEL_ARM_HIMIX200_LINUX is not set
export CONFIG_HI_CROSS=arm-himix100-linux-
export CONFIG_LIBC_TYPE=uclibc
export CONFIG_KERNEL_BIT=KERNEL_BIT_32
export CONFIG_USER_ARM_HIMIX100_LINUX=y
# CONFIG_USER_ARM_HIMIX200_LINUX is not set
export CONFIG_HI_CROSS_LIB=arm-himix100-linux-
export CONFIG_USER_BIT=USER_BIT_32
export CONFIG_RELEASE_TYPE_RELEASE=y
# CONFIG_RELEASE_TYPE_DEBUG is not set
export CONFIG_HI_RLS_MODE=HI_RELEASE

#
# Media Modules Setup
#

#
# media base config
#
# CONFIG_HI_VB_EXTPOOL_SUPPORT is not set

#
# media sys config
#
export CONFIG_HI_SYS_SUPPORT=y
# CONFIG_HI_SYS_SCALE_COEF_SUPPORT is not set

#
# media vi config
#
# CONFIG_HI_VI_SUPPORT is not set

#
# media isp config
#
# CONFIG_HI_ISP_SUPPORT is not set

#
# media dis config
#
# CONFIG_HI_DIS_SUPPORT is not set

#
# media vpss config
#
# CONFIG_HI_VPSS_SUPPORT is not set

#
# media avs config
#

#
# media gdc config
#
# CONFIG_HI_GDC_SUPPORT is not set

#
# media vgs config
#
# CONFIG_HI_VGS_SUPPORT is not set

#
# media chnl config
#
# CONFIG_HI_CHNL_SUPPORT is not set

#
# media venc config
#

#
# media vdec config
#
# CONFIG_HI_VDEC_SUPPORT is not set

#
# media vo config
#
# CONFIG_HI_VO_SUPPORT is not set
export CONFIG_HI_VO_FB_SEPARATE=y

#
# media region config
#
# CONFIG_HI_REGION_SUPPORT is not set
# CONFIG_HI_RGN_DIFFPIXOSD_SUPPORT is not set

#
# media audio config
#
# CONFIG_HI_AUDIO_SUPPORT is not set

#
# media hdr config
#
#
# media mcf config
#
# CONFIG_HI_MONO_COLOR_FUSION_SUPPORT is not set

#
# Device Driver Setup
#

#
# drv config
#
export CONFIG_DRV=y
export CONFIG_EXTDRV=y
export CONFIG_INTERDRV=y
export CONFIG_CIPHER=y
# CONFIG_HIUSER is not set
# CONFIG_MIPI_TX is not set
# CONFIG_MIPI_RX is not set
export CONFIG_HI_IR=y
export CONFIG_HI_WDG=y
export CONFIG_HI_LSADC=y

#
# Component Setup
#

#
# Component hdmi Config
#
# CONFIG_HI_HDMI_SUPPORT is not set

#
# Component hifb Config
#
export CONFIG_HI_HIFB_SUPPORT=y

#
# Component svp Config
#
export CONFIG_HI_SVP_SUPPORT=y
export CONFIG_HI_SVP_CNN=y
export CONFIG_HI_SVP_RUNTIME=y
export CONFIG_HI_SVP_IVE=y
export CONFIG_HI_SVP_IVE_CSC=y
export CONFIG_HI_SVP_IVE_FILTER_AND_CSC=y
export CONFIG_HI_SVP_IVE_NCC=y
export CONFIG_HI_SVP_IVE_LBP=y
export CONFIG_HI_SVP_IVE_NORM_GRAD=y
export CONFIG_HI_SVP_IVE_ST_CANDI_CORNER=y
export CONFIG_HI_SVP_IVE_RESIZE=y
# CONFIG_HI_SVP_IVE_PERSP_TRANS is not set
# CONFIG_HI_SVP_IVE_KCF is not set
# CONFIG_HI_SVP_IVE_HOG is not set
export CONFIG_HI_SVP_MD=y

#
# Component photo Config
#
# CONFIG_HI_PHOTO_SUPPORT is not set

#
# Component tde Config
#
export CONFIG_HI_TDE_SUPPORT=y

#
# Component pciv Config
#
# CONFIG_HI_PCIV_SUPPORT is not set

#
# Component avs lut Config
#
# CONFIG_HI_AVS_LUT_SUPPORT is not set

#
# Component pos_query Config
#
# CONFIG_HI_POS_QUERY_SUPPORT is not set

#
# Component tzasc Config
#
# CONFIG_HI_TZASC_SUPPORT is not set

#
# Component motionfusion config
#
# CONFIG_HI_MOTIONFUSION_SUPPORT is not set

#
# Component pm Config
#
# CONFIG_HI_PM_SUPPORT is not set

#
# HISYSLINK Setup
#

#
# hisyslink config
#
export CONFIG_HISYSLINK=y

#
# Debug Config
#
export CONFIG_HI_GDB_NO=y
# CONFIG_HI_GDB_YES is not set
export CONFIG_HI_GDB=n
export CONFIG_HI_LOG_TRACE_SUPPORT=y
export CONFIG_HI_LOG_TRACE_ALL=y
# CONFIG_HI_LOG_TRACE_ERR is not set
# CONFIG_HI_LOG_TRACE_WARN is not set
# CONFIG_HI_LOG_TRACE_INFO is not set
export CONFIG_HI_LOG_TRACE_LEVEL=7

#
# Test Config
#
