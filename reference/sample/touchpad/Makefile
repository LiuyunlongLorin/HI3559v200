# Base Define
OS_TYPE     := Linux
CUR_ROOT    := $(shell pwd)
REF_ROOT    := $(CUR_ROOT)/../../
include $(REF_ROOT)/build/base.mak

#++++++++++++++++++++++++++++++++++++++++++++++++++++
#	Compile Path
#++++++++++++++++++++++++++++++++++++++++++++++++++++
SRC_ROOT    := $(CUR_ROOT)
SRC_DIR     := $(SRC_ROOT)
OBJ_ROOT    := $(PDT_OUT_MODULES_OBJ)/$(CUR_PARENT_DIR_NAME)/$(OS_TYPE)
BIN_ROOT    := $(PDT_OUT_BIN)/$(CUR_DIR_NAME)

#++++++++++++++++++++++++++++++++++++++++++++++++++++
#	Source FileList
#++++++++++++++++++++++++++++++++++++++++++++++++++++
SRC_ROOT  := $(shell cd $(REF_ROOT);pwd)
SRC_DIR   := $(CUR_ROOT)

SRCS      := $(shell find $(SRC_DIR) -name '*.c')
SRCS      := $(sort $(SRCS))
ifneq ($(strip $(SRCS_OMIT)),)
SRCS      := $(filter-out $(SRCS_OMIT),$(SRCS))
endif

#++++++++++++++++++++++++++++++++++++++++++++++++++++
#	Object FileList
#++++++++++++++++++++++++++++++++++++++++++++++++++++
ifeq ($(RUN_ENV),pc)
CROSS  =
OBJ_ROOT  := $(REF_OUT_TEST_OBJ)/$(CUR_DIR_NAME)_pc
else
OBJ_ROOT  := $(REF_OUT_TEST_OBJ)/$(CUR_DIR_NAME)_board
endif

OBJS      := $(SRCS:$(SRC_ROOT)%.c=$(OBJ_ROOT)%.o)
OBJS      := $(sort $(OBJS))
OBJ_DIR   := $(sort $(dir $(OBJS)))
SRC_DEPS  := $(OBJS:%.o=%.d)

#++++++++++++++++++++++++++++++++++++++++++++++++++++
#	Module Specified Configure
#++++++++++++++++++++++++++++++++++++++++++++++++++++
VSS_MOD_CFLAGS := -DPDT_PARAM_MEDIAMODE_CNT=$(MEDIAMODE_CNT) -DCFG_DEBUG_LOG_ON  -DTWO_SENSOR_CONNECT
VSS_CFLAGS     := $(VSS_MOD_CFLAGS) -Wall -Werror -Wno-error

VSS_INC_PATH   += $(foreach dir, $(MOD_INC_PATH), -I$(dir))
#++++++++++++++++++++++++++++++++++++++++++++++++++++
#	Bin Configure
#++++++++++++++++++++++++++++++++++++++++++++++++++++
LIB_ROOT  := $(REF_OUT_TEST_LIB)/$(OS_TYPE)
BIN_ROOT  := $(REF_OUT_TEST_BIN)/$(CUR_DIR_NAME)
ifeq ($(RUN_ENV),pc)
CROSS     :=
EXE       := $(BIN_ROOT)/$(CUR_DIR_NAME)_pc
else
EXE       := $(CUR_ROOT)/$(CUR_DIR_NAME)_board
endif

#++++++++++++++++++++++++++++++++++++++++++++++++++++
#	Create Directory
#++++++++++++++++++++++++++++++++++++++++++++++++++++
CreateResult :=
dummy := $(call CreateDir, $(OBJ_ROOT))
dummy += $(call CreateDir, $(LIB_ROOT))
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

VSS_LIB     =  -L$(BIN_ROOT) -L$(PDT_OUT_MODULES_LIB)/Linux
VSS_LIB     =  -L$(REF_OUT_HAL_LIB)/$(OS_TYPE)/  
VSS_LIB     +=  -L$(REF_OUT_COMMON_LIB)/$(OS_TYPE)/ -lutils -lhilog

#++++++++++++++++++++++++++++++++++++++++++++++++++++
#	Link Libs
#++++++++++++++++++++++++++++++++++++++++++++++++++++

VSS_LIB    += -lpthread -lm -lrt -lstdc++ -ldl

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
	@#cp $(OSDRV_PUB_LINUX_LIB)/*.a $(LIB_ROOT)
	@cp $(HISYSLINK_IPCMSG_LIB_LINUX)/*.a $(LIB_ROOT)
	@cp $(HISYSLINK_DATAFIFO_LIB_LINUX)/*.a $(LIB_ROOT)
	@cp $(NDK_MAPI_REL_LINUX_LIB)/*.a $(LIB_ROOT)
	#@cp $(NDK_ROOT)/sdk/mpp/out/linux/big-little/lib/libhive_RES.so $(LIB_ROOT)/
	#@cp $(MPP_REL_LINUX_LIB_PATH)/*.a $(LIB_ROOT)/
	@cp $(REF_OUT_COMMON_LIB)/$(OS_TYPE)/*.a $(LIB_ROOT)
	@cp $(REF_OUT_HAL_LIB)/$(OS_TYPE)/*.a $(LIB_ROOT)
	#@cp $(PDT_OUT_MODULES_LIB)/$(OS_TYPE)/*.a $(LIB_ROOT)

success: $(EXE)
	@echo ""
	@echo -e "\033[31;32m `basename $(EXE)` Was SUCCESSFUL COMPILE \033[0m"
	@echo -e "\033[31;32m *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ \033[0m"
	@echo "";echo ""
	@cp $(EXE) ~
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
	$(LINK) -o $@ -Wl,--start-group $(OBJS) $(VSS_LIB) $(VSS_LIBS_PATH) -Wl,--end-group
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

