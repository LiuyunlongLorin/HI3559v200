############################# SRCs #################################
HWI_SRC    :=
MMU_SRC    :=
NET_SRC    :=
TIMER_SRC  :=
UART_SRC   :=
USB_SRC    :=

############################# hi3516a Options#################################
ifeq ($(LOSCFG_PLATFORM_HI3516A), y)
    LITEOS_CMACRO        += -DLOSCFG_PLATFORM_HI3516A
    HWI_TYPE    := gic
    MMU_TYPE    := general
    NET_TYPE    := higmac
    TIMER_TYPE  := general
    UART_TYPE   := amba-pl011
    USB_TYPE    := usb_temp

    ifeq ($(LOSCFG_VENDOR), y)
        -include $(LITEOSTOPDIR)/vendor/vendor_hi3516a.mk
    endif

############################# hi3518ev200 Options#################################
else ifeq ($(LOSCFG_PLATFORM_HI3518EV200), y)
    LITEOS_CMACRO        += -DLOSCFG_PLATFORM_HI3518EV200
    HWI_TYPE    := mono_vic
    MMU_TYPE    := general
    NET_TYPE    := hieth
    TIMER_TYPE  := general
    UART_TYPE   := amba-pl011
    USB_TYPE    := usb_temp

    ifeq ($(LOSCFG_VENDOR), y)
        -include $(LITEOSTOPDIR)/vendor/vendor_hi3518ev200.mk
    endif

############################# hi3516cv300 Options#################################
else ifeq ($(LOSCFG_PLATFORM_HI3516CV300), y)
    LITEOS_CMACRO        += -DLOSCFG_PLATFORM_HI3516CV300
    HWI_TYPE    := mono_vic
    MMU_TYPE    := general
    NET_TYPE    := hieth
    TIMER_TYPE  := general
    UART_TYPE   := amba-pl011
    USB_TYPE    := usb_temp

    ifeq ($(LOSCFG_VENDOR), y)
        -include $(LITEOSTOPDIR)/vendor/vendor_hi3516cv300.mk
    endif

############################# him5v100 Options#################################
else ifeq ($(LOSCFG_PLATFORM_HIM5V100), y)
    LITEOS_CMACRO        += -DLOSCFG_PLATFORM_HIM5V100
    HWI_TYPE    := mono_vic
    TIMER_TYPE  := general
    UART_TYPE   := amba-pl011

############################# hi3911 Options#################################
else ifeq ($(LOSCFG_PLATFORM_HI3911), y)
    LITEOS_CMACRO        += -DLOSCFG_PLATFORM_HI3911
    HWI_TYPE    := mono_vic
    MMU_TYPE    := general
    TIMER_TYPE  := general
    UART_TYPE   := dw-3.0.8a

############################# hi3519 Options#################################
else ifeq ($(LOSCFG_PLATFORM_HI3519), y)
    LITEOS_CMACRO        += -DLOSCFG_PLATFORM_HI3519
    ifeq ($(LOSCFG_ARCH_CORTEX_A7), y)
        LITEOS_CMACRO        += -DLOSCFG_ARCH_CORTEX_A7
        HWI_TYPE    := gic
        MMU_TYPE    := general
        NET_TYPE    := higmac
        TIMER_TYPE  := general
        UART_TYPE   := amba-pl011
        USB_TYPE    := usb2.0
    else ifeq ($(LOSCFG_ARCH_CORTEX_A17), y)
        LITEOS_CMACRO        += -DLOSCFG_ARCH_CORTEX_A17
        HWI_TYPE    := gic
        MMU_TYPE    := general
        NET_TYPE    := higmac
        TIMER_TYPE  := general
        UART_TYPE   := amba-pl011
        USB_TYPE    := usb2.0
    endif

############################# hi3519v101 Options#################################
else ifeq ($(LOSCFG_PLATFORM_HI3519V101), y)
    LITEOS_CMACRO        += -DLOSCFG_PLATFORM_HI3519V101
    ifeq ($(LOSCFG_ARCH_CORTEX_A7), y)
        LITEOS_CMACRO        += -DLOSCFG_ARCH_CORTEX_A7
        HWI_TYPE    := gic
        MMU_TYPE    := general
        NET_TYPE    := higmac
        TIMER_TYPE  := general
        UART_TYPE   := amba-pl011
    ifeq ($(LOSCFG_DRIVERS_USB_HOST_XHCI), y)
        USB_TYPE    := usb3.0
    else ifeq ($(LOSCFG_DRIVERS_USB_HOST_EHCI), y)
        USB_TYPE    := usb2.0
    endif
    else ifeq ($(LOSCFG_ARCH_CORTEX_A17), y)
        LITEOS_CMACRO        += -DLOSCFG_ARCH_CORTEX_A17
        HWI_TYPE    := gic
        MMU_TYPE    := general
        NET_TYPE    := higmac
        TIMER_TYPE  := general
        UART_TYPE   := amba-pl011
        USB_TYPE    := usb2.0
    endif

############################# hi3559 Options#################################
else ifeq ($(LOSCFG_PLATFORM_HI3559), y)
    LITEOS_CMACRO        += -DLOSCFG_PLATFORM_HI3559
    ifeq ($(LOSCFG_ARCH_CORTEX_A7), y)
        LITEOS_CMACRO        += -DLOSCFG_ARCH_CORTEX_A7
        HWI_TYPE    := gic
        MMU_TYPE    := general
        NET_TYPE    := higmac
        TIMER_TYPE  := general
        UART_TYPE   := amba-pl011
        USB_TYPE    := usb2.0
    else ifeq ($(LOSCFG_ARCH_CORTEX_A17), y)
        LITEOS_CMACRO        += -DLOSCFG_ARCH_CORTEX_A17
        HWI_TYPE    := gic
        MMU_TYPE    := general
        NET_TYPE    := higmac
        TIMER_TYPE  := general
        UART_TYPE   := amba-pl011
        USB_TYPE    := usb2.0
    endif

############################# HI3559AV100ES Options#################################
else ifeq ($(LOSCFG_PLATFORM_HI3559AV100ES), y)
    LITEOS_CMACRO    += -DLOSCFG_PLATFORM_HI3559AV100ES
    ifeq ($(LOSCFG_ARCH_CORTEX_M7), y)
        LITEOS_CMACRO    += -DLOSCFG_ARCH_CORTEX_M7
    else ifeq ($(LOSCFG_ARCH_CORTEX_A53_AARCH64), y)
        LITEOS_CMACRO    += -DLOSCFG_ARCH_CORTEX_A53_AARCH64
        HWI_TYPE    := gic
        MMU_TYPE    := arm-v8
        NET_TYPE    := higmac
        TIMER_TYPE  := general
        UART_TYPE   := amba-pl011
        USB_TYPE    := usb3.0_temp
    else ifeq ($(LOSCFG_ARCH_CORTEX_A53_AARCH32), y)
        LITEOS_CMACRO    += -DLOSCFG_ARCH_CORTEX_A53_AARCH32
        HWI_TYPE    := gic
        MMU_TYPE    := general
        NET_TYPE    := higmac
        TIMER_TYPE  := general
        UART_TYPE   := amba-pl011
        USB_TYPE    := usb3.0_temp
    endif
############################# HI3559AV100 Options#################################
else ifeq ($(LOSCFG_PLATFORM_HI3559AV100), y)
    LITEOS_CMACRO+= -DLOSCFG_PLATFORM_HI3559AV100
    ifeq ($(LOSCFG_ARCH_CORTEX_M7), y)
        LITEOS_CMACRO+= -DLOSCFG_ARCH_CORTEX_M7
    else ifeq ($(LOSCFG_ARCH_CORTEX_A53_AARCH64), y)
        LITEOS_CMACRO+= -DLOSCFG_ARCH_CORTEX_A53_AARCH64
        HWI_TYPE    := gic
        MMU_TYPE    := arm-v8
        NET_TYPE    := higmac
        TIMER_TYPE  := general
        UART_TYPE   := amba-pl011
        USB_TYPE    := usb3.0_temp
    else ifeq ($(LOSCFG_ARCH_CORTEX_A53_AARCH32), y)
        LITEOS_CMACRO+= -DLOSCFG_ARCH_CORTEX_A53_AARCH32
        HWI_TYPE    := gic
        MMU_TYPE    := general
        NET_TYPE    := higmac
        TIMER_TYPE  := general
        UART_TYPE   := amba-pl011
        USB_TYPE    := usb3.0_temp
    endif
############################# HI3556AV100 Options#################################
else ifeq ($(LOSCFG_PLATFORM_HI3556AV100), y)
    LITEOS_CMACRO+= -DLOSCFG_PLATFORM_HI3556AV100
    ifeq ($(LOSCFG_ARCH_CORTEX_A53_AARCH32), y)
        LITEOS_CMACRO+= -DLOSCFG_ARCH_CORTEX_A53_AARCH32
        HWI_TYPE    := gic
        MMU_TYPE    := general
        NET_TYPE    := higmac
        TIMER_TYPE  := general
        UART_TYPE   := amba-pl011
        USB_TYPE    := usb3.0_hi3556a
    endif
############################# HI3519AV100 Options#################################
else ifeq ($(LOSCFG_PLATFORM_HI3519AV100), y)
    LITEOS_CMACRO+= -DLOSCFG_PLATFORM_HI3519AV100
    ifeq ($(LOSCFG_ARCH_CORTEX_A53_AARCH32), y)
        LITEOS_CMACRO+= -DLOSCFG_ARCH_CORTEX_A53_AARCH32
        HWI_TYPE    := gic
        MMU_TYPE    := general
        NET_TYPE    := higmac
        TIMER_TYPE  := general
        UART_TYPE   := amba-pl011
        USB_TYPE    := usb3.0_hi3556a
    endif
############################# HI3516CV500 Options#################################
else ifeq ($(LOSCFG_PLATFORM_HI3516CV500), y)
    LITEOS_CMACRO+= -DLOSCFG_PLATFORM_HI3516CV500
        LITEOS_CMACRO += -DLOSCFG_ARCH_CORTEX_A7
        HWI_TYPE    := gic
        MMU_TYPE    := general
        NET_TYPE    := hieth
        TIMER_TYPE  := general
        UART_TYPE   := amba-pl011
        USB_TYPE    := usb3.0_temp
############################# HI3516EV200 Options#################################
else ifeq ($(LOSCFG_PLATFORM_HI3516EV200), y)
    LITEOS_CMACRO+= -DLOSCFG_PLATFORM_HI3516EV200
        LITEOS_CMACRO += -DLOSCFG_ARCH_CORTEX_A7
        HWI_TYPE    := gic
        MMU_TYPE    := general
        NET_TYPE    := hieth
        TIMER_TYPE  := general
        UART_TYPE   := amba-pl011
        USB_TYPE    := usb3.0_temp
############################# HI3516DV300 Options#################################
else ifeq ($(LOSCFG_PLATFORM_HI3516DV300), y)
    LITEOS_CMACRO+= -DLOSCFG_PLATFORM_HI3516DV300
        LITEOS_CMACRO += -DLOSCFG_ARCH_CORTEX_A7
        HWI_TYPE    := gic
        MMU_TYPE    := general
        NET_TYPE    := hieth
        TIMER_TYPE  := general
        UART_TYPE   := amba-pl011
        USB_TYPE    := usb3.0_temp
############################# HI3556V200 Options#################################
else ifeq ($(LOSCFG_PLATFORM_HI3556V200), y)
    LITEOS_CMACRO+= -DLOSCFG_PLATFORM_HI3556V200
        LITEOS_CMACRO += -DLOSCFG_ARCH_CORTEX_A7
        HWI_TYPE    := gic
        MMU_TYPE    := general
        NET_TYPE    := hieth
        TIMER_TYPE  := general
        UART_TYPE   := amba-pl011
        USB_TYPE    := usb3.0_temp
############################# HI3559V200 Options#################################
else ifeq ($(LOSCFG_PLATFORM_HI3559V200), y)
    LITEOS_CMACRO+= -DLOSCFG_PLATFORM_HI3559V200
        LITEOS_CMACRO += -DLOSCFG_ARCH_CORTEX_A7
        HWI_TYPE    := gic
        MMU_TYPE    := general
        NET_TYPE    := hieth
        TIMER_TYPE  := general
        UART_TYPE   := amba-pl011
        USB_TYPE    := usb3.0_temp
############################# hi3731 Options#################################
else ifeq ($(LOSCFG_PLATFORM_HI3731), y)
    LITEOS_CMACRO        += -DLOSCFG_PLATFORM_HI3731
    HWI_TYPE    := poly_vic
    MMU_TYPE    := general
    TIMER_TYPE  := general
    UART_TYPE   := amba-pl011
    USB_TYPE    := usb_temp

endif

HWI_SRC    := hwi/$(HWI_TYPE)
MMU_SRC    := mmu/$(MMU_TYPE)
NET_SRC    := net/$(NET_TYPE)
TIMER_SRC  := timer/$(TIMER_TYPE)
UART_SRC   := uart/$(UART_TYPE)
USB_SRC    := usb/$(USB_TYPE)

LITEOS_CORE_COPTS = $(LITEOS_CPU_OPTS) $(LITEOS_FLOAT_OPTS) $(LITEOS_FPU_OPTS)
ifeq ($(LITEOS_CPU_TYPE), cortex-a7)
    LITEOS_CPU_OPTS      := -mcpu=cortex-a7
    LITEOS_FLOAT_OPTS    := -mfloat-abi=softfp
    LITEOS_FPU_OPTS      := -mfpu=neon-vfpv4
    LITEOS_ARM_ARCH      := -march=armv7-a
    LITEOS_BASELIB    += -lcortex-a7
    LITEOS_GCCLIB        :=a7_softfp_neon-vfpv4
    LITEOS_INTERWORK  += $(LITEOS_CORE_COPTS)
    LITEOS_CPU_CAT       :=arm-a
else ifeq  ($(LITEOS_CPU_TYPE), cortex-a17)
    LITEOS_CPU_OPTS      := -mcpu=cortex-a17
    LITEOS_FLOAT_OPTS    := -mfloat-abi=softfp
    LITEOS_FPU_OPTS      := -mfpu=neon-vfpv4
    LITEOS_ARM_ARCH      := -march=armv7-a
    LITEOS_BASELIB    += -lcortex-a17
    LITEOS_GCCLIB        :=a17_softfp_neon-vfpv4
    LITEOS_INTERWORK  += $(LITEOS_CORE_COPTS)
    LITEOS_CPU_CAT       :=arm-a
else ifeq  ($(LITEOS_CPU_TYPE), arm926)
    LITEOS_CPU_OPTS      := -mcpu=arm926ej-s
    LITEOS_FLOAT_OPTS    :=
    LITEOS_FPU_OPTS      :=
    LITEOS_ARM_ARCH      := -march=armv5te
    LITEOS_BASELIB    += -larm926
    LITEOS_GCCLIB        :=armv5te_arm9_soft
    LITEOS_INTERWORK     :=
    LITEOS_CPU_CAT       :=arm-a
else ifeq  ($(LITEOS_CPU_TYPE), cortex-m7)
    LITEOS_CPU_OPTS      := -mcpu=cortex-m7 -mthumb
    LITEOS_FLOAT_OPTS    := -mfloat-abi=hard
    LITEOS_FPU_OPTS      := -mfpu=fpv4-sp-d16
    LITEOS_ARM_ARCH      :=
    LITEOS_BASELIB       += -lcortex-m7
    LITEOS_GCCLIB        :=
    LITEOS_INTERWORK     += $(LITEOS_CORE_COPTS)
    LITEOS_CPU_CAT       :=arm-m
else ifeq ($(LITEOS_CPU_TYPE), cortex-a53_aarch64)
    LITEOS_CPU_OPTS      := -mcpu=cortex-a53
    LITEOS_FLOAT_OPTS    :=
    LITEOS_FPU_OPTS      :=
    LITEOS_ARM_ARCH      := -march=armv8-a
    LITEOS_BASELIB    += -lcortex-a53_aarch64
    LITEOS_GCCLIB        :=armv8_arm64
    LITEOS_INTERWORK  += $(LITEOS_CORE_COPTS)
    LITEOS_CPU_CAT       :=arm-a
else ifeq ($(LITEOS_CPU_TYPE), cortex-a53_aarch32)
   LITEOS_CPU_OPTS      := -mcpu=cortex-a53
    LITEOS_FLOAT_OPTS    :=-mfloat-abi=softfp
    LITEOS_FPU_OPTS      :=-mfpu=neon-vfpv4
    LITEOS_ARM_ARCH      := -march=armv7-a
    LITEOS_BASELIB    += -lcortex-a53_aarch32
    LITEOS_GCCLIB        := a7_softfp_neon-vfpv4
    LITEOS_INTERWORK  += $(LITEOS_CORE_COPTS)
    LITEOS_CPU_CAT       :=arm-a
endif
    LITEOS_NODEBUG       += $(LITEOS_CORE_COPTS)
    LITEOS_ASOPTS        += $(LITEOS_CPU_OPTS)
    LITEOS_CXXOPTS_BASE  += $(LITEOS_CORE_COPTS)
    LITEOS_BASELIB       += -lbsp

PLATFORM_INCLUDE := -I $(LITEOSTOPDIR)/platform/cpu/arm/$(LITEOS_CPU_CAT)/include \
                    -I $(LITEOSTOPDIR)/platform/bsp/board/$(LITEOS_PLATFORM)/include \
                    -I $(LITEOSTOPDIR)/platform/bsp/common \
                    -I $(LITEOSTOPDIR)/platform/bsp/common/pm \
                    -I $(LITEOSTOPDIR)/platform/cpu/arm/$(LITEOS_CPU_CAT)/$(LITEOS_CPU_TYPE)/include

LIB_SUBDIRS             += platform/cpu/arm/$(LITEOS_CPU_CAT)/$(LITEOS_CPU_TYPE) platform/bsp
LITEOS_PLATFORM_INCLUDE += $(PLATFORM_INCLUDE)
LITEOS_CXXINCLUDE       += $(PLATFORM_INCLUDE)
