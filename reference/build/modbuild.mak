
#++++++++++++++++++++++++++++++++++++++++++++++++++++
#	Common Path
#++++++++++++++++++++++++++++++++++++++++++++++++++++
MODULE_ROOT ?=
MODULE_NAME ?= $(shell basename $(MODULE_ROOT))
SRC_ROOT    ?= $(MODULE_ROOT)/src
SRC_DIR     ?= $(SRC_ROOT)
SRCS_OMIT   ?=

OBJ_ROOT    ?=
LIB_ROOT    ?=

# static/dynamic/all
LIB_TYPE    ?= static

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
MOD_INC_PATH := $(sort $(dir $(shell find $(MODULE_ROOT) -name '*.h')))
VSS_INC_PATH += $(foreach dir, $(MOD_INC_PATH), -I$(dir))
VSS_CFLAGS   += $(VSS_MOD_CFLAGS)

#++++++++++++++++++++++++++++++++++++++++++++++++++++
#	Create Directory
#++++++++++++++++++++++++++++++++++++++++++++++++++++
CreateResult :=
dummy := $(call CreateDir, $(OBJ_ROOT))
dummy += $(call CreateDir, $(LIB_ROOT))
dummy += $(foreach dir, $(OBJ_DIR), CreateResult += $(call CreateDir, $(dir)))
ifneq ($(strip CreateResult),)
    err = $(error $(CreateResult))
endif

#++++++++++++++++++++++++++++++++++++++++++++++++++++
#	Lib Name
#++++++++++++++++++++++++++++++++++++++++++++++++++++
LIB	   := $(LIB_ROOT)/lib$(MODULE_NAME).a
SOLIB  := $(LIB_ROOT)/lib$(MODULE_NAME).so

#++++++++++++++++++++++++++++++++++++++++++++++++++++
#	Compile Command
#++++++++++++++++++++++++++++++++++++++++++++++++++++
COMPILE    = $(CC) $(VSS_CFLAGS) -Wall -c "$<" -o "$@" $(VSS_INC_PATH)
COMPILEDEP = $(CCDEP) -MM "$<"  $(VSS_CFLAGS) $(VSS_INC_PATH)
LINK       = $(CC) $(VSS_CFLAGS) -Wall

#++++++++++++++++++++++++++++++++++++++++++++++++++++
#	Compiler
#++++++++++++++++++++++++++++++++++++++++++++++++++++
.PHONY: all $(LIB) clean
default: all

ifeq ($(strip $(LIB_TYPE)),static)
all : prepare $(SRC_DEPS) $(OBJS) $(LIB) success
else ifeq ($(strip $(LIB_TYPE)),dynamic)
all : prepare $(SRC_DEPS) $(OBJS) $(SOLIB) success
else ifeq ($(strip $(LIB_TYPE)),all)
all : prepare $(SRC_DEPS) $(OBJS) $(LIB) $(SOLIB) success
endif

prepare:
	@echo "";echo ""
	@echo -e "\033[31;32m *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ \033[0m"
	@echo -e "\033[31;32m [COMPILING] `basename $(MODULE_NAME)` ... ... \033[0m"
	@echo -e "\033[31;32m [MOD CFLAG] $(VSS_MOD_CFLAGS)\033[0m"
	@echo -e "\033[31;32m [MOD CFLAG] $(VSS_CFLAGS)\033[0m"
	@echo ""
	@if [ -f  "/home/share/publish/Jenkins/shell/extracheck.sh" ]; then /home/share/publish/Jenkins/shell/extracheck.sh ${CUR_ROOT};fi

success:
	@echo ""
ifneq ($(findstring $(strip $(LIB_TYPE)),  "static all"),)
	@echo -e "\033[31;32m `basename $(LIB)` Was SUCCESSFUL COMPILE \033[0m"
endif
ifneq ($(findstring $(strip $(LIB_TYPE)),  "dynamic all"),)
	@echo -e "\033[31;32m `basename $(SOLIB)` Was SUCCESSFUL COMPILE \033[0m"
endif
	@echo -e "\033[31;32m *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ \033[0m"
	@echo "";echo ""

$(SRC_DEPS) : $(OBJ_ROOT)/%.d : $(SRC_ROOT)/%.c
	@echo "SRC_DEPS: " $(notdir $@)
	@set -e;$(COMPILEDEP) > $@.$$$$; \
	sed 's,.*\.o[ :]*,$(@:%.d=%.o) $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

$(OBJS)     : $(OBJ_ROOT)/%.o : $(SRC_ROOT)/%.c
	@echo -e "\033[31;32m [$(CC)]  \033[0m `basename $<` \033[0m"
	@$(COMPILE)
	@echo -e "\033[31;32m       [OK] \033[0m";echo ""

$(LIB)      : $(SRC_DEPS) $(OBJS)
	@echo -e "\033[31;32m [CREATING] \033[0m `basename $(LIB)` \033[0m"
	@$(AR) $(ARFLAGS) $@ $(OBJS)
	@$(RANLIB) $@
	@echo -e "\033[31;32m       [OK] \033[0m";echo ""

$(SOLIB)    : $(SRC_DEPS) $(OBJS)
	@echo -e "\033[31;32m [CREATING] \033[0m `basename $(SOLIB)` \033[0m"
	@$(CC)  -fPIC -shared -o $@ $(OBJS)
	@echo -e "\033[31;32m       [OK] \033[0m";echo ""

clean:
	@echo " "
	@echo "--------------------------------------------"
	@echo -e "\033[31;31m [DELETING Tmp files ......] \033[0m"
	@-rm -f $(SRC_DEPS)
	@if [ -n $(OBJ_ROOT) ];then rm -rf $(OBJ_ROOT);fi
ifneq ($(findstring $(strip $(LIB_TYPE)),  "static all"),)
	@echo -e "\033[31;31m [DELETING `basename $(LIB)` ......] \033[0m"
	@rm -f $(LIB)
endif
ifneq ($(findstring $(strip $(LIB_TYPE)),  "dynamic all"),)
	@echo -e "\033[31;31m [DELETING `basename $(SOLIB)` ......] \033[0m"
	@rm -f $(SOLIB)
endif
	@echo "--------------------------------------------"
	@echo " "
