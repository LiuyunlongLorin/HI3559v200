ifeq ($(REF_ROOT_BASECFG),)
REF_ROOT_BASECFG = 1

ifeq ($(REF_ROOT),)
$(error please specify reference root path)
endif

#++++++++++++++++++++++++++++++++++++++++++++++++++++
#	Common Function Definition
#++++++++++++++++++++++++++++++++++++++++++++++++++++
# Current Directory Name(exclude prefix)
CUR_DIR_NAME = $(shell pwd |sed 's/^\(.*\)[/]//' )

# Current Parent Directory Name(exclude prefix)
CUR_PARENT_DIR_NAME = $(shell cd ./..;pwd|sed 's/^\(.*\)[/]//')

# Make Result Check
MAKE_EXT_FLAG := || exit "$$?"

SVR_LONGBIT   := $(shell getconf LONG_BIT)

#++++++++++++++++++++++++++++++++++++++++++++++++++++
#	Common Configure
#++++++++++++++++++++++++++++++++++++++++++++++++++++
REF_ROOT     := $(shell cd $(REF_ROOT); pwd)
SDK_ROOT     ?= $(shell cd $(REF_ROOT)/..; pwd)
include $(SDK_ROOT)/build/base.mak

#++++++++++++++++++++++++++++++++++++++++++++++++++++
#	Out Path Configure
#++++++++++++++++++++++++++++++++++++++++++++++++++++
REF_OUT_ROOT       := $(REF_ROOT)/out
REF_OUT_COMMON     := $(REF_OUT_ROOT)/common
REF_OUT_COMMON_OBJ := $(REF_OUT_COMMON)/obj
REF_OUT_COMMON_LIB := $(REF_OUT_COMMON)/lib
REF_OUT_HAL        := $(REF_OUT_ROOT)/hal
REF_OUT_HAL_OBJ    := $(REF_OUT_HAL)/obj
REF_OUT_HAL_LIB    := $(REF_OUT_HAL)/lib
REF_OUT_THIRD      := $(REF_OUT_ROOT)/thirdparty
REF_OUT_THIRD_OBJ  := $(REF_OUT_THIRD)/obj
REF_OUT_THIRD_LIB  := $(REF_OUT_THIRD)/lib
REF_OUT_TEST       := $(REF_OUT_ROOT)/test
REF_OUT_TEST_OBJ   := $(REF_OUT_TEST)/obj
REF_OUT_TEST_LIB   := $(REF_OUT_TEST)/lib
REF_OUT_TEST_BIN   := $(REF_OUT_TEST)/bin
REF_OUT_TOOL       := $(REF_OUT_ROOT)/tools
REF_OUT_TOOL_OBJ   := $(REF_OUT_TOOL)/obj
REF_OUT_TOOL_LIB   := $(REF_OUT_TOOL)/lib
REF_OUT_TOOL_BIN   := $(REF_OUT_TOOL)/bin

#++++++++++++++++++++++++++++++++++++++++++++++++++++
#	Compile Configure
#++++++++++++++++++++++++++++++++++++++++++++++++++++
VSS_INC_PATH :=
VSS_CFLAGS   :=
VSS_STLIBS   :=
VSS_SHLIBS   :=

ifeq ($(CONFIG_MOTIONSENSOR),y)
VSS_CFLAGS += -DSUPPORT_GYRO
endif

include $(REF_ROOT)/build/compile_config.mak
include $(REF_ROOT)/build/inc.mak
include $(REF_APPCOMMON_PATH)/inc.mak
include $(REF_HAL_PATH)/inc.mak
include $(REF_THIRDPARTY_PATH)/inc.mak

endif # ifndef $(REF_ROOT_BASECFG)
