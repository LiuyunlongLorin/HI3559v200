#===============================================================================
# export variable
#===============================================================================
LOCAL_DIR := $(shell pwd)
MIDDLEWARE_DIR := $(LOCAL_DIR)/../../../../..
include $(MIDDLEWARE_DIR)/base.mak
MEDIA_ADPT_DIR := $(MIDDLEWARE_DIR)/media_adpt

#===============================================================================
# local variable
#===============================================================================
#mod dir
MOD_ROOT    := $(LOCAL_DIR)/../../..
OBJ_ROOT    := $(LOCAL_DIR)/obj
TRAGET_ROOT := $(MOD_ROOT)/lib
SRC_ROOT    := $(LOCAL_DIR)
TASK_DIR    := $(LOCAL_DIR)/task

FFMPEG_VER  := ffmpeg-y

#src files
SRCS        := $(shell find $(SRC_ROOT)  -type f -name '*.c' -print)
SRCS        := $(sort $(SRCS))

ifeq ($(CFG_OS_TYPE_LITEOS),y)
SRCS += $(LOCAL_DIR)/stdatomic.c
else
SRC := $(filter-out %/hi_liteos_utils.c,$(SRC))
endif
#obj files
OBJS_C      := $(SRCS:$(SRC_ROOT)%.c=$(OBJ_ROOT)/%.o)

OBJS        := $(OBJS_C)
OBJS        := $(sort $(OBJS))
OBJ_DIR     := $(dir $(OBJS_C))



SRC_DEPS    := $(OBJS:%.o=%.d)

#include path
#MOD_INC_PATH     := $(sort $(dir $(shell find $(MOD_ROOT) -name '*.h')))
#MOD_INC_PATH     := $(sort $(dir $(shell find $(LOCAL_DIR) -name '*.h')))
#HIBERRY_INCLUDE_PATH  += $(foreach dir, $(MOD_INC_PATH), -I$(dir))
#$(warning  $(HIBERRY_INCLUDE_PATH));
HIBERRY_INCLUDE_PATH += -I$(COMPONENT_DIR)/liteplayer/include
HIBERRY_INCLUDE_PATH += -I$(THIRDPARTY_DIR)/ffmpeg/$(FFMPEG_VER)/install/include  -I$(THIRDPARTY_DIR)/ffmpeg/$(FFMPEG_VER)/compat/atomics/pthread
HIBERRY_INCLUDE_PATH += -I$(COMMON_DIR)/include
HIBERRY_INCLUDE_PATH += -I$(MEDIA_ADPT_DIR)/include
HIBERRY_INCLUDE_PATH += -I$(COMPONENT_DIR)/liteplayer/src/demux/ffmpeg_demux/parser
HIBERRY_INCLUDE_PATH +=	-I$(COMMON_DIR)/securec/include
# create obj dir $$ target dir
CreateResult :=
dummy := $(call CreateDir, $(OBJ_ROOT))
dummy += $(call CreateDir, $(TRAGET_ROOT))
dummy += $(foreach dir, $(OBJ_DIR), CreateResult += $(call CreateDir, $(dir)))
ifneq ($(strip $(CreateResult)),)
    $(error $(CreateResult))
endif

#target
TARGET_A := $(TRAGET_ROOT)/libffmpeg_demuxer.a
TARGET_SO := $(TRAGET_ROOT)/libffmpeg_demuxer.so

#cflags
ifeq ($(CFG_OS_TYPE_LITEOS),y)
CFLAGS += -Wl,--whole-archive   -L$(THIRDPARTY_DIR)/ffmpeg/$(FFMPEG_VER)/install/lib     -lavutil -lavformat -lavcodec  -Wl,--no-whole-archive
CFLAGS += -mlong-calls -nostdlib -fPIC -Wl,--whole-archive -L$(LITEOSTOPDIR)/compat/linux/ -Wl,--no-whole-archive
else
CFLAGS += -Wl,--whole-archive   -L$(THIRDPARTY_DIR)/ffmpeg/$(FFMPEG_VER)/install/lib    -lavutil -lavformat -lavcodec  $(COMMON_DIR)/securec/lib/libsecurec.a -Wl,--no-whole-archive
CFLAGS += -lm -lpthread
endif
#===============================================================================
# rules
#===============================================================================
COMPILE    = $(CC) -fPIC $(CFLAGS) -c "$<" -o "$@" $(HIBERRY_INCLUDE_PATH)
COMPILEDEP = $(CC) -MM "$<" $(CFLAGS) $(HIBERRY_INCLUDE_PATH)
COMPILE_C  = $(CC) -fPIC $(CFLAGS) -c "$<" -o "$@" $(HIBERRY_INCLUDE_PATH)
LINK       = $(CC) $(CFLAGS)

.PHONY: all prepare clean install uninstall

all: prepare $(TARGET_A) $(TARGET_SO)
#all: prepare $(TARGET_SO)

prepare:
ifeq ($(CFG_OS_TYPE_LITEOS),y)
	@cp $(THIRDPARTY_DIR)/ffmpeg/$(FFMPEG_VER)/compat/atomics/pthread/stdatomic.c $(LOCAL_DIR)
	@echo "SRC: $(SRCS); OBJS: $(OBJS)"
endif

$(TARGET_A): $(OBJS)
	$(AT)$(AR) -rcv $@ $^

$(TARGET_SO): $(OBJS)
	$(AT)$(CC) $(CFLAGS) -o $@ $^ $(LFLAGS)
	$(STRIP) $(TARGET_SO)

$(OBJS_C): $(OBJ_ROOT)/%.o :$(SRC_ROOT)/%.c
	$(AT)$(COMPILE_C)

$(SRC_DEPS): $(OBJ_ROOT)/%.d : $(SRC_ROOT)/%.c
	$(AT)set -e;rm -f $@;$(COMPILEDEP) > $@.$$$$; \
        sed 's,.*\.o[ :]*,$(@:%.d=%.o) $@ : ,g' < $@.$$$$ > $@; \
        rm -f $@.$$$$

#-include $(SRC_DEPS)

install: all
	$(AT)mkdir -p $(OUT_LIB_DIR)
#	$(AT)mkdir -p $(OUT_HEADER_DIR)
#	$(AT)cp -af $(TARGET_A) $(OUT_LIB_DIR)/
	$(AT)cp -af $(TARGET_SO) $(OUT_LIB_DIR)/

uninstall:
#	$(AT)rm -rf $(OUT_LIB_DIR)/$(notdir $(TARGET_A))
	$(AT)rm -rf $(OUT_LIB_DIR)/$(notdir $(TARGET_SO))

clean:
	$(AT)rm -rf $(SRC_DEPS)
	$(AT)rm -rf $(OBJS)
	$(AT)rm -rf $(OBJ_ROOT)
	$(AT)rm -rf $(TARGET_A)
	$(AT)rm -rf $(TARGET_SO)
ifeq ($(CFG_OS_TYPE_LITEOS),y)
	$(AT)rm -rf $(LOCAL_DIR)/stdatomic.c
endif
