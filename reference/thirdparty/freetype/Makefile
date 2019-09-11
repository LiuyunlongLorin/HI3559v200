CUR_DIR := $(shell pwd)
################################################################################
#https://download.savannah.gnu.org/releases/freetype/freetype-2.9.1.tar.gz
BUILD := freetype-2.9.1
################################################################################
.PHONY: $(BUILD) clean distclean
defalut: $(BUILD)
################################################################################

download:
	@if [ ! -f $(BUILD).tar.gz ]; then curl --insecure -O https://download.savannah.gnu.org/releases/freetype/$(BUILD).tar.gz; fi

$(BUILD): download
	@if [ ! -d  "$(BUILD)" ]; then\
		tar -xzf $(BUILD).tar.gz;\
		cd $(BUILD);./configure --prefix=$(CUR_DIR)/$(BUILD)/_INSTALL --without-zlib;cd -;\
	fi
	make -C $(BUILD)
	make install -C $(BUILD)

clean:
	-make -C $(BUILD) clean

distclean: clean
	-rm -rf $(BUILD)

################################################################################
