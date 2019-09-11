
.PHONY: all clean core ini2bin_pc ini2bin_board
all: ini2bin_pc ini2bin_board core

core:
	@make -C core||exit $?
	
ini2bin_pc:
	@make -C ini2bin RUN_ENV=pc||exit $?
	
ini2bin_board:
	@make -C ini2bin RUN_ENV=board||exit $?

clean:
	@make clean -C core||exit $?
	@make clean -C ini2bin RUN_ENV=pc||exit $?
	@make clean -C ini2bin RUN_ENV=board||exit $?
