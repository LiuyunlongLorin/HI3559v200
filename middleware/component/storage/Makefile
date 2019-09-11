#===============================================================================
# export variable
#================================================================================
LOCAL_DIR := $(shell pwd)
MIDDLEWARE_DIR := $(LOCAL_DIR)/../..
include $(MIDDLEWARE_DIR)/base.mak
MEDIA_ADPT_DIR := $(MIDDLEWARE_DIR)/media_adpt

HIBERRY_COMMON_INCLUDE := $(MIDDLEWARE_DIR)/common/include
HIBERRY_COMMON_INCLUDE += $(MIDDLEWARE_DIR)/common/log/include
#===============================================================================
# STORAGE variable
#===============================================================================
STORAGE_ROOT    := $(LOCAL_DIR)
#===============================================================================
# local variable
#===============================================================================
#mod dir
MOD_ROOT    := $(LOCAL_DIR)
OBJ_ROOT    := $(MOD_ROOT)/obj
TARGET_ROOT := $(STORAGE_ROOT)/lib
HEAD_ROOT   := $(STORAGE_ROOT)/include
SRC_ROOT    := $(LOCAL_DIR)

#src files
SRCS        := $(shell find $(SRC_ROOT) -name '*.cpp' -print)
SRCS        := $(sort $(SRCS))

#obj files
OBJS        := $(SRCS:$(SRC_ROOT)%.cpp=$(OBJ_ROOT)%.o)
OBJS        := $(sort $(OBJS))
OBJ_DIR     := $(dir $(OBJS))

SRC_DEPS    := $(OBJS:%.o=%.d)

#include path
MOD_INC_PATH  := $(sort $(dir $(shell find $(SRC_ROOT) -name '*.h')))
MOD_INC_PATH  += $(HIBERRY_COMMON_INCLUDE)
MOD_INC_PATH  += $(MEDIA_ADPT_DIR)/include
MOD_INC_PATH  += $(COMMON_DIR)/securec/include
HIBERRY_INCLUDE_PATH  := $(foreach dir, $(MOD_INC_PATH), -I$(dir))

# create obj dir $$ target dir
CreateResult :=
dummy := $(call CreateDir, $(OBJ_ROOT))
dummy += $(call CreateDir, $(TARGET_ROOT))
dummy += $(foreach dir, $(OBJ_DIR), CreateResult += $(call CreateDir, $(dir)))
ifneq ($(strip $(CreateResult)),)
    $(error $(CreateResult))
endif

#target
TARGET_A := $(TARGET_ROOT)/libstorage.a
TARGET_SO := $(TARGET_ROOT)/libstorage.so


#===============================================================================
# rules
#===============================================================================

COMPILE    = $(CXX) $(CFLAGS) -c "$<" -o "$@" $(HIBERRY_INCLUDE_PATH)
COMPILEDEP = $(CXX) -MM "$<" $(CFLAGS) $(HIBERRY_INCLUDE_PATH)
LINK       = $(CXX) $(CFLAGS)

.PHONY: all clean install uninstall

all: $(TARGET_A) $(TARGET_SO)

$(TARGET_A): $(OBJS)
	$(AT)$(AR) -rcv $@ $^
	@echo "***************************************************"
	@echo "  *SUCCESSFUL COMPILE $(TARGET_A)"
	@echo "***************************************************"

$(TARGET_SO): $(OBJS)
	$(AT)$(CXX) $(CFLAGS) -fPIC -shared -o $@ $^

$(OBJS): $(OBJ_ROOT)/%.o :$(SRC_ROOT)/%.cpp
	@echo "---------$(COMPILE)----------"
	$(AT)$(COMPILE)

$(SRC_DEPS): $(OBJ_ROOT)/%.d : $(SRC_ROOT)/%.cpp
	$(AT)set -e;rm -f $@;$(COMPILEDEP) > $@.$$$$; \
        sed 's,.*\.o[ :]*,$(@:%.d=%.o) $@ : ,g' < $@.$$$$ > $@; \
        rm -f $@.$$$$

-include $(SRC_DEPS)

install: all
	$(AT)mkdir -p $(OUT_LIB_DIR)
	$(AT)mkdir -p $(OUT_HEADER_DIR)
	$(AT)cp -af $(TARGET_A) $(OUT_LIB_DIR)
	$(AT)cp -af $(TARGET_SO) $(OUT_LIB_DIR)
	$(AT)cp -af $(SRC_ROOT)/include/*.h $(OUT_HEADER_DIR)

uninstall:
	$(AT)rm -rf $(OUT_LIB_DIR)/$(notdir $(TARGET_A))
	$(AT)rm -rf $(OUT_LIB_DIR)/$(notdir $(TARGET_SO))

clean:
	@echo "DEL lib and obj dir.."
	$(AT)rm -rf $(OBJ_ROOT)
	$(AT)rm -rf $(TARGET_ROOT)
	@echo "DEL end!"
