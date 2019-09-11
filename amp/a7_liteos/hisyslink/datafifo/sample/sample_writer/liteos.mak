SDK_LIB_PATH := -L$(REL_LIB)

SDK_LIB := $(SDK_LIB_PATH) --start-group -lhi_osal -lmpi  -l$(ARCH_LIBNAME)_base -l$(ARCH_LIBNAME)_sys $(DATAFIFO_LIB) --end-group

LIBDEP = --start-group $(LITEOS_LIBDEP) -lipcm -lvirt-tty --end-group $(LITEOS_TABLES_LDFLAGS)

LDFLAGS := $(LITEOS_LDFLAGS) --gc-sections


# target source
OBJS  := $(SRCS:%.c=%.o)
OBJS += $(COMM_OBJ)

CFLAGS += -I$(OSAL_ROOT)/include
CFLAGS += -D$(HIARCH)

BIN := $(TARGET).bin
MAP := $(TARGET).map

.PHONY : clean all

all: $(BIN)

$(BIN):$(TARGET)
	@$(OBJCOPY) -O binary $(TARGET) $(BIN)

$(TARGET):$(OBJS)
	@$(LD) $(LDFLAGS) -Map=$(MAP) -o $(TARGET) $(OBJS) $(SDK_LIB) $(LIBDEP)

$(OBJS):%.o:%.c
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	@rm -f $(TARGET) $(BIN) $(MAP) *.asm
	@rm -f $(OBJS)
	@rm -f $(COMM_OBJ)

