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


IPCMSG_LIB := ../../out/$(CPU_TYPE)_$(OSTYPE)/libipcmsg_$(CPU_TYPE)_$(OSTYPE).a

SECUREC_LIB := $(REL_LIB)/libsecurec.a

# compile rule

OBJS  := $(SRCS:%.c=%.o)

.PHONY : clean all

all: $(TARGET)

$(TARGET): $(OBJS)
	@$(CC) $(CFLAGS) -o $@ $^ -Wl,-Bstatic $(IPCMSG_LIB) $(SECUREC_LIB) -Wl,-Bdynamic -lpthread

clean:
	@rm -f $(TARGET)
	@rm -f $(OBJS)
