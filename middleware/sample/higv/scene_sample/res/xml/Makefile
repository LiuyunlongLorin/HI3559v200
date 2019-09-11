ARCH = $(shell getconf LONG_BIT)

ifeq ($(ARCH), 64)
XML2BIN_DIR = 64bit_xml2bin
else
XML2BIN_DIR = 32bit_xml2bin
endif

all:xml
xml:
	-rm higv.bin
	-rm *.c *.h
	-rm xml2bin
	cp ../../../../../component/higv/higv/tools/$(XML2BIN_DIR)/xml2bin ./
	./xml2bin -b ./higv.bin -e ./ -t c -x ./ -l ../lan
	cp *.bin ../../ -rf
	cp higv*.c ../../ -rf
	cp higv*.h ../../ -rf
clean:
	-rm -rf *.c *.h
	-rm higv.bin
	-rm xml2bin
