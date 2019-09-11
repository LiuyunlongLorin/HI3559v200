
.PHONY: all clean core ini2bin_pc ini2bin_board bin2image bin2flash
all: core ini2bin_pc ini2bin_board bin2image bin2flash

core:
	@make -C core||exit $?

ini2bin_pc:
	@make -C ini2bin RUN_ENV=pc||exit $?

ini2bin_board:
	@make -C ini2bin RUN_ENV=board||exit $?

bin2image:
	@make -C bin2image||exit $?

bin2flash:
	@make -C bin2flash||exit $?

clean:
	@make clean -C core||exit $?
	@make clean -C bin2flash||exit $?
	@make clean -C bin2image||exit $?
	@make clean -C ini2bin RUN_ENV=pc||exit $?
	@make clean -C ini2bin RUN_ENV=board||exit $?
