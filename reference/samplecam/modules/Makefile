
SUBDIRS :=  init media

.PHONY: all $(SUBDIRS) clean
all:$(SUBDIRS)

$(SUBDIRS):
	@make -C $@||exit $?

clean:
	@for MOD in $(SUBDIRS);do\
		make clean -C $$MOD||exit $?;done
