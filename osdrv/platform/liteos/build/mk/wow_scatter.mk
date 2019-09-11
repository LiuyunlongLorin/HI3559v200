
WOW_SRC := $(LITEOSTOPDIR)/platform/bsp/board/$(LITEOS_PLATFORM)/os_adapt/os_adapt.c
SCATTER_SRC := $(LITEOSTOPDIR)/platform/bsp/board/$(LITEOS_PLATFORM)/os_adapt/os_adapt.c

############## make wow, when only runstop used #####################
wow_image: $(__LIBS)
	@$(CC) -E $(LITEOSTOPDIR)/platform/bsp/board/$(LITEOS_PLATFORM)/board.ld.S -o $(LITEOSTOPDIR)/platform/bsp/board/$(LITEOS_PLATFORM)/board.ld -P
	@touch $(WOW_SRC)

	@for dir in $(LITEOS_SUBDIRS); \
		do $(MAKE) -C $$dir all LITEOS_IMAGE_MACRO=-DMAKE_WOW_IMAGE || exit 1; \
	done

	@$(SCRIPTS_PATH)/scatter_sr/clear_ld.sh $(SCRIPTS_PATH)/ld
	@$(LD) $(LITEOS_LDFLAGS) $(LITEOS_TABLES_WOW_LDFLAGS) -Map=$(BUILD)/$(LITEOS_TARGET).map -o $(BUILD)/$(LITEOS_TARGET) --start-group $(LITEOS_LIBDEP) --end-group
	@touch $(WOW_SRC)

wow: wow_image
	@$(SCRIPTS_PATH)/scatter_sr/liblist.sh wow $(BUILD)/$(LITEOS_TARGET) $(BUILD)/$(LITEOS_TARGET).map $(SCRIPTS_PATH)/scatter_sr $(LITEOS_COMPILER_GCCLIB_PATH) $(LITEOS_COMPILER_CXXLIB_PATH) $(OUT)/lib

################ make scatter, when only scatter used ##############
scatter_image: $(__LIBS)
	@$(CC) -E $(LITEOSTOPDIR)/platform/bsp/board/$(LITEOS_PLATFORM)/board.ld.S -o $(LITEOSTOPDIR)/platform/bsp/board/$(LITEOS_PLATFORM)/board.ld -P
	@touch $(SCATTER_SRC)

	@for dir in $(LITEOS_SUBDIRS); \
		do $(MAKE) -C $$dir all LITEOS_IMAGE_MACRO=-DMAKE_SCATTER_IMAGE || exit 1; \
	done

	@$(SCRIPTS_PATH)/scatter_sr/clear_ld.sh $(SCRIPTS_PATH)/ld
	@$(LD) $(LITEOS_LDFLAGS) $(LITEOS_TABLES_WOW_LDFLAGS) $(LITEOS_TABLES_SCATTER_LDFLAGS) -Map=$(BUILD)/$(LITEOS_TARGET).map -o $(BUILD)/$(LITEOS_TARGET) --start-group $(LITEOS_LIBDEP) --end-group
	@touch $(SCATTER_SRC)

scatter: scatter_image
	@$(SCRIPTS_PATH)/scatter_sr/liblist.sh scatter $(BUILD)/$(LITEOS_TARGET) $(BUILD)/$(LITEOS_TARGET).map $(SCRIPTS_PATH)/scatter_sr $(LITEOS_COMPILER_GCCLIB_PATH) $(LITEOS_COMPILER_CXXLIB_PATH) $(OUT)/lib

################## make wow_scatter, when runstop and scatter both used ###################
wow_scatter:
	@+make wow
	@+make scatter

wow_scatter_clean:
	@-$(RM) $(SCRIPTS_PATH)/scatter_sr/lib_list.*
	@-$(RM) $(SCRIPTS_PATH)/scatter_sr/symbol_list.*
	@$(SCRIPTS_PATH)/scatter_sr/clear_ld.sh $(SCRIPTS_PATH)/ld
	@touch $(WOW_SRC)
	@touch $(SCATTER_SRC)

.PHONY: image wow_image scatter_image wow scatter wow_scatter wow_scatter_clean
