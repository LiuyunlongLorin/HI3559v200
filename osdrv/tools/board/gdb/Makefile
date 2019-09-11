TOOLS_TOP_DIR := $(shell pwd)
TOOL_TAR_BALL := gdb-7.9.1.tar.gz
TOOL_TAR_BALL2 := ncurses-6.0.tar.gz
TOOL_NAME := gdb-7.9.1
TOOL_NAME2 := ncurses-6.0
TOOL_BIN := inst
TOOL_BIN2 := bin
OSDRV_CROSS ?= arm-himix100-linux
OSDRV_CROSS_CFLAGS ?= -mcpu=cortex-a7 -mfloat-abi=softfp -mfpu=neon-vfpv4
all:
	@if [ ! -f $(TOOLS_TOP_DIR)/$(TOOL_NAME)/gdb/gdb ]; then \
		-rm $(TOOLS_TOP_DIR)/$(TOOL_NAME) -rf; \
		-rm $(TOOLS_TOP_DIR)/$(TOOL_NAME2) -rf; \
		tar -xf $(TOOL_TAR_BALL2); \
		mkdir -p $(TOOLS_TOP_DIR)/$(TOOL_BIN)/; \
		pushd $(TOOLS_TOP_DIR)/$(TOOL_NAME2)/; \
			./configure --host=$(OSDRV_CROSS) CFLAGS="$(OSDRV_CROSS_CFLAGS)" CPPFLAGS="-P" \
			LDFLAGS="$(OSDRV_CROSS_CFLAGS)" --prefix=$(TOOLS_TOP_DIR)/$(TOOL_BIN)/; \
			make -j > /dev/null; \
			make install;\
		popd; \
		tar -xf $(TOOL_TAR_BALL); \
		mkdir -p $(TOOLS_TOP_DIR)/$(TOOL_BIN)/; \
		pushd $(TOOLS_TOP_DIR)/$(TOOL_NAME)/; \
			./configure --host=$(OSDRV_CROSS) CFLAGS="$(OSDRV_CROSS_CFLAGS) -lm"\
			LDFLAGS="$(OSDRV_CROSS_CFLAGS)" --disable-tui\
			LDFLAGS=-L$(TOOLS_TOP_DIR)/$(TOOL_BIN)/lib --prefix=$(TOOLS_TOP_DIR)/$(TOOL_BIN2); gdb_cv_prfpregset_t_broken=no \
			make -j > /dev/null; \
			make install;\
		popd; \
	fi;
	cp $(TOOLS_TOP_DIR)/$(TOOL_NAME)/gdb/gdb $(TOOLS_TOP_DIR)/gdb-$(OSDRV_CROSS) -rf; \

.PHONY: clean
clean:
	-pushd $(TOOLS_TOP_DIR)/$(TOOL_NAME)/ &&\
	make -C $(TOOLS_TOP_DIR)/$(TOOL_NAME)/ clean && \
	popd
	-pushd $(TOOLS_TOP_DIR)/$(TOOL_NAME2)/ &&\
	make -C $(TOOLS_TOP_DIR)/$(TOOL_NAME2)/ clean && \
	popd

.PHONY: distclean
distclean:
	-rm $(TOOLS_TOP_DIR)/$(TOOL_NAME) -rf;
	-rm $(TOOLS_TOP_DIR)/$(TOOL_NAME2) -rf;
	-rm $(TOOLS_TOP_DIR)/$(TOOL_BIN) -rf;
	-rm $(TOOLS_TOP_DIR)/$(TOOL_BIN2) -rf;
	-rm $(TOOLS_TOP_DIR)/gdb-$(OSDRV_CROSS) -rf;

