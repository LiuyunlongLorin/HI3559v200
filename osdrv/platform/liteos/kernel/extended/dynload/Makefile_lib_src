include $(LITEOSTOPDIR)/config.mk

ARFLAGS = cr
LIBOUT = $(BUILD)/kernel/extended/dynload
LIB = $(OUT)/lib/libdynload.a

RM = -rm -rf

SRCS = $(wildcard src/*.c)
COBJS = $(patsubst %.c,$(LIBOUT)/%.o,$(SRCS))

LOCAL_INCLUDE := \
	-I $(LITEOSTOPDIR)/kernel/base/include -I $(LITEOSTOPDIR)/kernel/extended/include

all: $(LIBOUT) $(LIB)

$(COBJS): $(LIBOUT)/%.o : %.c
	@$(CC) $(LITEOS_CFLAGS_INTERWORK) $(LOCAL_INCLUDE) $(LITEOS_GCOV_OPTS) -c $< -o $@

OBJS = $(COBJS)

$(OBJS): $(LIBOUT)

$(LIB): $(OBJS)
	mkdir -p $(OUT)/lib
	cp -rf *.a $(OUT)/lib

$(LIBOUT):
	mkdir -p $(LIBOUT)/src

clean:
	@$(RM) $(OBJS) $(LIB) $(LIBOUT) *.bak *~

.PHONY: all clean
