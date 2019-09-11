#===============================================================================
#                         export variable
#===============================================================================
LOCAL_DIR := $(shell pwd)
MIDDLEWARE_DIR := $(LOCAL_DIR)/../..
include $(MIDDLEWARE_DIR)/base.mak

SERVER_COMMON_DIR         := server_common
RTSPSERVER_DIR       := rtspserver
#===============================================================================
#                         local variable
#===============================================================================



#===============================================================================
#                         rules
#===============================================================================
.PHONY : all clean install uninstall

all:  server_common rtspserver
clean: server_common_clean rtspserver_clean
install: server_common_install rtspserver_install
	#cp -rf $(LOCAL_DIR)/server_common/include/* $(OUT_HEADER_DIR)/
uninstall: server_common_uninstall rtspserver_uninstall

#===============================================================================
#                         server_common
#===============================================================================
.PHONY : server_common server_common_clean server_common_install server_common_uninstall

server_common:
	make -C $(SERVER_COMMON_DIR)
server_common_clean:
	make -C $(SERVER_COMMON_DIR) clean
server_common_install:
	make -C $(SERVER_COMMON_DIR) install
server_common_uninstall:
	make -C $(SERVER_COMMON_DIR) uninstall

#===============================================================================
#                         rtspserver
#===============================================================================
.PHONY : rtspserver rtspserver_clean rtspserver_install rtspserver_uninstall

rtspserver:
	make -C $(RTSPSERVER_DIR)
rtspserver_clean:
	make -C $(RTSPSERVER_DIR) clean
rtspserver_install:
	make -C $(RTSPSERVER_DIR) install
rtspserver_uninstall:
	make -C $(RTSPSERVER_DIR) uninstall