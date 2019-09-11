# DRV Include
VSS_INC_PATH += -I$(DRV_EXTDRV_PATH_LITEOS)/ahd/n3
ifeq ($(CONFIG_HI3518EV300), y)
VSS_INC_PATH += -I$(DRV_WIFI_DRV_PATH)/drv/sdio_rtl8189ftv/os_dep/liteos
VSS_INC_PATH += -I$(DRV_WIFI_DRV_PATH)/drv/sdio_rtl8189ftv/include/
VSS_INC_PATH += -I$(DRV_WIFI_DRV_PATH)/tools/wpa_supplicant-2.2/hostapd/
endif

# OSDRV Include
VSS_INC_PATH += -I$(OSDRV_COMPONENT_IPCM)/include
VSS_INC_PATH += -I$(OSDRV_COMPONENT_PATH)/sharefs/sharefs/include
VSS_INC_PATH += -I$(LITEOS_ROOT)/lib/zlib/include
VSS_INC_PATH += -I$(LINUX_ROOT)/include/uapi/linux/usb
VSS_INC_PATH += -I$(LINUX_ROOT)/drivers/usb/gadget/function

# MPP Include
VSS_INC_PATH += -I$(MPP_REL_LINUX_INC_PATH)
VSS_INC_PATH += -I$(MPP_REL_LITEOS_INC_PATH)
VSS_INC_PATH += -I$(HISYSLINK_IPCMSG_INC_PATH)
VSS_INC_PATH += -I$(MPP_REL_LITEOS_EXT_LIB_PATH)/motionsensor/include
VSS_INC_PATH += -I$(MPP_REL_LITEOS_EXT_LIB_PATH)/motionsensor/motionsensor_chip/pub
VSS_INC_PATH += -I$(MPP_REL_LITEOS_EXT_LIB_PATH)/motionsensor/motionsensor_mng/include

# NDK Include
VSS_INC_PATH += -I$(NDK_MAPI_INC_PATH)
ifeq ($(OS_TYPE),HuaweiLite)
VSS_INC_PATH += -I$(NDK_MAPI_REL_LITEOS_PATH)/include
else
VSS_INC_PATH += -I$(NDK_MAPI_REL_LINUX_PATH)/include
endif
VSS_INC_PATH += -I$(NDK_MAPI_CFG_PATH)/sensor/include
VSS_INC_PATH += -I$(NDK_MAPI_CFG_PATH)/dis/include
VSS_INC_PATH += -I$(NDK_MAPI_ADAPT_PATH)/aenc/include
VSS_INC_PATH += -I$(NDK_MAPI_ADAPT_PATH)/audio/include
VSS_INC_PATH += -I$(NDK_MAPI_ADAPT_PATH)/audio/aac/include
VSS_INC_PATH += -I$(NDK_MAPI_ADAPT_PATH)/gyro/include

# MiddleWare Include
VSS_INC_PATH += -I$(MWARE_REL_PATH)/include
VSS_INC_PATH += -I$(MWARE_REL_LINUX_INC)
VSS_INC_PATH += -I$(MWARE_COMMON_PATH)/include
VSS_INC_PATH += -I$(MWARE_COMMON_PATH)/msghandler/include
VSS_INC_PATH += -I$(MWARE_COMMON_PATH)/mbuffer/include
VSS_INC_PATH += -I$(MWARE_COMMON_PATH)/fsm/include
VSS_INC_PATH += -I$(MWARE_COMMON_PATH)/hitimer/include
VSS_INC_PATH += -I$(MWARE_COMPONENT_PATH)/fileformat/mp4/include
VSS_INC_PATH += -I$(MWARE_COMPONENT_PATH)/fileformat/ts/include
VSS_INC_PATH += -I$(MWARE_COMPONENT_PATH)/fileformat/xmp/include
VSS_INC_PATH += -I$(MWARE_COMPONENT_PATH)/fileformat/dng/include
VSS_INC_PATH += -I$(MWARE_COMPONENT_PATH)/fileformat/exif/include
VSS_INC_PATH += -I$(MWARE_COMPONENT_PATH)/dcf/include
VSS_INC_PATH += -I$(MWARE_COMPONENT_PATH)/dtcf/include
VSS_INC_PATH += -I$(MWARE_COMPONENT_PATH)/eventhub/include
VSS_INC_PATH += -I$(MWARE_COMPONENT_PATH)/recorder_pro/include
VSS_INC_PATH += -I$(MWARE_COMPONENT_PATH)/statemachine/hfsm/include
VSS_INC_PATH += -I$(MWARE_COMPONENT_PATH)/storage/include
VSS_INC_PATH += -I$(MWARE_COMPONENT_PATH)/fstool/include
VSS_INC_PATH += -I$(MWARE_COMPONENT_PATH)/higv/higv/include
VSS_INC_PATH += -I$(MWARE_COMPONENT_PATH)/higv/higo/$(LINUX_CROSS)/include
VSS_INC_PATH += -I$(MWARE_SAMPLE_PATH)/livestream/server_common/include
VSS_INC_PATH += -I$(MWARE_SAMPLE_PATH)/livestream/rtspserver/include
VSS_INC_PATH += -I$(MWARE_SAMPLE_PATH)/livestream/httpserver/include
VSS_INC_PATH += -I$(MWARE_COMPONENT_PATH)/liteplayer/include
VSS_INC_PATH += -I$(MWARE_COMPONENT_PATH)/liteplayer/include/liteos
VSS_INC_PATH += -I$(MWARE_THIRDPARTY_PATH)/timers/include
VSS_INC_PATH += -I$(MWARE_THIRDPARTY_PATH)/tiff/tiff/libtiff
VSS_INC_PATH += -I$(MWARE_THIRDPARTY_PATH)/ffmpeg/ffmpeg-y/install/include/