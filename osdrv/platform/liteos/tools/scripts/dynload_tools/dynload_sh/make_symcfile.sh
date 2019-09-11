#/bin/bash

if [ -z "$2" ] ; then
	echo "Output file not specified!" 1>&2
	exit 1
fi

if [ ! -f "$1" ] ; then
	echo "Input file not found." 1>&2
	exit 1
fi

cat > "$2" <<EOFH
#include "los_elf_symbol.inc"


#define SYMBOL_ENTRY(_l, _symbol) \\
extern void _symbol(void); \\
GDSYMBOL _l LOS_HAL_TABLE_ENTRY(dynload_gsymbol) = \\
{ \\
    #_symbol, \\
    (AARCHPTR)_symbol \\
};

EOFH

sed -r 's#^(.+)$#SYMBOL_ENTRY\(\1_symbol, \1\)#' < "$1" >> "$2"
