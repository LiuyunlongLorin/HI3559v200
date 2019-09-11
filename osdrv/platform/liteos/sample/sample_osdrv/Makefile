LITEOSTOPDIR ?= ../..

SAMPLE_OUT = .

include $(LITEOSTOPDIR)/config.mk
RM = -rm -rf

LITEOS_LIBDEPS := --start-group $(LITEOS_LIBDEP) --end-group

SRCS = $(wildcard sample.c)

OBJS = $(patsubst %.c,$(SAMPLE_OUT)/%.o,$(SRCS))

all: $(OBJS)

clean:
	@$(RM) *.o  sample *.bin *.map *.asm

$(OBJS): $(SAMPLE_OUT)/%.o : %.c
ifneq ($(LITEOS_CPU_TYPE), arm926)

ifneq ($(OUT)/lib/libipcm.a, $(wildcard $(OUT)/lib/libipcm.a))
	echo "$(OUT)"
	cp -rf $(LITEOS_CPU_TYPE)/*.a $(OUT)/lib
endif
endif
	$(CC) $(LITEOS_CFLAGS) $(LITEOS_CXXFLAGS) -c $< -o $@
	$(LD) $(LITEOS_LDFLAGS) --gc-sections -Map=$(SAMPLE_OUT)/sample.map -o $(SAMPLE_OUT)/sample ./$@ $(LITEOS_LIBDEPS) $(LITEOS_TABLES_LDFLAGS)
	$(OBJCOPY) -O binary $(SAMPLE_OUT)/sample $(SAMPLE_OUT)/sample.bin
	$(OBJDUMP) -d $(SAMPLE_OUT)/sample >$(SAMPLE_OUT)/sample.asm

