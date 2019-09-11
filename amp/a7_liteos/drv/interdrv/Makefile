#Created by liucan, 2012/12/26

ifeq ($(PARAM_FILE), )
	PARAM_FILE:=../../mpp/Makefile.param
	include $(PARAM_FILE)
endif

sub_dir:= 

ifeq ($(CONFIG_HIUSER), y)
    sub_dir += hiuser
endif

ifeq ($(CONFIG_MIPI_RX), y)
    sub_dir += mipi_rx
endif

ifeq ($(CONFIG_MIPI_TX), y)
    sub_dir += mipi_tx
endif

ifeq ($(CONFIG_HI_IR), y)
    sub_dir += hi_ir
endif

ifeq ($(CONFIG_HI_WDG), y)
    sub_dir += wdt
endif

ifeq ($(CONFIG_HI_SYSCFG), y)
    sub_dir += sysconfig
endif

ifeq ($(CONFIG_HI_LSADC), y)
    sub_dir += hi_adc
endif

ifeq ($(CONFIG_HI_RTC), y)
    sub_dir += rtc
endif

clean_dir:= $(foreach dir, $(sub_dir), $(patsubst %, %_clean, $(dir)))

.PHONY: all clean $(sub_dir) $(clean_dir)

all: $(sub_dir)

$(sub_dir):
	@cd $@; make || exit1;

clean: $(clean_dir)

$(clean_dir):
	@cd $(patsubst %_clean, %, $@) ; make clean;
