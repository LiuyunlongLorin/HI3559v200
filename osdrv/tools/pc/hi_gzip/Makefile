CC = gcc
RM = -rm -rf

GZIP_VER=gzip-1.8
SRCS = $(wildcard src/*.c)
EXENAME = hi_gzip

all: gzip hi_gzip

gzip:
	@if [ ! -f $(GZIP_VER)/gzip ]; then \
		rm -rf $(GZIP_VER); \
		tar -xf $(GZIP_VER).tgz; \
		pushd $(GZIP_VER);./configure;make -j12;popd; \
	fi

hi_gzip: $(OBJS)
	mkdir bin -p
	cp $(GZIP_VER)/gzip bin;
	@$(CC) $(SRCS) -o bin/$(EXENAME)

clean:
	@rm -rf *.bak *~ bin
	@rm -rf $(GZIP_VER)

distclean: clean

.PHONY: all clean
