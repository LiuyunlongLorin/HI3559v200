ifeq ($(PARAM_FILE), )
    PARAM_FILE:=../../../../mpp/Makefile.param
    include $(PARAM_FILE)
endif

default:
	$(CC) -Wall -O2 -I../ wtdg_test.c -o wtdg_test -static

clean:
	@rm -rf rtc_test *.o
