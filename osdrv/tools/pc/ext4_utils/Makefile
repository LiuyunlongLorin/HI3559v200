###############################################################################
#
#         Heli Creates it at September,22 2011.
###############################################################################
PREFIX         ?= $(shell pwd)
BIN_DIR         = $(PREFIX)/bin

###############################################################################
CC=gcc
AR=ar
CPP=cpp
LD=ld
RANLIB=ranlib
NM=nm
STRIP=strip

###############################################################################
all:
	@test -d $(PREFIX) || mkdir -p $(PREFIX)
	tar xf ext4_utils.tar.bz2 -C $(PREFIX)
	@test -d $(BIN_DIR)|| mkdir -p $(BIN_DIR)
	cd $(PREFIX)/ext4_utils; \
		make;cp make_ext4fs $(BIN_DIR); \
	cd -

clean:
	rm -rf $(PREFIX)/ext4_utils
	rm -rf $(PREFIX)/bin

distclean: clean
###############################################################################
.PHONY: clean distclean all
###############################################################################
