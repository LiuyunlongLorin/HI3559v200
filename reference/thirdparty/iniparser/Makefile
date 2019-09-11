################################################################################
#https://github.com/ndevilla/iniparser/archive/v4.1.tar.gz
BUILD := iniparser-4.1
INIPARSER_VER := iniparser-y
################################################################################
.PHONY: $(BUILD) prepare clean
defalut: $(BUILD) prepare
################################################################################

download:
	@if [ ! -f $(BUILD).tar.gz ]; then curl --insecure -O https://github.com/ndevilla/iniparser/archive/v4.1.tar.gz; fi

$(BUILD): download
	@if [ ! -d  include ]; then\
		tar -xzf $(BUILD).tar.gz;\
		cd $(BUILD);patch -Np1 -f < ../$(BUILD).patch;cd -;\
	fi

prepare:
	@if [ ! -d  include ]; then\
		mkdir -p $(INIPARSER_VER)/include $(INIPARSER_VER)/src;\
		mv $(BUILD)/src/*.c $(INIPARSER_VER)/src;mv $(BUILD)/src/*.h $(INIPARSER_VER)/include;\
		cd $(BUILD);mv INSTALL AUTHORS LICENSE README.md ../$(INIPARSER_VER)/;cd -;\
		rm -rf $(BUILD);\
	fi

clean:
