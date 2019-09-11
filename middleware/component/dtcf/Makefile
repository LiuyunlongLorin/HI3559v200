#===============================================================================
# export variable
#===============================================================================
LOCAL_DIR := $(shell pwd)
MIDDLEWARE_DIR := $(LOCAL_DIR)/../..
include $(MIDDLEWARE_DIR)/base.mak

MEDIA_ADPT_DIR := $(MIDDLEWARE_DIR)/media_adpt
HIPROC_INC_PATH := $(COMMON_DIR)/uproc/include
HIBERRY_COMMON_INCLUDE := $(MIDDLEWARE_DIR)/common/include
#===============================================================================
# local variable
#===============================================================================
#mod dir
MOD_ROOT    := $(LOCAL_DIR)
OBJ_ROOT    := $(MOD_ROOT)/obj
TRAGET_ROOT := $(MOD_ROOT)/lib
SRC_ROOT    := $(LOCAL_DIR)

#src files
SRCS        := $(shell find $(SRC_ROOT) -name '*.c' -print)
SRCS        := $(sort $(SRCS))

#obj files
OBJS        := $(SRCS:$(SRC_ROOT)%.c=$(OBJ_ROOT)%.o)
OBJS        := $(sort $(OBJS))
OBJ_DIR     := $(dir $(OBJS))

SRC_DEPS    := $(OBJS:%.o=%.d)

#include path
MOD_INC_PATH     := $(sort $(dir $(shell find $(MOD_ROOT) -name '*.h')))
MOD_INC_PATH	 += $(OUT_HEADER_DIR)
MOD_INC_PATH	 += $(HIBERRY_COMMON_INCLUDE)
MOD_INC_PATH     += $(HIBERRY_DEP_MOD_INCLUDE)
MOD_INC_PATH     += $(MEDIA_ADPT_DIR)/include
MOD_INC_PATH     += $(COMMON_DIR)/securec/include
HIBERRY_INCLUDE_PATH  := $(foreach dir, $(MOD_INC_PATH), -I$(dir))

# create obj dir $$ target dir
CreateResult :=
dummy := $(call CreateDir, $(OBJ_ROOT))
dummy += $(call CreateDir, $(TRAGET_ROOT))
dummy += $(foreach dir, $(OBJ_DIR), CreateResult += $(call CreateDir, $(dir)))
ifneq ($(strip $(CreateResult)),)
    $(error $(CreateResult))
endif

#target
TARGET_A := $(TRAGET_ROOT)/libdtcf.a
TARGET_SO := $(TRAGET_ROOT)/libdtcf.so

#cflags
CFLAGS +=
#default ioctl cmd,fix compile.
IOCTL_CMD_READDIR_ALL = VFAT_IOCTL_READDIR_ALL_0X14
ifeq ($(CFG_CHIP_TYPE),hi3519v101)
IOCTL_CMD_READDIR_ALL = VFAT_IOCTL_READDIR_ALL_0X14
endif
ifeq ($(CFG_CHIP_TYPE),hi3559)
IOCTL_CMD_READDIR_ALL = VFAT_IOCTL_READDIR_ALL_0X14
endif
ifeq ($(CFG_CHIP_TYPE),hi3556)
IOCTL_CMD_READDIR_ALL = VFAT_IOCTL_READDIR_ALL_0X14
endif
$(warning ~~~~~~~~~~~~~~~~~~~~ $(CFG_CHIP_TYPE_HI3559AES))
ifeq ($(CFG_CHIP_TYPE),hi3559aes)
IOCTL_CMD_READDIR_ALL = VFAT_IOCTL_READDIR_ALL_0X13 #TODO
endif
ifeq ($(CFG_CHIP_TYPE),hi3559av100)
IOCTL_CMD_READDIR_ALL = VFAT_IOCTL_READDIR_ALL_0X13 #TODO
endif
CFLAGS += -DIOCTL_CMD_READDIR_ALL=$(IOCTL_CMD_READDIR_ALL)
#===============================================================================
# rules
#===============================================================================

COMPILE    = $(CC) $(CFLAGS) -c "$<" -o "$@" $(HIBERRY_INCLUDE_PATH)
COMPILEDEP = $(CC) -MM "$<" $(CFLAGS) $(HIBERRY_INCLUDE_PATH)
LINK       = $(CC) $(CFLAGS)

.PHONY: all clean install uninstall

all: $(TARGET_A) $(TARGET_SO)

$(TARGET_A): $(OBJS)
	$(AT)$(AR) -rcv $@ $^
	@echo "***************************************************"
	@echo "  *SUCCESSFUL COMPILE $(TARGET_A)"
	@echo "***************************************************"

$(TARGET_SO): $(OBJS)
	$(AT)$(CC) $(CFLAGS)  -fPIC -shared -o $@ $^

$(OBJS): $(OBJ_ROOT)/%.o :$(SRC_ROOT)/%.c
	@echo "---------$(COMPILE)----------"
	$(AT)$(COMPILE)

$(SRC_DEPS): $(OBJ_ROOT)/%.d : $(SRC_ROOT)/%.c
	$(AT)set -e;rm -f $@;$(COMPILEDEP) > $@.$$$$; \
        sed 's,.*\.o[ :]*,$(@:%.d=%.o) $@ : ,g' < $@.$$$$ > $@; \
        rm -f $@.$$$$

-include $(SRC_DEPS)

install: all
	$(AT)mkdir -p $(OUT_LIB_DIR)
	$(AT)mkdir -p $(OUT_HEADER_DIR)
	$(AT)cp -af $(TARGET_A) $(OUT_LIB_DIR)
	$(AT)cp -af $(TARGET_SO) $(OUT_LIB_DIR)
	$(AT)cp -af $(LOCAL_DIR)/include/*.h $(OUT_HEADER_DIR)

uninstall:
	$(AT)rm -rf $(OUT_LIB_DIR)/$(notdir $(TARGET_A))
	$(AT)rm -rf $(OUT_LIB_DIR)/$(notdir $(TARGET_SO))


clean:
	@echo "DEL lib and obj dir.."
	$(AT)rm -rf $(OBJ_ROOT)
	$(AT)rm -rf $(TRAGET_ROOT)
	@echo "DEL end!"
