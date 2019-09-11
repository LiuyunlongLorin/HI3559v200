#===============================================================================
# export variable
#===============================================================================
LOCAL_DIR := $(shell pwd)
MIDDLEWARE_DIR := $(LOCAL_DIR)/../..
HIOSCSERVER_DIR := $(LOCAL_DIR)
include $(MIDDLEWARE_DIR)/base.mak

COMMON_DIR := $(MIDDLEWARE_DIR)/common
COM_INC_PATH     := $(COMMON_DIR)/include
MEDIA_ADPT_DIR := $(MIDDLEWARE_DIR)/media_adpt
HIMBUF_INC_PATH := $(COMMON_DIR)/mbuffer/include
SERVER_RAPIDJSON_INC_PATH := $(MIDDLEWARE_DIR)/thirdparty/rapidjson/rapidjson/include/rapidjson
SECUREC_INC_PATH := $(COMMON_DIR)/securec/include
#===============================================================================
# local variable
#===============================================================================
#mod dir
MOD_ROOT    := $(LOCAL_DIR)
OBJ_ROOT    := $(MOD_ROOT)/obj
TRAGET_ROOT := $(MOD_ROOT)/lib
SRC_ROOT    := $(LOCAL_DIR)/src
INC_INNER   := $(LOCAL_DIR)/include

#src files
SRCS        := $(shell find $(SRC_ROOT) -path  -prune -o -type f -name '*.c' -print)
SRCS        := $(sort $(SRCS))

#C obj files
OBJS        := $(SRCS:$(SRC_ROOT)%.c=$(OBJ_ROOT)%.o)
OBJS        := $(sort $(OBJS))
OBJ_DIR     := $(dir $(OBJS))

SRC_DEPS    := $(OBJS:%.o=%.d)

#src files
CXX_SRCS        := $(shell find $(SRC_ROOT) -path  -prune -o -type f -name '*.cpp' -print)
CXX_SRCS        := $(sort $(CXX_SRCS))

#CPP obj files
CXX_OBJS        := $(CXX_SRCS:$(SRC_ROOT)%.cpp=$(OBJ_ROOT)%.o)
CXX_OBJS        := $(sort $(CXX_OBJS))
#CXX_OBJS        := $(dir $(CXX_OBJS))

CXX_SRC_DEPS    := $(CXX_OBJS:%.o=%.d)

#include path
MOD_INC_PATH     := $(sort $(dir $(shell find $(MOD_ROOT) -name '*.h')))
MOD_INC_PATH     += -I$(INC_INNER)/
HIBERRY_INCLUDE_PATH  := $(foreach dir, $(MOD_INC_PATH), -I$(dir))
HIBERRY_INCLUDE_PATH  +=-I$(COM_INC_PATH)/
HIBERRY_INCLUDE_PATH  +=-I$(HIUTIL_INC_PATH)/
HIBERRY_INCLUDE_PATH  +=-I$(HIMBUF_INC_PATH)/
HIBERRY_INCLUDE_PATH  +=-I$(SERVER_RAPIDJSON_INC_PATH)/
HIBERRY_INCLUDE_PATH  +=-I$(SECUREC_INC_PATH)/
HIBERRY_INCLUDE_PATH  +=-I$(MEDIA_ADPT_DIR)/include

# create obj dir $$ target dir
CreateResult :=
dummy := $(call CreateDir, $(OBJ_ROOT))
dummy += $(call CreateDir, $(TRAGET_ROOT))
dummy += $(foreach dir, $(OBJ_DIR), CreateResult += $(call CreateDir, $(dir)))
ifneq ($(strip $(CreateResult)),)
    $(error $(CreateResult))
endif

#target
TARGET_A := $(TRAGET_ROOT)/liboscserver.a
TARGET_SO := $(TRAGET_ROOT)/liboscserver.so

#===============================================================================
# rules
#===============================================================================
CFLAGS += -O0
CXXFLAGS += -O0

COMPILE    = $(CC) $(CFLAGS) -c "$<" -o "$@" $(HIBERRY_INCLUDE_PATH)
COMPILEDEP = $(CC) -MM "$<" $(CFLAGS) $(HIBERRY_INCLUDE_PATH)
LINK       = $(CC) $(CFLAGS)

CXX_COMPILE    = $(CXX) $(CXXFLAGS) -c "$<" -o "$@" $(HIBERRY_INCLUDE_PATH)
CXX_COMPILEDEP = $(CXX) -MM "$<" $(CXXFLAGS) $(HIBERRY_INCLUDE_PATH)
CXX_LINK       = $(CXX) $(CXXFLAGS)

.PHONY: all clean install uninstall

all: $(TARGET_A) $(TARGET_SO)

$(TARGET_A): $(OBJS) $(CXX_OBJS)
	$(AT)$(AR) -rcv $@ $^

$(TARGET_SO): $(OBJS) $(CXX_OBJS)
	$(AT)$(CC) -shared -o $@ $^

$(OBJS): $(OBJ_ROOT)/%.o :$(SRC_ROOT)/%.c
	$(AT)$(COMPILE)

$(CXX_OBJS): $(OBJ_ROOT)/%.o :$(SRC_ROOT)/%.cpp
	$(AT)$(CXX_COMPILE)

$(SRC_DEPS): $(OBJ_ROOT)/%.d : $(SRC_ROOT)/%.c
	$(AT)set -e;rm -f $@;$(COMPILEDEP) > $@.$$$$; \
        sed 's,.*\.o[ :]*,$(@:%.d=%.o) $@ : ,g' < $@.$$$$ > $@; \
        rm -f $@.$$$$

$(CXX_SRC_DEPS): $(OBJ_ROOT)/%.d : $(SRC_ROOT)/%.cpp
	$(AT)set -e;rm -f $@;$(CXX_COMPILEDEP) > $@.$$$$; \
        sed 's,.*\.o[ :]*,$(@:%.d=%.o) $@ : ,g' < $@.$$$$ > $@; \
        rm -f $@.$$$$

-include $(SRC_DEPS) $(CXX_SRC_DEPS)

install: all
	$(AT)mkdir -p $(OUT_LIB_DIR)
	$(AT)mkdir -p $(OUT_HEADER_DIR)
	$(AT)cp -af $(TARGET_A) $(OUT_LIB_DIR)
	$(AT)cp -af $(TARGET_SO) $(OUT_LIB_DIR)
	$(AT)cp -af $(LOCAL_DIR)/include/*.h $(OUT_HEADER_DIR)

uninstall:
	$(AT)rm -rf $(OUT_LIB_DIR)/$(notdir $(TARGET_A))
	$(AT)rm -rf $(OUT_LIB_DIR)/$(notdir $(TARGET_SO))
	#$(AT)rm -rf $(OUT_HEADER_DIR)/outer.h
	#$(AT)rm -rf $(OUT_HEADER_DIR)/outer2.h

clean:
	$(AT)rm -rf $(SRC_DEPS)
	$(AT)rm -rf $(OBJS)
	$(AT)rm -rf $(OBJ_ROOT)
	$(AT)rm -rf $(TARGET_A)
	$(AT)rm -rf $(TARGET_SO)

info:
	@echo $(HIBERRY_INCLUDE_PATH)
