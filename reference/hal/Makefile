PDT_ROOT := $(shell pwd)
include $(PDT_ROOT)/../../.config

.PHONY : all clean

exclude_components := sample
ifneq ($(CONFIG_GAUGE),y)
exclude_components += gauge
endif
ifneq ($(CONFIG_SCREEN),y)
exclude_components += screen
endif
ifneq ($(CONFIG_WIFI),y)
exclude_components += wifi
endif
ifneq ($(CONFIG_GSENSOR),y)
exclude_components += gsensor
endif
ifneq ($(CONFIG_AHD),y)
exclude_components += ahd
endif
ifneq ($(CONFIG_LED),y)
exclude_components += led
endif
ifneq ($(CONFIG_MOTIONSENSOR),y)
exclude_components += motionsensor
endif
ifneq ($(CONFIG_WATCHDOG),y)
exclude_components += watchdog
endif
ifneq ($(CONFIG_TOUCHPAD),y)
exclude_components += touchpad
endif
ifneq ($(CONFIG_GPS),y)
exclude_components += gps
endif
ifneq ($(CONFIG_ACC),y)
exclude_components += acc
endif
components := $(shell find . -maxdepth 1 -type d)
components := $(basename $(patsubst ./%,%,$(components)))
components := $(filter-out $(exclude_components), $(components))

all:
	for comp in $(components);do make -C $$comp||exit;done

clean: $(components)
	for comp in $(components);do make -C $$comp clean;done

