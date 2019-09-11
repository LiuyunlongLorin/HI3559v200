NORMAL="^[[0;39m"
RED=\\033[40;31m
GREEN=\\033[42;31m
YELLOW="^[[1;33m"
BLUE="^[[1;34m"
MAGENTA="^[[1;35m"
CYAN="^[[1;36m"
WHITE="^[[1;37m"
CLOSE=\\033[0m

MIDDLEWARE_DIR := $(CODE_ROOT)/../..
include $(MIDDLEWARE_DIR)/base.mak
include $(MEDIA_ADPT_DIR)/medialibs.mak

build := $(CFG_CHIP_TYPE)
type :=release

HIGV_INC_PATH += -I$(CODE_ROOT)/higv/include
HIGO_INC_PATH += -I$(CODE_ROOT)/higo/$(CFG_SDK_TOOLCHAIN)/include

SDK_INC_PATH += $(MEDIA_INCLUDE_PATH)

HIGV_LIB_PATH += -L$(CODE_ROOT)/higv/lib/static
HIGO_LIB_PATH += -L$(CODE_ROOT)/higo/$(CFG_SDK_TOOLCHAIN)/r_static
HIGO_LIB_PATH += -L$(CODE_ROOT)/higo/$(CFG_SDK_TOOLCHAIN)/depend

SDK_LIB_PATH += $(MEDIA_LIBS_PATH_LINK)

SAMPLE_INC_PATH += $(HIGV_INC_PATH)
SAMPLE_INC_PATH += $(HIGO_INC_PATH)
SAMPLE_INC_PATH += $(SDK_INC_PATH)

ifeq ($(build),$(filter $(build), hi3559av100 hi3556av100))
MIDDLEWARE_LIB_PATH +=-L$(COMMON_DIR)/mbuffer/lib
MIDDLEWARE_LIB_PATH +=-L$(COMMON_DIR)/log/lib
MIDDLEWARE_LIB_PATH +=-L$(COMPONENT_DIR)/fileformat/exif/lib
MIDDLEWARE_LIB_PATH +=-L$(THIRDPARTY_DIR)/openexif/lib
MIDDLEWARE_LIB_PATH +=-L$(COMPONENT_DIR)/liteplayer/lib
LIBS +=  -Wl,-Bstatic  -lmbuf -lexif -lopenexif -lavplay_linux  -lsecurec -lmwlog -Wl,-Bdynamic
endif

SAMPLE_LIB_PATH += $(HIGV_LIB_PATH)
SAMPLE_LIB_PATH += $(HIGO_LIB_PATH)
SAMPLE_LIB_PATH += $(SDK_LIB_PATH)
SAMPLE_LIB_PATH += $(MIDDLEWARE_LIB_PATH)

LIBS += -Wl,-Bstatic -ltde -lz -lhigo -lhigoadp  -lpng -ljpeg -lfreetype -lstdc++ -lhigv -Wl,-Bdynamic -lm
LIBS += -Wl,-Bstatic $(MEDIA_LIBS_LINK) -Wl,-Bdynamic $(MEDIA_SO_LIBS_LINK) -ldl -lpthread

CFLAGS += -DHIGV_USE_ADP_INPUT
CFLAGS += -DMINIDV -D_HIGV_KEY_SINGLEQUE -DHIGV_PERFORMACE
CFLAGS += -DNEW_HIGO -DHIGO_VER=410  -D_USE_HD
CXXFLAGS += -Wall -O2 -DNDEBUG $(SAMPLE_INC_PATH)
CXXFLAGS += $(CFLAGS) -Wno-error
LDFLAGS += $(SAMPLE_LIB_PATH) -Wl,--start-group $(LIBS) -Wl,--end-group
LDXXFLAGS += $(SAMPLE_LIB_PATH) -Wl,--start-group $(LIBS) -Wl,--end-group
