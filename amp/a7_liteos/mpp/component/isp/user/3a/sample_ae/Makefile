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
#LIBPATH = $(ISP_PATH)/user/lib
LIBPATH = ./lib
OBJPATH = ./obj

3A_INC  := $(ISP_PATH)/user/3a/include
3A_INC  += -I$(ISP_PATH)/user/3a/arch/include
3A_INC  += -I$(ISP_PATH)/user/3a/arch/$(HIARCH)/include
VREG_INC := $(ISP_PATH)/../vreg
		 
DRV_INC := $(ISP_PATH)/firmware/drv
ISP_INC := $(ISP_PATH)/user/include
ISP_INC += -I$(ISP_PATH)/user/firmware/include
ISP_INC += -I$(ISP_PATH)/user/firmware/arch/include 
ISP_INC += -I$(ISP_PATH)/user/firmware/arch/$(HIARCH)/include

ISP_INCLUDE := $(ISP_PATH)/include
ISP_INCLUDE += -I$(ISP_PATH)/include/arch/$(HIARCH)/include
ISP_MKP := $(ISP_PATH)/kernel/mkp/include
ISP_MKP += -I$(ISP_PATH)/kernel/arch/include
ISP_MKP += -I$(ISP_PATH)/kernel/arch/$(HIARCH)/include

ifeq ($(MPP_BUILD), y)
EXT_PATH := $(ISP_PATH)/ext_drv/$(HIARCH)
else
EXT_PATH := $(ISP_PATH)/ext_drv/$(HIARCH)
endif

BUS_DIR := $(EXT_PATH)/pwm
PIRIS_DIR := $(EXT_PATH)/piris

ARFLAGS = rcv
CFLAGS += -Wall -fPIC

ifeq ($(HIGDB),HI_GDB)
CFLAGS += -g
endif
ifeq ($(CONFIG_JPEGEDCF), y)
     CFLAGS += -D ENABLE_JPEGEDCF 
endif
CFLAGS  += -O2
CFLAGS  += $(LIBS_CFLAGS)
DFLAGS  := -DEXT_REG
DFLAGS  += $(MPP_CFLAGS)

INC := $(COMM_INC) -I$(ISP_INCLUDE) -I$(ISP_MKP) -I$(ISP_INC) -I$(BUS_DIR) -I$(3A_INC) -I$(VREG_INC) -I$(PIRIS_DIR) -I$(DRV_INC)

DEPEND_FILE := $(foreach file,$(subst -I, ,$(INC)),$(wildcard $(file)/*.h))

COMPILE = $(CC) $(CFLAGS) $(DFLAGS) -I$(ISP_MKP) -I$(ISP_INC) -lm

$(OBJPATH)/%.o: ./%.c $(DEPEND_FILE)
	@(echo "compiling $< ...")
	@[ -e $(LIBPATH) ] || mkdir -p $(LIBPATH)
	@[ -e $(OBJPATH) ] || mkdir -p $(OBJPATH)
	@($(COMPILE) -o $@ -c $< $(INC))

SRCS = $(wildcard ./*.c)
OBJS = $(SRCS:%.c=%.o)
OBJS := $(OBJS:./%=obj/%)

LIB_A := $(LIBPATH)/lib_hiae.a
LIB_S := $(LIBPATH)/lib_hiae.so

all:$(OBJS)
	@($(AR) $(ARFLAGS) $(LIB_A) $(OBJS))
	@($(CC) $(LIBS_LD_CFLAGS) -shared -fPIC -o $(LIB_S) $(OBJS))
	#@cp $(LIB_S) $(REL_LIB)
	#@cp $(LIB_A) $(REL_LIB)
clean:
	@$(RM) -rf $(LIB_A) $(LIB_S) $(OBJS)
	@$(RM) -rf $(LIBPATH) $(OBJPATH)

