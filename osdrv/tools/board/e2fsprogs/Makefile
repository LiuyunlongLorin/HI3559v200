TOOLS_TOP_DIR ?= $(shell pwd)
TOOL_TAR_BALL := e2fsprogs-1.42.13.tar.gz
TOOL_NAME := e2fsprogs-1.42.13
TOOL_BIN := bin
OSDRV_CROSS ?= arm-himix100-linux
OSDRV_CROSS_CFLAGS ?= -mcpu=cortex-a7 -mfloat-abi=softfp -mfpu=neon-vfpv4

all:
	@if [ -d $(TOOLS_TOP_DIR)/$(TOOL_NAME)/$(TOOL_BIN) ] && \
		[ -f $(TOOLS_TOP_DIR)/$(TOOL_NAME)/$(TOOL_BIN)/sbin/mkfs.ext2 ] && \
		[ -f $(TOOLS_TOP_DIR)/$(TOOL_NAME)/$(TOOL_BIN)/sbin/mkfs.ext3 ] && \
		[ -f $(TOOLS_TOP_DIR)/$(TOOL_NAME)/$(TOOL_BIN)/sbin/mkfs.ext4 ] && \
		[ -f $(TOOLS_TOP_DIR)/$(TOOL_NAME)/$(TOOL_BIN)/sbin/mke2fs ]; then \
		echo "exe already exist"; \
	else \
		if [ -d $(TOOLS_TOP_DIR)/$(TOOL_NAME) ];then rm $(TOOLS_TOP_DIR)/$(TOOL_NAME) -rf; fi; \
		tar -xzf $(TOOL_TAR_BALL); \
		mkdir -p $(TOOLS_TOP_DIR)/$(TOOL_NAME)/$(TOOL_BIN)/; \
		pushd $(TOOLS_TOP_DIR)/$(TOOL_NAME)/; \
			./configure --host=$(OSDRV_CROSS) CFLAGS="$(OSDRV_CROSS_CFLAGS)" \
			LDFLAGS="$(OSDRV_CROSS_CFLAGS)" --prefix=$(TOOLS_TOP_DIR)/$(TOOL_NAME)/$(TOOL_BIN) \
			--disable-tls --disable-defrag > /dev/null; \
			make -j 20 > /dev/null; \
			make install  > /dev/null; \
		popd; \
	fi;
	mkdir -p $(TOOLS_TOP_DIR)/$(TOOL_BIN)/;
	pushd $(TOOLS_TOP_DIR)/$(TOOL_NAME)/$(TOOL_BIN)/sbin; \
		cp -f mkfs.ext2 mkfs.ext3 mkfs.ext4 mke2fs $(TOOLS_TOP_DIR)/$(TOOL_BIN) -rf; \
	popd

.PHONY: clean
clean:
	@if [ -d $(TOOLS_TOP_DIR)/$(TOOL_NAME) ];then \
		make -C $(TOOLS_TOP_DIR)/$(TOOL_NAME)/ clean; \
	fi;

.PHONY: distclean
distclean:
	@if [ -d $(TOOLS_TOP_DIR)/$(TOOL_NAME) ];then rm $(TOOLS_TOP_DIR)/$(TOOL_NAME) -rf;fi;
	@if [ -d $(TOOLS_TOP_DIR)/$(TOOL_BIN) ];then rm $(TOOLS_TOP_DIR)/$(TOOL_BIN) -rf;fi;

