
.PHONY: all clean linux liteos
all: linux liteos

linux:
	@make -C linux||exit $?

liteos:
	@make -C liteos||exit $?

clean:
	@make clean -C linux||exit $?
	@make clean -C liteos||exit $?
