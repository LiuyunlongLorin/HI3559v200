PROJECT_ROOT := $(shell pwd)/..
-include $(PROJECT_ROOT)/Makefile.param

CROSS_COMPILE = arm-himix200-linux-

PUB_DIR = ../pub/
MNG_DIR = ../../motionsensor_mng/
INCLUDE = ../../include/

CFLAGS := -Wall -Werror -g
SRC := MotionSensor_test.c 
CC := $(CROSS_COMPILE)gcc
RM := rm

INCLUDES := -I$(PUB_DIR)
INCLUDES += -I$(MNG_DIR)
INCLUDES += -I$(INCLUDE)
CFLAGS += $(INCLUDES) 

.PHONY:all app clean
all:app_test install

app_test:$(patsubst %.c,%.o, $(SRC))
	$(CC) $(CFLAGS) $< -o $@

install:app_test
#	mv app_test $(OUT_DIR) 
clean:
	$(RM) app_test $(patsubst %.c,%, $(SRC)) $(patsubst %.c,%.o, $(SRC)) -rf
