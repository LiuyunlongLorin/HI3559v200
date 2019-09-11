# Base Define
CUR_ROOT := $(shell pwd)
REF_ROOT ?= $(CUR_ROOT)/../..
include $(REF_ROOT)/build/base.mak

#++++++++++++++++++++++++++++++++++++++++++++++++++++
#	Source FileList
#++++++++++++++++++++++++++++++++++++++++++++++++++++
SRC_ROOT  := $(shell cd $(REF_ROOT);pwd)
SRC_DIR   := $(CUR_ROOT)/src

SRCS      := $(shell find $(SRC_DIR) -name '*.c')
SRCS      += $(REF_APPCOMMON_PATH)/utils/hilog/src/hi_appcomm_log.c
SRCS      += $(REF_APPCOMMON_PATH)/upgrade/src/crc32.c
SRCS_OMIT :=
ifneq ($(strip $(SRCS_OMIT)),)
SRCS      := $(filter-out $(SRCS_OMIT),$(SRCS))
endif
SRCS      := $(sort $(SRCS))

#++++++++++++++++++++++++++++++++++++++++++++++++++++
#	Object FileList
#++++++++++++++++++++++++++++++++++++++++++++++++++++
CROSS  =
OBJ_ROOT  := $(REF_OUT_TOOL_OBJ)/$(CUR_DIR_NAME)
OBJS      := $(SRCS:$(SRC_ROOT)%.c=$(OBJ_ROOT)%.o)
OBJS      := $(sort $(OBJS))
OBJ_DIR   := $(sort $(dir $(OBJS)))
SRC_DEPS  := $(OBJS:%.o=%.d)

#++++++++++++++++++++++++++++++++++++++++++++++++++++
#	Module Specified Configure
#++++++++++++++++++++++++++++++++++++++++++++++++++++
VSS_MOD_CFLAGS := -DCFG_DEBUG_LOG_ON
VSS_CFLAGS     := $(VSS_MOD_CFLAGS) -Wall -Werror
VSS_STLIBS     += -lz

MOD_INC_PATH   := $(REF_APPCOMMON_PATH)/upgrade/include
MOD_INC_PATH   += $(CUR_ROOT)/include/
VSS_INC_PATH   += $(foreach dir, $(MOD_INC_PATH), -I$(dir))

#++++++++++++++++++++++++++++++++++++++++++++++++++++
#	Bin Configure
#++++++++++++++++++++++++++++++++++++++++++++++++++++
BIN_ROOT  := $(REF_OUT_TOOL_BIN)/$(CUR_DIR_NAME)
EXE       := $(BIN_ROOT)/upgrade_generate

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
