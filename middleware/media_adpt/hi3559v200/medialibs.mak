# define MEDIA_LIBRARY_PATH and MEDIA_LIBS in different chip dir
CURRENT_DIR := $(shell pwd)
include $(MIDDLEWARE_DIR)/base.mak
SDK_LINK_ROOT := $(DEP_DIR)
NDK_LINK_ROOT := ndk
NDK_ARCH := hi3559v200
MW_MEDIA_INCLUDE := $(SDK_LINK_ROOT)/amp/a7_linux/mpp/include  \
                    $(NDK_LINK_ROOT)/code/mediaserver/adapt/aenc/include \
                    $(SDK_LINK_ROOT)/amp/a7_linux/hisyslink/ipcmsg/include \
                    $(SDK_LINK_ROOT)/amp/a7_linux/hisyslink/datafifo/include

MW_MEDIA_DEPRECATED_INC := $(SDK_LINK_ROOT)/amp/a7_linux/mpp/include

MW_MEDIA_LIBRARY_PATH := . \
                 $(NDK_LINK_ROOT)/out/$(NDK_ARCH)/dual/host/lib \
                 $(SDK_LINK_ROOT)/amp/a7_linux/mpp/lib \
                 $(SDK_LINK_ROOT)/amp/a7_linux/hisyslink/datafifo/out/a7_linux \
                 $(SDK_LINK_ROOT)/amp/a7_linux/hisyslink/ipcmsg/out/a7_linux \
                 $(SDK_LINK_ROOT)/amp/a7_linux/drv/extdrv/userproc/out

MW_MEDIA_LIBS := mw_media_adpt uproc $(NDK_ARCH)_sys_client $(NDK_ARCH)_msg_client $(NDK_ARCH)_vcap_client $(NDK_ARCH)_vproc_client $(NDK_ARCH)_venc_client \
        $(NDK_ARCH)_disp_client $(NDK_ARCH)_mapi_acap_client $(NDK_ARCH)_mapi_aenc_client $(NDK_ARCH)_mapi_ao_client \
        $(NDK_ARCH)_log_client mpi hi_cipher tde ipcmsg_a7_linux datafifo_a7_linux securec userproc
