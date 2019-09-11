include ../liteos.param

SRCS += $(wildcard ../HuaweiLite/*.c)
SRCS += $(NDK_PATH)/sample/arch/$(HIARCH)/comm/sample_comm.c

ifeq ($(HIARCH),hi3559v200)
SRCS += $(wildcard ./n3/*.c)
SRCS := $(sort $(SRCS))
endif

$(warning echo $(SRCS))
CFLAGS += -I$(MPP_HEADER_LITEOS)

ifeq ($(CFG_SUPPORT_GYRO), y)
CFLAGS += -I$(MOTIONSENSOR_CHIP_HEADER)
CFLAGS += -I$(MOTIONSENSOR_MNG_HEADER)
CFLAGS += -I$(MOTIONSENSOR_HEADER)
endif

LITEOS_LIBDEPS = --start-group $(LITEOS_LIBDEP)  --end-group $(LITEOS_TABLES_LDFLAGS)
LDFLAGS := $(LITEOS_LDFLAGS) --gc-sections




CFLAGS += -I$(NDK_PATH)/sample/arch/$(HIARCH)/comm
CFLAGS += -I$(NDK_PATH)/code/mediaserver/adapt/aenc/aac/include
CFLAGS += -I$(NDK_PATH)/code/mediaserver/adapt/aenc/include
CFLAGS += -I$(NDK_PATH)/code/mediaserver/comm/arch/$(HIARCH)
CFLAGS += -I$(REL_INC) 


OBJS  := $(SRCS:%.c=%.o)

BIN := $(TARGET).bin
MAP := $(TARGET).map

.PHONY : all clean prepare prepare_clean

all: $(BIN)

$(BIN):$(TARGET)
	@rm -f $(OBJS)
	@$(OBJCOPY) -O binary $(TARGET) $(BIN)

$(TARGET): $(OBJS)
	$(LD) $(LDFLAGS) -Map=$(MAP) -o $(TARGET) $(OBJS) -static --start-group $(NDK_LIB_A) $(SDK_LIB) $(LITEOS_LIBDEPS) $(MPP_LIB_LITEOS)/libsecurec.a --end-group
	#@$(OBJDUMP) -d $@ >$@.asm

$(OBJS):%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean: prepare_clean
	@rm -f $(TARGET) $(BIN) $(MAP) *.asm
	@rm -f $(OBJS)
