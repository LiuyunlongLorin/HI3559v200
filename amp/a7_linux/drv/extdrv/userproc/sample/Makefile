ifeq ($(PARAM_FILE), )
    PARAM_FILE:=../../../../mpp/Makefile.param
    include $(PARAM_FILE)
endif

SRCS := sample_userproc.c
TARGET := sample_userproc

INC := -I$(PWD)/../include
INC += -I$(REL_INC)

CFLAGS += $(INC)
CFLAGS += $(LIBS_CFLAGS)

API_LIBS := ../out/libuserproc.a

OBJS := $(SRCS:%.c=%.o)

.PHONY : clean all

all: $(TARGET)

$(TARGET): $(OBJS)
	@$(CC) $(CFLAGS) -lpthread -o $@ $^ $(API_LIBS)  

clean:
	@rm -f $(TARGET)
	@rm -f $(OBJS)

