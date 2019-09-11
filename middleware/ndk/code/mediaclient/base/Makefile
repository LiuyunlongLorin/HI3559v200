ifeq ($(MC_PARAM_FILE), )
    MC_PARAM_FILE:=../Makefile.param
    include $(MC_PARAM_FILE)
endif

#*************************************************************************
# all source file in this module
SDK_LINUX_INC := hi_comm_sys.h hi_comm_video.h hi_common.h hi_debug.h hi_defines.h \
               hi_errno.h hi_math.h hi_tde_api.h hi_tde_errcode.h hi_tde_type.h \
		   hi_type.h hifb.h
SDK_LINUX_INC := $(addprefix $(MPI_OUT_PATH)/include/, $(SDK_LINUX_INC))

SDK_LINUX_KO := hi_osal.ko hi3559_base.ko hi3559_sys.ko hi3559_tde.ko hifb.ko
SDK_LINUX_KO := $(addprefix $(MPI_OUT_PATH)/ko/, $(SDK_LINUX_KO))

SDK_LINUX_LIB := libtde.a
SDK_LINUX_LIB := $(addprefix $(MPI_OUT_PATH)/lib/, $(SDK_LINUX_LIB))


#*************************************************************************
INC_DIR := $(REL_INC)/inner
LIB_DIR := $(REL_LIB)


#*************************************************************************
.PHONY: all clean

all:
	@echo -e "\e[0;32;1m--Compiling 'base'...\e[0;36;1m"
	@echo ---- OSTYPE=$(OSTYPE) HIARCH=$(HIARCH)
	@echo ---- KERNEL_ROOT=$(KERNEL_ROOT)
	@echo -e "\e[0m"
	@mkdir -p $(INC_DIR)
	@mkdir -p $(LIB_DIR)
	@mkdir -p $(REL_KO)
	@cp -f $(SDK_LINUX_INC) $(INC_DIR)
	@cp -f $(SDK_LINUX_KO) $(REL_KO)
	@cp -f $(SDK_LINUX_LIB) $(LIB_DIR)

clean:
	@rm -rf $(INC_DIR)/*
	@rm -rf $(LIB_DIR)/*
	@rm -rf $(REL_KO)/*.ko
