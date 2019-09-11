################################################################################
#http://www.acme.com/software/thttpd/thttpd-2.29.tar.gz
BUILD := thttpd-2.29
################################################################################
.PHONY: $(BUILD) clean distclean
defalut: $(BUILD)
################################################################################

download:
	@if [ ! -f $(BUILD).tar.gz ]; then curl --insecure -O http://www.acme.com/software/thttpd/$(BUILD).tar.gz; fi

$(BUILD): download
	@if [ ! -d  "$(BUILD)" ]; then\
		tar -xzf $(BUILD).tar.gz;\
		cd $(BUILD);patch -Np1 -f < ../$(BUILD)-hisnet.patch;cd -;\
	fi
	make -C $(BUILD)

clean:
	-make -C $(BUILD) clean

distclean: clean
	-rm -rf $(BUILD)

################################################################################
