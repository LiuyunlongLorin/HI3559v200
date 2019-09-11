# Enum the C files needed to be compiled, using the relative path
HIFB_SRC:=src/hifb_main.c src/hifb_proc.c
ifeq ($(CONFIG_SYNC_FILE),y)
HIFB_SRC+=src/drv_hifb_fence.c
endif
ifeq ($(OSTYPE),linux)
    ifeq ($(HIARCH), hi3559av100)
        HIFB_SRC += src/hifb_dmabuf.c
    endif
    HIFB_SRC += src/hifb_vsync.c
else ifeq ($(OSTYPE),liteos)
endif

# IPC needn't higo component
#ifneq ($(HIARCH),hi3518)
HIFB_SRC+=src/hifb_klog.c
#endif
