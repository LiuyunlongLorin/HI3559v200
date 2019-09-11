# target source

CFLAGS += -I$(NDK_PATH)/sample/arch/$(HIARCH)/comm
SRCS += $(NDK_PATH)/sample/arch/$(HIARCH)/comm/sample_comm.c

CFLAGS += -I$(NDK_PATH)/code/include/inner
CFLAGS += -I$(NDK_PATH)/code/mediaserver/utils
CFLAGS += -I$(NDK_PATH)/code/mediaserver/adapt/aenc/aac/include/
CFLAGS += -I$(NDK_PATH)/code/mediaserver/adapt/aenc/include/
CFLAGS += -I$(NDK_PATH)/code/mediaserver/configs/sensor/include/
CFLAGS += -I$(NDK_PATH)/code/mediaserver/comm/arch/$(HIARCH)
CFLAGS += -I$(MPP_HEADER_LINUX)
CFLAGS += -I$(MPP_HEADER_LITEOS)
CFLAGS += -L$(REL_LIB)
ifeq ($(CFG_SUPPORT_GYRO), y)
CFLAGS += -I$(MOTIONSENSOR_CHIP_HEADER)
CFLAGS += -I$(MOTIONSENSOR_MNG_HEADER)
CFLAGS += -I$(MOTIONSENSOR_HEADER)
endif
OBJS := $(SRCS:%.c=%.o)


.PHONY : clean all

all: $(TARGET)

ifeq ($(CFG_ENABLE_HUTAF_LLT_WRAPPER),y)
HUTAF_LIBS := -lNCSCore_linux_HiSi-ARM
CFLAGS += -L$(HUTAF_LIB_DIR)
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -Wl,-Bdynamic -lpthread -lm -o $@ $^ $(NDK_LIB_A) $(HUTAF_LIBS) -Wl,-Bstatic $(SDK_LIB) -Wl,-Bdynamic $(AUDIO_LIBS)
else
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -Wl,-Bdynamic -lpthread -lm -o $@ $^ -Wl,-Bstatic $(NDK_LIB_A) $(SDK_LIB) -Wl,-Bdynamic $(AUDIO_LIBS)
#	@$(CC) $(CFLAGS) -lpthread -lm -o $@ $^ $(MAPI_LIBS) $(NDK_LIB_A)
endif

clean:
	echo $(COMMON_DIR)
	@rm -f $(TARGET)
	@rm -f $(OBJS)

cleanstream:
	@rm -f *.h264 *.h265 *.H264 *.H265
	@rm -f *.jpg *.JPG
	@rm -f *.mjp
	@rm -f *.mp4
	@rm -f *.raw
	@rm -f *.yuv *.YUV
	@rm -f *.aac

