.PHONY:all mediaserver mediaclient clean

all: mediaserver mediaclient

mediaserver:
	@cd mediaserver; make MAPI_CPU_TYPE=$(MAPI_CPU_TYPE)

mediaclient:
ifeq ($(MAPI_CPU_TYPE), dual)
	@cd mediaclient; make
endif

clean:
	@cd mediaserver; make clean MAPI_CPU_TYPE=$(MAPI_CPU_TYPE)
ifeq ($(MAPI_CPU_TYPE), dual) 
	@cd mediaclient; make clean 
endif
