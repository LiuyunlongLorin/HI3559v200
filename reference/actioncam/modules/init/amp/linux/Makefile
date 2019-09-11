
OS_TYPE     := Linux
CUR_ROOT    := $(shell pwd)
PDT_ROOT    := $(CUR_ROOT)/../../../..
include $(PDT_ROOT)/build/base.mak

#++++++++++++++++++++++++++++++++++++++++++++++++++++
#	Compile Path
#++++++++++++++++++++++++++++++++++++++++++++++++++++
SRC_ROOT    := $(CUR_ROOT)/src
SRC_DIR     := $(SRC_ROOT)
OBJ_ROOT    := $(PDT_OUT_MODULES_OBJ)/$(CUR_PARENT_DIR_NAME)/$(OS_TYPE)
BIN_ROOT    := $(PDT_OUT_BIN)/$(CUR_DIR_NAME)

#++++++++++++++++++++++++++++++++++++++++++++++++++++
#	Source FileList
#++++++++++++++++++++++++++++++++++++++++++++++++++++
SRCS      := $(shell find $(SRC_DIR) -name '*.c')
SRCS      := $(sort $(SRCS))
ifneq ($(strip $(SRCS_OMIT)),)
SRCS      := $(filter-out $(SRCS_OMIT),$(SRCS))
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
TARGET     := $(BIN_ROOT)/main_app
EXE        := $(TARGET)
MAP        := $(TARGET).map

#++++++++++++++++++++++++++++++++++++++++++++++++++++
#	Compile Command
#++++++++++++++++++++++++++++++++++++++++++++++++++++
COMPILE      = $(CC) $(VSS_CFLAGS) -Wall  -Werror -c "$<" -o "$@" $(VSS_INC_PATH)  -ffunction-sections -fdata-sections
COMPILEDEP   = $(CCDEP) -MM "$<"  $(VSS_CFLAGS) $(VSS_INC_PATH)
LINK         = $(CC) $(VSS_CFLAGS) -Wall

#++++++++++++++++++++++++++++++++++++++++++++++++++++
#	Link Libs
#++++++++++++++++++++++++++++++++++++++++++++++++++++
PDT_LIBS  :=	-lparam_host -lstatemng -lui -lusbctrl -lnetctrl -lmedia_client -lupgrade

COMM_LIBS :=	-ldevmng -lphotomng -lrecordmng -lusb \
                -lutils -lhilog -ltimedtask -ltimestamp -lmsg_client -lhicycle -lhiflash \
                -lthttpd -lhisnet -lpowercontrol -lfiletrans -lliveserver -lplayback \
                -lsharefs -lscene_client -lsystem
ifeq ($(CONFIG_DCF), y)
COMM_LIBS +=    -lfilemng_dcf
else ifeq ($(CONFIG_DTCF), y)
COMM_LIBS +=    -lfilemng_dtcf
endif

HAL_LIBS  :=	-lhalkey  -lhalcommon

ifeq ($(CONFIG_SCREEN),y)
HAL_LIBS += -lhalscreen_client
ifeq ($(CONFIG_SCREEN_OTA5182),y)
HAL_LIBS += -lhalscreen_ota5182_client
else ifeq ($(CONFIG_SCREEN_ST7796S),y)
HAL_LIBS += -lhalscreen_st7796s_client
else ifeq ($(CONFIG_SCREEN_ST7789),y)
HAL_LIBS += -lhalscreen_st7789_client
else ifeq ($(CONFIG_SCREEN_ST7796S_RGB16BIT),y)
HAL_LIBS += -lhalscreen_st7796rgb_client
endif
endif

ifeq ($(CONFIG_TOUCHPAD),y)
HAL_LIBS += -lhaltouchpad
endif

ifeq ($(CONFIG_RAWDEBUG),y)
COMM_LIBS += -lrawcap
endif

ifeq ($(CONFIG_WIFI),y)
HAL_LIBS += -lhalwifi
endif

ifeq ($(CONFIG_GAUGE),y)
HAL_LIBS += -lhalgauge
endif

MWARE_LIBS:= 	-lhfsm -lfsm -leventhub -lmessagehandler \
                -lhitimer -ltimers -lmwlog -luproc\
                -ldcf_wrapper -ldcf_core -ldcf_impl -lstorage -lfstool \
                -lrecorder_pro -lfileformat -lts -lmp4 -lopenexif -lexif -ljpeg\
                -lrtspserver -lservercommon -lmbuf \
                -lhigv -lhigo -lhigoadp -lfreetype -lpng -lz \
                -lmsg_client -lavplay_linux -lliteplayer \
                -lmpf_demuxer
                # -luserproc
ifeq ($(CONFIG_DNG), y)
MWARE_LIBS += -ldng -ltiff
endif
MAPI_LIBS :=    -l$(CFG_CHIP_TYPE)_vcap_client -l$(CFG_CHIP_TYPE)_vproc_client \
                -l$(CFG_CHIP_TYPE)_venc_client -l$(CFG_CHIP_TYPE)_disp_client \
                -l$(CFG_CHIP_TYPE)_mapi_acap_client -l$(CFG_CHIP_TYPE)_mapi_aenc_client \
                -l$(CFG_CHIP_TYPE)_msg_client -l$(CFG_CHIP_TYPE)_sys_client \
                -l$(CFG_CHIP_TYPE)_mapi_ao_client \
                -l$(CFG_CHIP_TYPE)_log_client -lsecurec -ltde
ifeq ($(CONFIG_HI3559AV100),y)
MAPI_LIBS +=    -lipcmsg_big-little -ldatafifo_big-little
ifeq ($(CONFIG_POST_PROCESS),y)
MAPI_LIBS +=    -lmpi_photo -lmfnr_ap -lsfnr_ap -ldetail_ap -lhdr_ap -lsecurec -lxml_ap -ldsp
endif
else ifeq ($(CONFIG_HI3556AV100),y)
MAPI_LIBS +=    -lipcmsg_a53_linux -ldatafifo_a53_linux
else ifeq ($(CONFIG_HI3559V200),y)
MAPI_LIBS +=    -lipcmsg_a7_linux -ldatafifo_a7_linux
endif
MPP_LIBS  :=    -lmpi -luserproc
#--------------------------------------------------------------------

VSS_LIB    := -L$(PDT_OUT_LIB_LINUX) -L$(PDT_OUT_MODULES_LIB)/$(OS_TYPE) -L$(REF_OUT_THIRD_LIB)
VSS_LIB    += $(PDT_LIBS)
VSS_LIB    += $(COMM_LIBS)
VSS_LIB    += $(HAL_LIBS)
VSS_LIB    += $(MWARE_LIBS)
VSS_LIB    += $(MAPI_LIBS)
VSS_LIB    += $(MPP_LIBS)
VSS_LIB    += -lpthread -lm -lrt -lstdc++ -ldl -lz
LINK_MAP = -Wl,-Map,$(MAP)

#--------------------------------------------------------------------

#++++++++++++++++++++++++++++++++++++++++++++++++++++
#	Compiler
#++++++++++++++++++++++++++++++++++++++++++++++++++++
.PHONY: all clean target
default: all

all: prepare target success

all : prepare $(SRC_DEPS) $(OBJS) target success

prepare:
	@echo "";echo ""
	@echo -e "\033[31;32m *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ \033[0m"
	@echo -e "\033[31;32m [COMPILING] `basename $(TARGET)` ... ... \033[0m"
	@echo -e "\033[31;32m [COMPILING] $(VSS_CFLAGS) ... ... \033[0m"
	@echo ""
	@cp $(DRV_REL_PATH_LINUX)/*.a $(PDT_OUT_LIB_LINUX)
	@cp $(NDK_MAPI_REL_LINUX_LIB)/*.a $(PDT_OUT_LIB_LINUX)
	@cp $(HISYSLINK_IPCMSG_LIB_LINUX)/*.a $(PDT_OUT_LIB_LINUX)
	@cp $(HISYSLINK_DATAFIFO_LIB_LINUX)/*.a $(PDT_OUT_LIB_LINUX)
	@cp $(OSDRV_ROOT)/components/ipcm/ipcm/class/sharefs/linux/libsharefs.a $(PDT_OUT_LIB_LINUX)
ifeq ($(CONFIG_RELEASE), y)
	@cp $(OSDRV_LINUX_CROSS_LIB)/libstdc++.a $(PDT_OUT_LIB_LINUX)
else
	@if [ -f $(PDT_OUT_LIB_LINUX)/libstdc++.a ];then rm -f $(PDT_OUT_LIB_LINUX)/libstdc++.a;fi
endif
#	@cp $(NDK_COMPONENT_PATH)/userproc/out/*.a $(PDT_OUT_LIB_LINUX)
	@cp $(MWARE_THIRDPARTY_PATH)/tiff/tiff/_install/lib/*.a $(PDT_OUT_LIB_LINUX)
	@cp $(MWARE_REL_LINUX_LIB)/*.a $(PDT_OUT_LIB_LINUX)
	@cp $(MWARE_REL_LINUX_LIB)/libffmpeg_demuxer.so $(PDT_OUT_LIB_LINUX)
ifeq ($(CONFIG_HI3559AV100),y)
	@cp $(MWARE_REL_LINUX_LIB)/libmpf_demuxer.so $(PDT_OUT_LIB_LINUX)
endif
	@cp $(REF_OUT_COMMON_LIB)/$(OS_TYPE)/*.a $(PDT_OUT_LIB_LINUX)
	@cp $(REF_OUT_HAL_LIB)/$(OS_TYPE)/*.a $(PDT_OUT_LIB_LINUX)

	@cp $(MPP_REL_LINUX_LIB_PATH)/*.a $(PDT_OUT_LIB_LINUX)/
	@cp $(REF_OUT_HAL_LIB)/$(OS_TYPE)/*.a $(PDT_OUT_LIB_LINUX)
	@cp $(PDT_OUT_MODULES_LIB)/$(OS_TYPE)/*.a $(PDT_OUT_LIB_LINUX)

success:
	@echo ""
	@echo -e "\033[31;32m `basename $(EXE)` Was SUCCESSFUL COMPILE \033[0m"
	@echo -e "\033[31;32m *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ \033[0m"
	@echo "";echo ""

target: $(SRC_DEPS) $(OBJS)
	@echo "* VSS_LIB $(VSS_LIB)"
	@echo "* CREATE $(EXE)"
	@$(LINK) -o $(EXE) $(LINK_MAP) -Wl,--gc-sections -Wl,--start-group $(OBJS) $(VSS_LIB) -Wl,--end-group
#	@$(STRIP) $(EXE)
	cp $(EXE) ~/main_app


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
	@rm -f $(TARGET)
	@echo "--------------------------------------------"
	@echo " "
