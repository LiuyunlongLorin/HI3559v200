
.PHONY: all clean host slave
all: host slave

host:
	@make -C host||exit $?

slave:
	@make -C slave||exit $?

clean:
	@make clean -C host ||exit $?
	@make clean -C slave||exit $?
