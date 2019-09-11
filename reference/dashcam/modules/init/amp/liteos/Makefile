
# Common Definition
OS_TYPE     := HuaweiLite
CUR_ROOT    := $(shell pwd)
PDT_ROOT    := $(CUR_ROOT)/../../../..
include $(PDT_ROOT)/build/base.mak
#add pci include for sdk_init
VSS_INC_PATH += -I$(MPP_REL_LITEOS_INC_PATH)/../../../../component/pci/include
VSS_INC_PATH += -I$(OSAL_LITEOS_PATH)/include

# Lib
VSS_LIB :=

# MPP Libs
VSS_LIB +=  -lhi_osal -lmpi -l$(CFG_CHIP_TYPE)_base -l$(CFG_CHIP_TYPE)_sys \
            -lhi_user -l$(CFG_CHIP_TYPE)_isp -l$(CFG_CHIP_TYPE)_vi \
            -l$(CFG_CHIP_TYPE)_vo -l$(CFG_CHIP_TYPE)_vpss -l$(CFG_CHIP_TYPE)_vgs \
            -l$(CFG_CHIP_TYPE)_gdc \
            -l$(CFG_CHIP_TYPE)_chnl -l$(CFG_CHIP_TYPE)_rc -l$(CFG_CHIP_TYPE)_rgn -l$(CFG_CHIP_TYPE)_vedu \
            -l$(CFG_CHIP_TYPE)_venc -l$(CFG_CHIP_TYPE)_h265e -l$(CFG_CHIP_TYPE)_jpege -l$(CFG_CHIP_TYPE)_h264e \
            -l_hidehaze -l_hidrc -l_hildci -l_hiawb_natura -l_hiae -lisp  -lhi_sensor_i2c\
            -laacdec -laacenc -lupvqe -ldnvqe -lVoiceEngine -l$(CFG_CHIP_TYPE)_ai -l$(CFG_CHIP_TYPE)_ao \
            -l$(CFG_CHIP_TYPE)_aio -l$(CFG_CHIP_TYPE)_aenc -l$(CFG_CHIP_TYPE)_adec -l$(CFG_CHIP_TYPE)_acodec \
            -l$(CFG_CHIP_TYPE)_vdec -l$(CFG_CHIP_TYPE)_jpegd  -l$(CFG_CHIP_TYPE)_vfmw  -l$(CFG_CHIP_TYPE)_rtc\
            -lhi_sensor_spi -lhi_pwm
ifeq ($(CONFIG_LDC_SUPPORT),y)
VSS_LIB +=  -l$(CFG_CHIP_TYPE)_gdc
endif
ifeq ($(CFG_CHIP_TYPE),hi3556av100)
VSS_LIB +=  -lhi_tzasc  -lhi_mipi_rx
VSS_LIB +=  $(HISYSLINK_IPCMSG_LIB_LITEOS)/libipcmsg_a53_liteos.a
VSS_LIB +=  $(HISYSLINK_DATAFIFO_LIB_LITEOS)/libdatafifo_a53_liteos.a
VSS_LIB +=  -ldsp
else ifeq ($(CFG_CHIP_TYPE),hi3519av100)
VSS_LIB +=  -lhi_tzasc  -lhi_mipi_rx
VSS_LIB +=  $(HISYSLINK_IPCMSG_LIB_LITEOS)/libipcmsg_a53_liteos.a
VSS_LIB +=  $(HISYSLINK_DATAFIFO_LIB_LITEOS)/libdatafifo_a53_liteos.a
VSS_LIB +=  -ldsp
else ifeq ($(CFG_CHIP_TYPE),hi3559v200)
VSS_LIB +=  -laaccomm -lhi_mipi_rx -l$(CFG_CHIP_TYPE)_pm -lhi_ssp_sony
VSS_LIB +=  $(HISYSLINK_IPCMSG_LIB_LITEOS)/libipcmsg_a7_liteos.a
VSS_LIB +=  $(HISYSLINK_DATAFIFO_LIB_LITEOS)/libdatafifo_a7_liteos.a
endif

# NDK Libs
VSS_LIB +=  -l$(CFG_CHIP_TYPE)_mapi_log -l$(CFG_CHIP_TYPE)_media_server -l$(CFG_CHIP_TYPE)_mapi_sys \
            -l$(CFG_CHIP_TYPE)_mapi_vcap_inner_cfg -l$(CFG_CHIP_TYPE)_mapi_register_sensor \
            -l$(CFG_CHIP_TYPE)_mapi_comm_sensor_cfg -l$(CFG_CHIP_TYPE)_mapi_vcap -l$(CFG_CHIP_TYPE)_mapi_venc \
            -l$(CFG_CHIP_TYPE)_mapi_disp -l$(CFG_CHIP_TYPE)_mapi_vproc -l$(CFG_CHIP_TYPE)_mapi_acap \
            -l$(CFG_CHIP_TYPE)_mapi_aenc  -l$(CFG_CHIP_TYPE)_mapi_ao

# MW Libs
VSS_LIB +=  -lavplay_liteos -lfsm_liteos

# Sensor Libs
VSS_LIB += -L$(PDT_OUT_LIB_HUAWEILITE)
ifeq ($(CFG_CHIP_TYPE)_$(CFG_SENSOR_TYPE0)_$(MIPI_RX_LANE_DIVIDE_MODE),hi3559v200_imx307_LANE_DIVIDE_MODE_1)
VSS_LIB += -lsns_$(CFG_SENSOR_TYPE0)_2l
else ifeq ($(CFG_SENSOR_TYPE0),imx335)
VSS_LIB += -lsns_$(CFG_SENSOR_TYPE0)_forcar
else
VSS_LIB += -lsns_$(CFG_SENSOR_TYPE0)
endif
VSS_LIB += -l$(CFG_CHIP_TYPE)_mapi_cfg_$(CFG_SENSOR_TYPE0)

ifneq ($(findstring $(CFG_SENSOR_TYPE1),bt656_1080p30 bt656_1080p25 bt656_720p30 bt656_720p25),)
VSS_LIB += -l$(CFG_CHIP_TYPE)_mapi_cfg_bt656
endif

# Gyro Lib
ifeq ($(CFG_GYRO_ON),yes)
VSS_LIB += -lgyro_bosch
endif

# LITEOS Libs
VSS_LIB += $(LITEOS_LIBS) -lsharefs -lipcm -lvirt-tty
ifeq ($(CONFIG_SCREEN),y)
VSS_LIB += -lhalscreen_server
ifeq ($(CONFIG_SCREEN_OTA5182),y)
VSS_LIB += -lhi_ssp_ota5182 -lhalscreen_ota5182_server
else ifeq ($(CONFIG_SCREEN_ST7796S),y)
VSS_LIB += -lhalscreen_st7796s_server -lhi_mipi_tx
else ifeq ($(CONFIG_SCREEN_ST7789),y)
VSS_LIB += -lhi_ssp_st7789 -lhalscreen_st7789_server
else ifeq ($(CONFIG_SCREEN_ST7796S_RGB16BIT),y)
VSS_LIB += -lhi_ssp_st7796s_rgb16bit -lhalscreen_st7796rgb_server
else ifeq ($(CONFIG_SCREEN_OTA7290B),y)
VSS_LIB += -lhalscreen_ota7290b_server -lhi_mipi_tx
endif
ifeq ($(CONFIG_PQT_SUPPORT_ON), y)
ifeq ($(CONFIG_PQT_STREAM_SUPPORT_ON), y)
VSS_LIB += -lhdmi
VSS_LIB += -l$(CFG_CHIP_TYPE)_hdmi
endif
endif
else
VSS_LIB += -lhdmi
VSS_LIB += -l$(CFG_CHIP_TYPE)_hdmi
endif

ifeq ($(CONFIG_AHD),y)
VSS_LIB += -lhalahd
ifeq ($(CONFIG_AHD_N3),y)
VSS_LIB += -lhi_n3
endif
endif

# PDT Libs
VSS_LIB += -L$(PDT_OUT_MODULES_LIB)/$(OS_TYPE) -lmsg_server -lparam_slave -lmedia_server -lscene_server -ltimestamp -lhilog -lhiosd_server -lsystem
ifeq ($(CONFIG_PQT_SUPPORT_ON), y)
ifeq ($(CONFIG_PQT_STREAM_SUPPORT_ON), y)
VSS_LIB += -lstream
endif
VSS_LIB += -lcontrol
endif

ifeq ($(CONFIG_TEMPRATURE_CYCLE),y)
VSS_LIB += -lhitempcycle_server -lpm
endif

LIB_OMIT := -lmulti_partition -lnfs -ltelnet -lgcc -lsupc++
ifneq ($(strip $(LIB_OMIT)),)
VSS_LIB := $(filter-out $(LIB_OMIT),$(VSS_LIB))
endif
#++++++++++++++++++++++++++++++++++++++++++++++++++++
#	Compile Path
#++++++++++++++++++++++++++++++++++++++++++++++++++++
SRC_ROOT    := $(shell cd $(SDK_ROOT);pwd)
SRC_DIR     := $(CUR_ROOT)/src
OBJ_ROOT    := $(PDT_OUT_MODULES_OBJ)/$(CUR_PARENT_DIR_NAME)/$(OS_TYPE)
BIN_ROOT    := $(PDT_OUT_BIN)/$(CUR_DIR_NAME)

ifeq ($(CONFIG_BOOT_LOGO), y)
BOOT_LOGO_BMP :=$(SRC_DIR)/bootlogo.bmp
BOOT_LOGO_YUV :=$(SRC_DIR)/bootlogo.yuv
BOOT_LOGO_DAT :=$(SRC_DIR)/bootlogo.dat
endif

ifeq ($(CONFIG_BOOT_SOUND), y)
BOOT_SOUND_WAV :=$(SRC_DIR)/bootsound.wav
BOOT_SOUND_PCM :=$(SRC_DIR)/bootsound.pcm
BOOT_SOUND_DAT :=$(SRC_DIR)/bootsound.dat
endif

#++++++++++++++++++++++++++++++++++++++++++++++++++++
#	Source FileList
#++++++++++++++++++++++++++++++++++++++++++++++++++++
SRCS      := $(shell find $(SRC_DIR) -name '*.c')
SRCS      := $(sort $(SRCS))
SRCS_OMIT := $(shell find $(SRC_DIR) -name 'hi_product_init_hi35*.c')
ifneq ($(strip $(SRCS_OMIT)),)
SRCS      := $(filter-out $(SRCS_OMIT),$(SRCS))
endif
SRCS      += $(CUR_ROOT)/src/hi_product_init_$(CFG_CHIP_TYPE).c
SRCS      += $(LITEOS_ROOT)/kernel/extended/dynload/src/los_dynload_gsymbol.c
ifeq ($(CONFIG_PQT_SUPPORT_ON), y)
ifeq ($(CONFIG_PQT_STREAM_SUPPORT_ON), y)
SRCS      += $(PQT_PATH)/pq_sample/pq_stream.c
endif
SRCS      += $(PQT_PATH)/pq_sample/pq_control.c
endif

#++++++++++++++++++++++++++++++++++++++++++++++++++++
#	Object FileList
#++++++++++++++++++++++++++++++++++++++++++++++++++++
OBJS      := $(SRCS:$(SRC_ROOT)%.c=$(OBJ_ROOT)%.o)
OBJS      := $(sort $(OBJS))
OBJ_DIR   := $(sort $(dir $(OBJS)))
SRC_DEPS  := $(OBJS:%.o=%.d)

#++++++++++++++++++++++++++++++++++++++++++++++++++++
#	Module Specified Configure
#++++++++++++++++++++++++++++++++++++++++++++++++++++
MOD_INC_PATH := $(sort $(dir $(shell find $(CUR_ROOT) -name '*.h')))
VSS_INC_PATH += $(foreach dir, $(MOD_INC_PATH), -I$(dir))
ifeq ($(CFG_PQT_SUPPORT_ON),yes)
VSS_INC_PATH += -I$(PQT_STREAM_PATH)/slave/libhal_open/include
endif

#++++++++++++++++++++++++++++++++++++++++++++++++++++
#	Create Directory
#++++++++++++++++++++++++++++++++++++++++++++++++++++
CreateResult :=
dummy := $(call CreateDir, $(OBJ_ROOT))
dummy += $(call CreateDir, $(BIN_ROOT))
dummy += $(foreach dir, $(OBJ_DIR), CreateResult += $(call CreateDir, $(dir)))
ifneq ($(strip CreateResult),)
    err = $(error $(CreateResult))
endif

#++++++++++++++++++++++++++++++++++++++++++++++++++++
#	Target
#++++++++++++++++++++++++++++++++++++++++++++++++++++
TARGET     := $(BIN_ROOT)/media_app
TARGET_BIN := $(TARGET).bin
TARGET_MAP := $(TARGET).map
LINK_MAP = -Map=$(TARGET_MAP)

#++++++++++++++++++++++++++++++++++++++++++++++++++++
#	Compile Command
#++++++++++++++++++++++++++++++++++++++++++++++++++++
COMPILE      = $(CC) $(VSS_CFLAGS) -Wall -c "$<" -o "$@" $(VSS_INC_PATH)
COMPILEDEP   = $(CCDEP) -MM "$<"  $(VSS_CFLAGS) $(VSS_INC_PATH)
LINK         = $(CC) $(VSS_CFLAGS) -Wall
VSS_LDFLAGS := $(LITEOS_LDFLAGS) -nostartfiles -static --gc-sections

#++++++++++++++++++++++++++++++++++++++++++++++++++++
#	Compiler
#++++++++++++++++++++++++++++++++++++++++++++++++++++
.PHONY: all clean target
default: all

all: prepare target success install

success:
	@echo ""
	@echo -e "\033[31;32m `basename $(TARGET_BIN)` Was SUCCESSFUL COMPILE \033[0m"
	@echo -e "\033[31;32m *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ \033[0m"
	@echo "";echo ""

install:
	@cp -f $(TARGET_BIN) $(PDT_OUT_BURN)
	@-cd $(OSDRV_PUB_PC_BIN_PATH);./hi_gzip $(PDT_OUT_BURN)/media_app.bin $(PDT_OUT_BURN)/media_app_zip.bin;cd -

prepare:
	@echo "";echo ""
	@echo -e "\033[31;32m *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ \033[0m"
	@echo -e "\033[31;32m [COMPILING] `basename $(TARGET_BIN)` ... ... \033[0m"
	@echo ""
	@cp -f $(MWARE_REL_LITEOS_LIB)/libhiaacdec.so $(PDT_OUT_LIB_HUAWEILITE)/
	@cd $(MPP_REL_LITEOS_LIB_PATH);if [ -f libhive_record.so ];then cp -f libhive_record.so $(PDT_OUT_LIB_HUAWEILITE)/;fi;cd -
	@#cd $(MPP_REL_LITEOS_LIB_PATH);if [ -f libhive_RES.so ];then cp -f libhive_RES.so $(PDT_OUT_LIB_HUAWEILITE)/;fi;cd -
	@find $(PDT_OUT_LIB_HUAWEILITE)/ -name *.so
	@cd $(OSDRV_LITEOS_DYNLOAD_TOOLS);./sym.sh $(PDT_OUT_LIB_HUAWEILITE);cd -
ifeq ($(CONFIG_BOOT_LOGO), y)
ifeq ($(CONFIG_LCD_UISRC_ADAPT_AUTO), y)
	@if [ ! -f  "$(BOOT_LOGO_DAT)" ]; then\
		echo -e "\033[31;32m [PRODUCE] `basename $(BOOT_LOGO_DAT)` ... ... \033[0m";\
		ffmpeg -i $(BOOT_LOGO_BMP) -pix_fmt nv21 -s $(CONFIG_LCD_UISRC_DEST_WIDTH)x$(CONFIG_LCD_UISRC_DEST_HEIGHT) $(BOOT_LOGO_YUV);\
		cd $(SRC_DIR);xxd -c 16 -i `basename $(BOOT_LOGO_YUV)` > $(BOOT_LOGO_DAT);cd -;\
		rm $(BOOT_LOGO_YUV);\
	fi
else
	@if [ ! -f  "$(BOOT_LOGO_DAT)" ]; then\
		echo -e "\033[31;32m [PRODUCE] `basename $(BOOT_LOGO_DAT)` ... ... \033[0m";\
		ffmpeg -i $(BOOT_LOGO_BMP) -pix_fmt nv21 $(BOOT_LOGO_YUV);\
		cd $(SRC_DIR);xxd -c 16 -i `basename $(BOOT_LOGO_YUV)` > $(BOOT_LOGO_DAT);cd -;\
		rm $(BOOT_LOGO_YUV);\
	fi
endif
endif
ifeq ($(CONFIG_BOOT_SOUND), y)
	@if [ ! -f  "$(BOOT_SOUND_DAT)" ]; then\
		echo -e "\033[31;32m [PRODUCE] `basename $(BOOT_SOUND_DAT)` ... ... \033[0m";\
		ffmpeg -i $(BOOT_SOUND_WAV) -f s16le -ar 48000 -ac 1 -acodec pcm_s16le $(BOOT_SOUND_PCM);\
		cd $(SRC_DIR);xxd -c 16 -i `basename $(BOOT_SOUND_PCM)` > $(BOOT_SOUND_DAT);cd -;\
		rm $(BOOT_SOUND_PCM);\
	fi
endif
	#refresh Compile Time
	@touch $(CUR_ROOT)/src/hi_product_init_os.c

target: $(SRC_DEPS) $(OBJS)
	@cp $(MPP_REL_LITEOS_LIB_PATH)/*.a $(PDT_OUT_LIB_HUAWEILITE)/
	@cp $(MPP_REL_LITEOS_EXT_LIB_PATH)/*.a $(PDT_OUT_LIB_HUAWEILITE)/
	@cp $(NDK_MAPI_REL_LITEOS_LIB)/*.a $(PDT_OUT_LIB_HUAWEILITE)/
	@cp $(HISYSLINK_IPCMSG_LIB_LITEOS)/*.a $(PDT_OUT_LIB_HUAWEILITE)/
	@cp $(HISYSLINK_DATAFIFO_LIB_LITEOS)/*.a $(PDT_OUT_LIB_HUAWEILITE)/
	@-cp $(DRV_REL_PATH_LITEOS)/*.a $(PDT_OUT_LIB_HUAWEILITE)/
	@cp $(MWARE_REL_LITEOS_LIB)/*.a $(PDT_OUT_LIB_HUAWEILITE)
	@cp $(REF_OUT_COMMON_LIB)/$(OS_TYPE)/*.a $(PDT_OUT_LIB_HUAWEILITE)
	@if [ -d $(REF_OUT_HAL_LIB)/$(OS_TYPE) ];then cp $(REF_OUT_HAL_LIB)/$(OS_TYPE)/*.a $(PDT_OUT_LIB_HUAWEILITE);fi
ifeq ($(CONFIG_PQT_SUPPORT_ON), y)
ifeq ($(CONFIG_PQT_STREAM_SUPPORT_ON), y)
	@cp $(PQT_PATH)/pq_sample/libstream.a $(PDT_OUT_LIB_HUAWEILITE)/
endif
	@cp $(PQT_PATH)/pq_sample/libcontrol.a $(PDT_OUT_LIB_HUAWEILITE)/
endif
	@$(LD) $(VSS_LDFLAGS)  $(LINK_MAP) -o $(TARGET) $(OBJS) --start-group $(VSS_LIB) --end-group  $(LITEOS_TABLES_LDFLAGS) -lgcc
	@$(OBJDUMP) -d $(TARGET) > $(TARGET).sym
	@$(OBJCOPY) -O binary $(TARGET) $(TARGET_BIN)

$(SRC_DEPS) : $(OBJ_ROOT)/%.d : $(SRC_ROOT)/%.c
	@#echo "SRC_DEPS: " $(notdir $@)
	@set -e;$(COMPILEDEP) > $@.$$$$; \
	sed 's,.*\.o[ :]*,$(@:%.d=%.o) $@ : ,g' < $@.$$$$ > $@; rm -f $@.$$$$

$(OBJS)     : $(OBJ_ROOT)/%.o : $(SRC_ROOT)/%.c
	@echo -e "\033[31;32m [$(CC)]  \033[0m `basename $<` \033[0m"
	@$(COMPILE)
	@echo -e "\033[31;32m       [OK] \033[0m";echo ""

clean:
	@echo " "
	@echo "--------------------------------------------"
	@echo -e "\033[31;31m [DELETING Tmp files ......] \033[0m"
	@-rm -f $(SRC_DEPS)
	@if [ -n $(OBJ_ROOT) ];then rm -rf $(OBJ_ROOT);fi
	@echo -e "\033[31;31m [DELETING `basename $(TARGET)` ......] \033[0m"
	@rm -f $(TARGET) $(TARGET_BIN) $(TARGET_MAP) *.sym
	@echo "--------------------------------------------"
	@echo " "
