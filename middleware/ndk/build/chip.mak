export MIDDLEWARE_DIR?= $(NDK_PATH)/..
include $(MIDDLEWARE_DIR)/cfg.mak



ifeq ($(CFG_CHIP_TYPE),hi3556av100)
    export HIARCH=hi3556av100
else ifeq ($(CFG_CHIP_TYPE),hi3519av100)
    export HIARCH=hi3519av100
else ifeq ($(CFG_CHIP_TYPE),hi3559av100)
    export HIARCH=hi3559av100
else ifeq ($(CFG_CHIP_TYPE),hi3518ev300)
    export HIARCH=hi3518ev300
else
    export HIARCH=hi3559v200
endif
