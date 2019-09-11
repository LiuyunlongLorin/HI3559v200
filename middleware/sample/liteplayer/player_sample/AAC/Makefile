
LOCAL_DIR := $(shell pwd)
MIDDLEWARE_DIR := $(LOCAL_DIR)/../../../..
include $(MIDDLEWARE_DIR)/base.mak
include $(MEDIA_ADPT_DIR)/medialibs.mak


ifeq ($(CFG_OS_TYPE),linux_liteos)
APP_INC := $(MEDIA_LITEOS_INCLUDE_PATH)
else
APP_INC := $(MEDIA_INCLUDE_PATH)
endif

#LDFLAGS := $(LITEOS_LIBDEPS)
# target source
SRCS  += aac_adec_intf.c
OBJS  := $(SRCS:%.c=%.o)

ifeq ($(CFG_OS_TYPE),liteos)
CFLAGS_SO := $(CFLAGS) -mlong-calls -nostdlib -nostdinc -fPIC
LDFLAGS_SO := -fPIC -shared -nostdlib
else ifeq ($(CFG_OS_TYPE),linux_liteos)
CFLAGS_SO := $(LiteCFLAGS) -Wno-error -nostdlib -nostdinc -fPIC
LDFLAGS_SO := -fPIC -shared -nostdlib
else
CFLAGS_SO := $(CFLAGS) -Wno-error
LDFLAGS_SO := -fPIC -shared
endif



TARGET:= hiaacdec

LIB_SO:= lib$(TARGET).so

.PHONY : clean all

all: $(LIB_SO)

$(OBJS): %.o : %.c
	@$(CC) $(CFLAGS_SO) $(APP_INC) -c $< -o $@

$(LIB_SO): $(OBJS)
	$(CC) $(LDFLAGS_SO)  $< -o $@

clean:
	@rm -f $(OBJS)  $(LIB_SO)
