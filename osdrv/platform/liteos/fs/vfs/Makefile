include $(LITEOSTOPDIR)/config.mk

MODULE_NAME := $(notdir $(shell pwd))

LOCAL_SRCS := $(wildcard operation/*.c) $(wildcard inode/*.c) $(wildcard dirent/*.c) \
    $(wildcard mount/*.c) $(wildcard driver/*.c) $(wildcard vfs_cmd/*.c)

LOCAL_INCLUDE := \
	-I $(LITEOSTOPDIR)/fs/vfs/include\
	-I $(LITEOSTOPDIR)/fs/vfs/include/multi_partition

ifeq ($(LOSCFG_FS_YAFFS), y)
LOCAL_INCLUDE += -I $(LITEOSTOPDIR)/fs/yaffs2/yaffs/include
endif

ifeq ($(LOSCFG_FS_FAT), y)
LOCAL_INCLUDE += -I $(LITEOSTOPDIR)/fs/fat/include
endif

LOCAL_FLAGS := $(LOCAL_INCLUDE) $(LITEOS_GCOV_OPTS)

include $(MODULE)
