
LIBDEP = --start-group $(LITEOS_LIBDEP) -lipcm -lvirt-tty --end-group $(LITEOS_TABLES_LDFLAGS)

LDFLAGS := $(LITEOS_LDFLAGS) --gc-sections

# target source
OBJS  := $(SRCS:%.c=%.o)
OBJS += $(COMM_OBJ)

CFLAGS += -D$(HIARCH)

BIN := $(TARGET).bin
MAP := $(TARGET).map

.PHONY : clean all

all: $(BIN)

$(BIN):$(TARGET)
	@$(OBJCOPY) -O binary $(TARGET) $(BIN)

$(TARGET):$(OBJS)
	@$(LD) $(LDFLAGS) -Map=$(MAP) -o $(TARGET) $(OBJS) $(IPCMSG_LIB) $(SECUREC_LIB) $(LIBDEP)

$(OBJS):%.o:%.c
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	@rm -f $(TARGET) $(BIN) $(MAP) *.asm
	@rm -f $(OBJS)
	@rm -f $(COMM_OBJ)

