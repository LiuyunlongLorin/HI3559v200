#OSTYPE should be linux, CPU_TYPE should be single or big-little
OSTYPE=linux

ifeq ($(PARAM_FILE), )
    PARAM_FILE:=../../../../mpp/Makefile.param
    include $(PARAM_FILE)
endif

ifeq ($(HIARCH),$(filter $(HIARCH), hi3559av100))
    CPU_TYPE=big-little
    include $(PARAM_FILE)
endif

CFLAGS += -I../../include \
          -I$(REL_INC)

SRCS := $(wildcard *.c)
TARGET := $(SRCS:%.c=%)


DATAFIFO_LIB := ../../out/$(CPU_TYPE)_$(OSTYPE)/libdatafifo_$(CPU_TYPE)_$(OSTYPE).a

EXT_LIBA := -L$(REL_LIB) -lmpi -lsecurec -lm

ifeq ($(CONFIG_HI_AUDIO_SUPPORT),y)
    EXT_LIBA+= -lVoiceEngine -ldnvqe -lupvqe
endif

# compile rule

OBJS  := $(SRCS:%.c=%.o)

.PHONY : clean all

all: $(TARGET)

$(TARGET): $(OBJS)
	@$(CC) $(CFLAGS) -o $@ $^ -Wl,-Bstatic $(DATAFIFO_LIB) $(EXT_LIBA) -Wl,-Bdynamic -lpthread -ldl

clean:
	@rm -f $(TARGET)
	@rm -f $(OBJS)
