ifeq ($(PARAM_FILE), )
    PARAM_FILE:=../../../../mpp/Makefile.param
    include $(PARAM_FILE)
endif

default:
	$(CC) -Wall -O2 -I../ adc_test.c -o adc_test -static

clean:
	@rm -rf adc_test *.o
