ifeq ($(PARAM_FILE), )
    PARAM_FILE:=../../../../../mpp/Makefile.param
    include $(PARAM_FILE)
endif

OUT_PATH := ../../out
REL_PATH := $(SDK_PATH)/../pub/linux
CreateDir = $(shell [ -d $1 ] || mkdir -p $1 || echo ":mkdir '$1' fail")
dummy := $(call CreateDir, $(OUT_PATH))
dummy := $(call CreateDir, $(REL_PATH))

INC := -I../../include
INC += -I../drv
INC += -I$(REL_INC)
INC += -I$(PWD)/../../include

CFLAGS += $(INC)
CFLAGS += $(LIBS_CFLAGS)

SRCS := mpi_userproc.c


OBJS = $(SRCS:%.c=%.o)

LIB_A := libuserproc.a
LIB_SO := libuserproc.so

.PHONY: all clean

all: $(OBJS) $(LIB_A) $(LIB_SO)
	@echo "$(SRCS)"
	@echo -e "\e[0;32;1m--Compiling userproc...\e[0;36;1m"
	@echo ---- OSTYPE=$(OSTYPE) HIARCH=$(HIARCH) CROSS=$(CROSS)
	@echo ---- HIARCH=$(HIARCH), HICHIP=$(HICHIP), CVER=$(CVER), DBG=$(HIDBG), HI_FPGA=$(HI_FPGA)
	@echo ---- CPU_TYPE=$(CPU_TYPE)
	@echo ---- CFLAGS=$(CFLAGS)
	@echo -e "\e[0m"
	@cp $(LIB_A) $(LIB_SO) $(OUT_PATH)/
	@cp $(LIB_A) $(LIB_SO) $(REL_PATH)/


$(OBJS): %.o : %.c
	@$(CC) $(CFLAGS) -c $< -o $@

$(LIB_A): $(OBJS)
	@$(AR) $(ARFLAGS) $@ $(OBJS)

$(LIB_SO): $(SRCS)
	@$(CC) $(CFLAGS) -fPIC -shared -o $@ $(SRCS)
clean:
	@rm -f $(OBJS) $(LIB_A) $(LIB_SO)
	@rm -f $(OUT_PATH)/$(LIB_A)
	@rm -f $(OUT_PATH)/$(LIB_SO)
