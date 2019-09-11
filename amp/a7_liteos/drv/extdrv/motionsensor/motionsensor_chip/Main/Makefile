ifeq ($(EXTDRV_PARAM_FILE), ) 
    EXTDRV_PARAM_FILE:=../Makefile.param
    include $(EXTDRV_PARAM_FILE)
endif


INC := -I$(PUB_DIR)
INC += -I$(MAIN_DIR)
INC += -I$(PROC_DIR)
#INC += -I$(DEV_DIR)/BMI150

#obj-m := $(TARGET).o
#$(TARGET)-objs := motionsensor.o

ifeq ($(SENSOR_TYPE), ICM20690)
	INC += -I$(DEV_DIR)/ICM20690
	$(OBJS) += ../SensorDev/ICM20690/icm20690.o
	$(OBJS) += ../SensorDev/ICM20690/motionsensorgpio.o
	#$(TARGET)-objs += ../SensorDev/ICM20690/icm20690.o
	#$(TARGET)-objs += ../SensorDev/ICM20690/motionsensorgpio.o
endif

ifeq ($(TRANSFER_TYPE), I2C)
	$(TARGET)-objs += ../TransferType/i2c/i2c_dev.o
	INC += -I$(TRANSFER_DIR)/i2c
endif

ifeq ($(TRANSFER_TYPE), SPI)
	INC += -I$(TRANSFER_DIR)/spi
	$(OBJS) += ../TransferType/spi/spi_dev.o
	#$(TARGET)-objs += ../TransferType/spi/spi_dev.o
	INC += -I$(TRANSFER_DIR)/spi
endif
#$(TARGET)-objs += ../proc/motionsensor_proc.o

EXTRA_CFLAGS += $(INC)
EXTRA_CFLAGS +=-Wno-error=date-time
ALL_DIR := $(INC)
ifeq ($(OSTYPE), liteos)
.PHONY: all clean driver_clean install obj_clean

all:driver install
clean:driver_clean obj_clean

driver:
	make  ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(LINUXDIR) M=$(MAIN_DIR) EXTRA_CFLAGS='$(EXTRA_CFLAGS)'  modules

install:
	@mv $(TARGET).ko $(OUT_DIR)

driver_clean:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(LINUXDIR) M=$(MAIN_DIR) clean  

obj_clean:
	$(foreach n, $(subst -I, , $(ALL_DIR)), @rm $(n)/*.o -f && rm $(n)/.*.o.cmd -f);	
else
.PHONY: clean


clean:
	rm -f $(LIB) *.o $(OBJS)
endif
