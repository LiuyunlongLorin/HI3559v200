# Base Define
CUR_ROOT := $(shell pwd)
PDT_ROOT ?= $(CUR_ROOT)/../../..
ARCH64=y
include $(PDT_ROOT)/build/base.mak

#++++++++++++++++++++++++++++++++++++++++++++++++++++
#	Source FileList
#++++++++++++++++++++++++++++++++++++++++++++++++++++
SRC_ROOT  := $(shell cd $(REF_ROOT);pwd)
SRC_DIR   := $(CUR_ROOT)/src
SRC_DIR   += $(REF_APPCOMMON_PATH)/hiconfaccess
SRC_DIR	  += $(REF_THIRDPARTY_PATH)/iniparser

SRCS      := $(shell find $(SRC_DIR) -name '*.c')
SRCS      += $(REF_APPCOMMON_PATH)/utils/hilog/src/hi_appcomm_log.c
SRCS      := $(sort $(SRCS))

#++++++++++++++++++++++++++++++++++++++++++++++++++++
#	Object FileList
#++++++++++++++++++++++++++++++++++++++++++++++++++++
ifeq ($(RUN_ENV),pc)
CROSS  =
OBJ_ROOT  := $(PDT_OUT_MODULES_OBJ)/$(CUR_DIR_NAME)_pc
else
OBJ_ROOT  := $(PDT_OUT_MODULES_OBJ)/$(CUR_DIR_NAME)_board
endif
OBJS      := $(SRCS:$(SRC_ROOT)%.c=$(OBJ_ROOT)%.o)
OBJS      := $(sort $(OBJS))
OBJ_DIR   := $(sort $(dir $(OBJS)))
SRC_DEPS  := $(OBJS:%.o=%.d)

#++++++++++++++++++++++++++++++++++++++++++++++++++++
#	Module Specified Configure
#++++++++++++++++++++++++++++++++++++++++++++++++++++
MEDIAMODE_CNT   = $(shell grep -nr "config_product_mediamode_*" $(PDT_INICFG_PATH)/config_cfgaccess_entry.ini |grep -v comm |wc -l)
VSS_MOD_CFLAGS := -DPDT_PARAM_MEDIAMODE_CNT=$(MEDIAMODE_CNT) -DCFG_DEBUG_LOG_ON
ifeq ($(RUN_ENV),pc)
VSS_MOD_CFLAGS += -DINIPARAM_INI2BIN_PC
endif
VSS_CFLAGS     := $(VSS_MOD_CFLAGS) -Wall -Werror
ifeq ($(CONFIG_DCF), y)
VSS_CFLAGS     += -DCONFIG_FILEMNG_DCF
else ifeq ($(CONFIG_DTCF), y)
VSS_CFLAGS     += -DCONFIG_FILEMNG_DTCF
endif
VSS_CFLAGS     += $(KCONFIG_CFLAGS)

MOD_INC_PATH   := $(PDT_MODULE_PATH)/param/core/common
MOD_INC_PATH   += $(CUR_ROOT)/include/
VSS_INC_PATH   += $(foreach dir, $(MOD_INC_PATH), -I$(dir))

#++++++++++++++++++++++++++++++++++++++++++++++++++++
#	Bin Configure
#++++++++++++++++++++++++++++++++++++++++++++++++++++
BIN_ROOT  := $(PDT_OUT_MODULES_BIN)
ifeq ($(RUN_ENV),pc)
CROSS     :=
EXE       := $(BIN_ROOT)/ini2bin_pc
else
EXE       := $(BIN_ROOT)/ini2bin_board
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
#	Compile Command
#++++++++++++++++++++++++++++++++++++++++++++++++++++
COMPILE    = $(CROSS)gcc $(VSS_CFLAGS) -Wall -c "$<" -o "$@" $(VSS_INC_PATH)
COMPILEDEP = $(CROSS)gcc -MM "$<"  $(VSS_CFLAGS) $(VSS_INC_PATH)
LINK       = $(CROSS)gcc $(VSS_CFLAGS) -Wall

#++++++++++++++++++++++++++++++++++++++++++++++++++++
#	Compiler
#++++++++++++++++++++++++++++++++++++++++++++++++++++
.PHONY: all clean distclean
all : prepare $(EXE) success

prepare:
	@echo "";echo ""
	@echo -e "\033[31;32m *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ \033[0m"
	@echo -e "\033[31;32m [COMPILING] `basename $(EXE)` ... ... \033[0m"
	@echo -e "\033[31;32m [MOD CFLAG] $(VSS_MOD_CFLAGS)\033[0m"
	@echo -e "\033[31;32m [VSS CFLAG] $(VSS_CFLAGS)\033[0m"
	@echo ""

success: $(EXE)
	@echo ""
	@echo -e "\033[31;32m `basename $(EXE)` Was SUCCESSFUL COMPILE \033[0m"
	@echo -e "\033[31;32m *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ \033[0m"
	@echo "";echo ""

$(SRC_DEPS) : $(OBJ_ROOT)/%.d : $(SRC_ROOT)/%.c
	@set -e;$(COMPILEDEP) > $@.$$$$; \
	sed 's,.*\.o[ :]*,$(@:%.d=%.o) $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

$(OBJS)     : $(OBJ_ROOT)/%.o : $(SRC_ROOT)/%.c
	@echo -e "\033[31;32m [$(CC)]  \033[0m `basename $<` \033[0m"
	@$(COMPILE)
	@echo -e "\033[31;32m       [OK] \033[0m";echo ""

$(EXE)      : $(SRC_DEPS) $(OBJS)
	@echo -e "\033[31;32m [CREATING] \033[0m `basename $(EXE)` \033[0m"
	@$(LINK) -o $@ $(OBJS) $(VSS_STLIBS) $(VSS_SHLIBS) $(VSS_LIBS_PATH)
	@echo -e "\033[31;32m       [OK] \033[0m";echo ""

clean:
	@echo " "
	@echo "--------------------------------------------"
	@echo -e "\033[31;31m [DELETING Tmp files ......] \033[0m"
	@if [ -n $(OBJ_ROOT) ];then rm -rf $(OBJ_ROOT);fi
	@echo -e "\033[31;31m [DELETING `basename $(EXE)` ......] \033[0m"
	@rm -f $(EXE)
	@echo "--------------------------------------------"
	@echo " "

distclean: clean
