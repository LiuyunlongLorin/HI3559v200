# define MEDIA_LIBRARY_PATH and MEDIA_LIBS in different chip dir
CURRENT_DIR := $(shell pwd)
include $(MIDDLEWARE_DIR)/base.mak
HIARCH := hi3559v200
NDK_ARCH := hi3559v200
SDK_LINK_ROOT := $(DEP_DIR)
NDK_LINK_ROOT := ndk
MW_MEDIA_INCLUDE_LITEOS := $(NDK_LINK_ROOT)/out/$(NDK_ARCH)/dual/slave/include \
                                          $(SDK_LINK_ROOT)/amp/a7_liteos/mpp/include \
					  $(SDK_LINK_ROOT)/amp/a7_liteos/hisyslink/datafifo/include \
					  $(SDK_LINK_ROOT)/amp/a7_liteos/hisyslink/ipcmsg/include

MW_MEDIA_DEPRECATED_INC := 

MW_MEDIA_LIBRARY_PATH_LITEOS := . \
					  $(NDK_LINK_ROOT)/out/$(NDK_ARCH)/dual/slave/lib \
					  $(SDK_LINK_ROOT)/amp/a7_liteos/mpp/lib \
					  $(SDK_LINK_ROOT)/amp/a7_liteos/mpp/lib/extdrv \
					  $(SDK_LINK_ROOT)/amp/a7_liteos/hisyslink/datafifo/out/a7_liteos \
					  $(SDK_LINK_ROOT)/amp/a7_liteos/hisyslink/ipcmsg/out/a7_liteos

# include sensor lib

SENSOR_LIBS =
SENSOR_TYPE0 = $(shell echo $(CFG_SENSOR_TYPE0) | tr A-Z a-z)
SENSOR_TYPE1 = $(shell echo $(CFG_SENSOR_TYPE1) | tr A-Z a-z)
SENSOR_TYPE2 = $(shell echo $(CFG_SENSOR_TYPE2) | tr A-Z a-z)
SENSOR_TYPE3 = $(shell echo $(CFG_SENSOR_TYPE3) | tr A-Z a-z)
SENSOR_TYPE4 = $(shell echo $(CFG_SENSOR_TYPE4) | tr A-Z a-z)

ifneq ($(CFG_SENSOR_TYPE0), )
	ifneq ($(SENSOR_TYPE0), unused)
		ifeq ($(SENSOR_TYPE0), imx335)
			SENSOR_LIBS += sns_imx335_forcar
		else ifeq ($(SENSOR_TYPE0), imx307)
			ifeq ($(SENSOR_CABLE_TYPE), LANE_DIVIDE_MODE_1)
				SENSOR_LIBS += sns_imx307_2l
			else
				SENSOR_LIBS += sns_$(SENSOR_TYPE0)
			endif
		else
			SENSOR_LIBS += sns_$(SENSOR_TYPE0)
		endif
		SENSOR_LIBS += $(HIARCH)_mapi_cfg_$(SENSOR_TYPE0)
	endif
endif

ifneq ($(CFG_SENSOR_TYPE1), )
	ifneq ($(SENSOR_TYPE1), unused)
		ifneq ($(SENSOR_TYPE1), bt656)
			ifeq ($(SENSOR_TYPE1), imx335)
				SENSOR_LIBS += sns_imx335_forcar
			else ifeq ($(SENSOR_TYPE1), imx307)
				ifeq ($(SENSOR_CABLE_TYPE), LANE_DIVIDE_MODE_1)
					SENSOR_LIBS += sns_imx307_2l
				else
					SENSOR_LIBS += sns_$(SENSOR_TYPE1)
				endif
			else
				SENSOR_LIBS += sns_$(SENSOR_TYPE1)
			endif
		endif
		SENSOR_LIBS += $(HIARCH)_mapi_cfg_$(SENSOR_TYPE1)
	endif
endif

ifneq ($(CFG_SENSOR_TYPE2), )
ifneq ($(SENSOR_TYPE2), unused)
SENSOR_LIBS += sns_$(SENSOR_TYPE2)
SENSOR_LIBS += $(HIARCH)_mapi_cfg_$(SENSOR_TYPE2)
endif
endif

ifneq ($(CFG_SENSOR_TYPE3), )
ifneq ($(SENSOR_TYPE3), unused)
SENSOR_LIBS += sns_$(SENSOR_TYPE3)
SENSOR_LIBS += $(HIARCH)_mapi_cfg_$(SENSOR_TYPE3)
endif
endif

ifneq ($(CFG_SENSOR_TYPE4), )
ifneq ($(SENSOR_TYPE4), unused)
SENSOR_LIBS += sns_$(SENSOR_TYPE4)
SENSOR_LIBS += $(HIARCH)_mapi_cfg_$(SENSOR_TYPE4)
endif
endif

NDK_LIB_A := $(NDK_ARCH)_mapi_log $(NDK_ARCH)_media_server $(NDK_ARCH)_mapi_sys \
             $(NDK_ARCH)_mapi_vcap_inner_cfg $(NDK_ARCH)_mapi_register_sensor \
	     $(NDK_ARCH)_mapi_comm_sensor_cfg $(NDK_ARCH)_mapi_vcap $(NDK_ARCH)_mapi_venc \
              $(NDK_ARCH)_mapi_vproc $(NDK_ARCH)_mapi_acap \
	     $(NDK_ARCH)_mapi_aenc  $(NDK_ARCH)_mapi_ao  \
	     $(NDK_ARCH)load_sdk  $(NDK_ARCH)_mapi_disp
	     #-l$(NDK_ARCH)_mapi_pm
SDK_LIB := hi_osal mpi $(HIARCH)_base $(HIARCH)_sys hi_user hdmi $(HIARCH)_isp $(HIARCH)_vi \
							$(HIARCH)_vo $(HIARCH)_vpss $(HIARCH)_vgs $(HIARCH)_gdc hi_mipi_rx \
							$(HIARCH)_chnl $(HIARCH)_rc $(HIARCH)_rgn $(HIARCH)_vedu \
							$(HIARCH)_venc $(HIARCH)_h265e $(HIARCH)_jpege $(HIARCH)_h264e \
							$(HIARCH)_dis  $(HIARCH)_hdmi \
							_hidehaze _hidrc _hildci _hiawb _hiae isp  hi_sensor_i2c \
							aacdec aacenc upvqe dnvqe VoiceEngine $(HIARCH)_ai $(HIARCH)_ao $(HIARCH)_aio $(HIARCH)_aenc $(HIARCH)_adec $(HIARCH)_acodec aaccomm \
							$(HIARCH)_vdec $(HIARCH)_jpegd $(HIARCH)_vfmw \
							hi_ssp_sony hi_sensor_spi hi_pwm \
							datafifo_a7_liteos ipcmsg_a7_liteos \
							hi_ssp_st7789 hi_mipi_tx
ifeq ($(CFG_SUPPORT_GYRO), y)
	SDK_LIB += motionfusion motionsensor_chip motionsensor_mng $(HIARCH)_gyrodis $(HIARCH)_motionfusion
endif
MW_MEDIA_LIBS_LITEOS := mw_media_adpt_liteos uproc $(NDK_LIB_A) $(SENSOR_LIBS) $(SDK_LIB)
