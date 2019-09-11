#
# isp firmware Makefile
#

ifeq ($(PARAM_FILE), )
    PARAM_FILE:=../../../../../Makefile.param
    include $(PARAM_FILE)
endif

ifeq ($(CBB_PARAM_FILE), )
    CBB_PARAM_FILE:=../../../../Makefile.param
    include $(CBB_PARAM_FILE)
endif

ISP_PATH := $(ISP_ROOT)
LIB_PATH = $(ISP_PATH)/user/lib/
OBJ_PATH = $(ISP_PATH)/user/firmware/src/obj
ISP_MKP := $(ISP_PATH)/kernel/mkp/include
ISP_MKP += -I$(ISP_PATH)/kernel/arch/include
ISP_MKP += -I$(ISP_PATH)/kernel/arch/$(HIARCH)/include

ifeq ($(MPP_BUILD), y)
EXT_PATH := $(SDK_PATH)/mpp/$(EXTDRV)
else
EXT_PATH := $(SDK_PATH)/mpp/extdrv
endif

BUS_DIR := $(EXT_PATH)/pwm

ISP_INC :=  -I$(ISP_PATH)/include\
	-I$(ISP_PATH)/include/arch/$(HIARCH)/include\
	-I$(ISP_PATH)/user/3a/include\
	-I$(ISP_PATH)/user/firmware/drv\
	-I$(ISP_PATH)/user/firmware/vreg\
	-I$(ISP_PATH)/user/firmware/vreg/arch/$(HIARCH)\
	-I$(ISP_PATH)/../vreg/arch/$(HIARCH)/include\
	-I$(ISP_PATH)/user/firmware/include\
	-I$(ISP_PATH)/user/firmware/arch/include\
	-I$(ISP_PATH)/user/firmware/arch/$(HIARCH)/include\
	-I$(ISP_PATH)/user/firmware/src/main\
	-I$(EXT_PATH)\
	-I$(SDK_PATH)/mpp/code/arch/$(HIARCH)/include \
	-I$(SDK_PATH)/mpp/code/mkp/include \
	-I$(SDK_PATH)/mpp/code/mkp_inc \
	-I$(SDK_PATH)/mpp/code/include/inner_inc\
	-I$(SDK_PATH)/mpp/cbb/include\
	-I$(SDK_PATH)/mpp/cbb/base/arch/$(HIARCH)/include\
	-I$(SDK_PATH)/mpp/cbb/vi/ext_inc\
	-I$(SDK_PATH)/mpp/cbb/vi/include\
	-I$(ISP_MKP)\
	-I$(COMM_INC)\

SRC_DIR := $(ISP_PATH)/user/firmware/arch/$(HIARCH)/algorithms\
           $(ISP_PATH)/user/firmware/arch/$(HIARCH)/hal\
           $(ISP_PATH)/user/firmware/src/main\
           $(ISP_PATH)/user/firmware/vreg\
		   $(ISP_PATH)/../vreg/src\

LIB_A := $(LIB_PATH)/libisp.a
LIB_S := $(LIB_PATH)/libisp.so

CFLAGS += -Wall -fPIC

ifeq ($(HIGDB),HI_GDB)
CFLAGS += -g
endif

CFLAGS  += -O2
CFLAGS  += $(LIBS_CFLAGS)
DFLAGS  := -DEXT_REG -D$(HI_FPGA)
DFLAGS  += $(ISP_FLAGS)

DEPEND_FILE := $(foreach file,$(subst -I, ,$(ISP_INC)), $(wildcard $(file)/*.h))
SRCS_TMP := $(foreach file,$(subst -S, ,$(SRC_DIR)), $(notdir $(wildcard $(file)/*.c)))
SRCS = $(filter-out hi_drv_vreg.c, $(SRCS_TMP))
CONFIG_JPEGEDCF :=y
ifeq ($(CONFIG_JPEGEDCF), y)
     CFLAGS += -D ENABLE_JPEGEDCF
else
     DEL_SRC_C := isp_dcfinfo.c
     SRCS := $(filter-out $(DEL_SRC_C), $(SRCS))
endif

ifneq ($(CONFIG_HI_ISP_AF_SUPPORT), y)
     SRCS := $(filter-out isp_af.c, $(SRCS))
endif

ifneq ($(CONFIG_HI_ISP_CR_SUPPORT), y)
     SRCS := $(filter-out isp_green_equalization.c, $(SRCS))
endif

ifneq ($(CONFIG_HI_ISP_PREGAMMA_SUPPORT), y)
     SRCS := $(filter-out isp_pregamma.c, $(SRCS))
endif

ifneq ($(CONFIG_HI_ISP_GCAC_SUPPORT), y)
     SRCS := $(filter-out isp_gcac.c, $(SRCS))
endif

ifneq ($(CONFIG_HI_ISP_CA_SUPPORT), y)
     SRCS := $(filter-out isp_ca.c, $(SRCS))
endif

ifneq ($(CONFIG_HI_ISP_EDGEMARK_SUPPORT), y)
     SRCS := $(filter-out isp_edgemark.c, $(SRCS))
endif

ifneq ($(CONFIG_HI_ISP_HLC_SUPPORT), y)
     SRCS := $(filter-out isp_hlc.c, $(SRCS))
endif

ifneq ($(CONFIG_HI_VI_FPN_SUPPORT), y)
     SRCS := $(filter-out isp_fpn.c, $(SRCS))
endif

OBJS := $(SRCS:%.c=$(OBJ_PATH)/%.o)
VPATH = $(subst -S, ,$(SRC_DIR))

COMPILE = $(CC) $(CFLAGS) $(DFLAGS) $(ISP_INC) -lm

$(OBJ_PATH)/%.o : %.c $(DEPEND_FILE)
	@(echo "compiling $< ...")
	@[ -e $(LIB_PATH) ] || mkdir -p $(LIB_PATH)
	@[ -e $(OBJ_PATH) ] || mkdir -p $(OBJ_PATH)
	@($(COMPILE) -o $@ -c $<)

all:$(OBJS)
	@($(AR) -rcv $(LIB_A) $(OBJS))
	@($(CC) $(LIBS_LD_CFLAGS) -shared -fPIC -o $(LIB_S) $(OBJS))
	@mkdir -p $(REL_LIB)
	@cp $(LIB_S) $(REL_LIB)
	@cp $(LIB_A) $(REL_LIB)
clean:
	@$(RM) -rf $(LIB_A) $(LIB_S) $(OBJS)
	@$(RM) -rf $(LIB_PATH) $(OBJ_PATH)
