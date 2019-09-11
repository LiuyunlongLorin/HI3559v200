# Base Define
OS_TYPE  := Linux
CUR_ROOT := $(shell pwd)
PDT_ROOT ?= $(CUR_ROOT)/../../..
include $(PDT_ROOT)/build/base.mak

#++++++++++++++++++++++++++++++++++++++++++++++++++++
#	Source FileList
#++++++++++++++++++++++++++++++++++++++++++++++++++++
SRC_ROOT  := $(shell cd $(REF_ROOT);pwd)
SRC_DIR   := $(CUR_ROOT)/src
SRCS      := $(shell find $(SRC_DIR) -name '*.c')
SRCS      := $(sort $(SRCS))

#++++++++++++++++++++++++++++++++++++++++++++++++++++
#	Object FileList
#++++++++++++++++++++++++++++++++++++++++++++++++++++
OBJ_ROOT  := $(PDT_OUT_MODULES_OBJ)/$(CUR_DIR_NAME)
OBJS      := $(SRCS:$(SRC_ROOT)%.c=$(OBJ_ROOT)%.o)
OBJS      := $(sort $(OBJS))
OBJ_DIR   := $(sort $(dir $(OBJS)))
SRC_DEPS  := $(OBJS:%.o=%.d)

#++++++++++++++++++++++++++++++++++++++++++++++++++++
#	Module Specified Configure
#++++++++++++++++++++++++++++++++++++++++++++++++++++
MEDIAMODE_CNT   = $(shell grep -nr "config_product_mediamode_*" $(PDT_INICFG_PATH)/config_cfgaccess_entry.ini |grep -v comm |wc -l)
VSS_MOD_CFLAGS := -DPDT_PARAM_MEDIAMODE_CNT=$(MEDIAMODE_CNT) -DCFG_DEBUG_LOG_ON
VSS_CFLAGS     += $(VSS_MOD_CFLAGS) -Wall -Werror
MOD_INC_PATH   := $(PDT_MODULE_PATH)/param/core/common
VSS_INC_PATH   += $(foreach dir, $(MOD_INC_PATH), -I$(dir))

#++++++++++++++++++++++++++++++++++++++++++++++++++++
#	Bin Configure
#++++++++++++++++++++++++++++++++++++++++++++++++++++
LIB_ROOT  := $(REF_OUT_TEST_LIB)/$(OS_TYPE)
BIN_ROOT  := $(PDT_OUT_MODULES_BIN)
EXE       := $(BIN_ROOT)/bin2flash

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
#	Link Libs
#++++++++++++++++++++++++++++++++++++++++++++++++++++
VSS_LIB    := -L$(PDT_OUT_LIB_LINUX)
VSS_LIB    += -lfsm -leventhub -lmessagehandler -lz -lhitimer -ltimers -lmwlog -luproc -luserproc -lsecurec
VSS_LIB    += -lparam_host -lutils -ltimestamp -lhicycle -lhiflash -lhilog -ltimestamp
VSS_LIB    += -lpthread -lm -lrt -lstdc++ -ldl

#++++++++++++++++++++++++++++++++++++++++++++++++++++
#	Compiler
#++++++++++++++++++++++++++++++++++++++++++++++++++++
.PHONY: all clean distclean
ifneq ($(AMP_TYPE), liteos)
all : prepare $(EXE) success
else
all :
	@echo -e "\033[31;32m liteos system need not to compile `basename $(EXE)` ... \033[0m"
endif

prepare:
	@echo "";echo ""
	@echo -e "\033[31;32m *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ \033[0m"
	@echo -e "\033[31;32m [COMPILING] `basename $(EXE)` ... ... \033[0m"
	@echo -e "\033[31;32m [MOD CFLAG] $(VSS_MOD_CFLAGS)\033[0m"
	@echo ""
	@cp $(DRV_REL_PATH_LINUX)/*.a $(PDT_OUT_LIB_LINUX)
	@cp $(MWARE_REL_LINUX_LIB)/*.a $(PDT_OUT_LIB_LINUX)
	@cp $(REF_OUT_COMMON_LIB)/$(OS_TYPE)/*.a $(PDT_OUT_LIB_LINUX)
	@cp $(REF_OUT_HAL_LIB)/$(OS_TYPE)/*.a $(PDT_OUT_LIB_LINUX)
	@cp $(PDT_OUT_MODULES_LIB)/$(OS_TYPE)/*.a $(PDT_OUT_LIB_LINUX)

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
	@$(LINK) -o $@ -Wl,--start-group $(OBJS) $(VSS_LIB) -Wl,--end-group
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
