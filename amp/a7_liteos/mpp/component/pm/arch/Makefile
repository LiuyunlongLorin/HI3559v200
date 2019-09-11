# Describe the include path, using the full path

# Enum the C files needed to be compiled, using the relative path
include $(PM_ROOT)/arch/$(HIARCH)/cfg.mak

#add hal header file
MKP_CFLAGS +=-I $(PM_ROOT)/arch/$(HIARCH)/include
MKP_CFLAGS +=-I $(PM_ROOT)/arch/$(HIARCH)/hal/$(OSTYPE)

#all pm domain needed
SRCS +=  $(ARCH_ROOT)/hal/pm_chip_hal.c

#add regulator 
ifeq ($(CONFIG_PM_REGULATOR_DCDC), y)
MKP_CFLAGS += -DPM_REGULATOR_TYPE_DCDC
else
SRCS += $(ARCH_ROOT)/hal/hal_regualtor_i2c.c
endif

ifeq ($(CONFIG_PM_DOMAIN_CPU_ON), y)
    #Define switch macro 
    MKP_CFLAGS += -DPM_DOMAIN_CPU_ON

    #add  hal src file
    SRCS +=  $(ARCH_ROOT)/hal/hal_cpu_avs.c
    SRCS +=  $(ARCH_ROOT)/hal/$(OSTYPE)/hal_cpu_freq.c
    SRCS +=  $(ARCH_ROOT)/hal/hal_cpu_regulator.c
endif

ifeq ($(CONFIG_PM_DOMAIN_MEDIA_ON), y)
    #Define switch macro 
    MKP_CFLAGS += -DPM_DOMAIN_MEDIA_ON
    
    #add  hal src file
    SRCS +=  $(ARCH_ROOT)/hal/hal_media_avs.c
    SRCS +=  $(ARCH_ROOT)/hal/hal_media_freq.c
    SRCS +=  $(ARCH_ROOT)/hal/hal_media_regulator.c
endif

ifeq ($(CONFIG_PM_DOMAIN_CORE_ON), y)
    #Define switch macro 
    MKP_CFLAGS += -DPM_DOMAIN_CORE_ON
    
    #add  hal src file
    SRCS +=  $(ARCH_ROOT)/hal/hal_core_avs.c
    SRCS +=  $(ARCH_ROOT)/hal/hal_core_freq.c
    SRCS +=  $(ARCH_ROOT)/hal/hal_core_regulator.c
endif

