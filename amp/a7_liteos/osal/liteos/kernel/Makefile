ifeq ($(PARAM_FILE), )
    PARAM_FILE:=../../../mpp/Makefile.param
    include $(PARAM_FILE)
endif

src = $(PWD)
EXTRA_CFLAGS := -I$(src)
EXTRA_CFLAGS += -I$(src)/../../include/

csrcs = $(wildcard *.c)
csrcs += ./mmz/media-mem.c ./mmz/mmz-userdev.c
OBJS = $(patsubst %.c,%.o,$(csrcs))

LIB = libhi_osal.a

CFLAGS += $(EXTRA_CFLAGS)
CFLAGS += -D__KERNEL__
CFLAGS += -Wno-implicit-function-declaration

all: $(OBJS) $(LIB)
	@echo -e "\e[0;32;1m--Compiling 'hi_osal'...\e[0;36;1m" 
	@echo --CC=$(CC)
	@echo --HIARCH=$(HIARCH), HICHIP=$(HICHIP)
	@echo -e "\e[0m" 
	@mkdir -p $(REL_KO) && cp $(LIB) $(REL_KO)
	
$(OBJS): %.o : %.c
	@$(CC) $(CFLAGS) -c $< -o $@

$(LIB): $(OBJS)
	@$(AR) $(ARFLAGS) $@ $(OBJS)

clean:
	@$(RM) $(OBJS) $(LIB)

