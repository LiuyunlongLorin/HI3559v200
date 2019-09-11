WORKDIR = $(shell pwd)
export CODE_ROOT := $(shell pwd)/../../../component/higv

include ./../../../cfg.mak
include ./config.mk

ifeq ($(type),debug)
CFLAGS += -g3
else
ifeq ($(type), release)
CFLAGS += -O2
else
CFLAGS += -g -O
endif
endif

BASE_FONT_FILE:= ./res/font/ttf/simhei.ttf
ifneq ($(BASE_FONT_FILE), $(wildcard $(BASE_FONT_FILE)))
all: font_check tips mkxml $(EXE)
endif

SRC := $(notdir $(wildcard *.c))
OBJ := $(patsubst %.c,%.o,$(SRC))

EXE = $(notdir $(WORKDIR))

DEPEND = $(notdir $(wildcard .depend))

.PHONY: all obj exe

ifeq ($(build), )
all: tips mkxml
else
all: tips mkxml $(EXE)
endif

mkxml:
	cd res/xml; make
clxml:
	cd res/xml; make clean

tips:
	@echo -e  $(TIPS)

obj: $(OBJ)
exe: $(OBJ) $(EXE)

.depend: $(SRC)
	$(CXX) $(CXXFLAGS) -w -MM $^ > $@

$(OBJ):%.o:%.c
	echo "  CXX      $@"
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(EXE): $(OBJ)
	echo "  LD      $@"
	echo "$(CFG_LIB)"
	$(CC) $(CFLAGS) $^ $(LDFLAGS) $(LIBS) -o $@

clean: clxml
	echo "  CLEAN"
	$(RM) $(EXE) $(OBJ) $(LIB) $(DEPEND)
	$(RM) higv.bin
font_check:
	@echo -e ""
	@echo -e ""
	@echo -e "\033[41;37m/*                  error!!!                         */\033[0m"
	@echo -e "\033[44;37m/*                  error!!!                         */\033[0m"
	@echo -e ""
	@echo -e "\033[44;37m  There aren't ttf fonts in the project!!!\033[0m"
	@echo -e "\033[44;37m  please put the fonts to ./middleware/sample/higv/higv_sample/res/font/ttf directory!\033[0m"
	@echo -e "\033[44;37m  You can see ./middleware/sample/higv/Readme.txt to get more information\033[0m"
	@echo -e ""
	@echo -e ""
	@echo -e ""
	@echo -e ""

ifneq ($(DEPEND),)
include .depend
endif
