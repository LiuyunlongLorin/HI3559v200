#!/bin/bash

echo "LITEOS_DYNLDFLAGS += \$(OUT)/obj/kernel/extended/dynload/src/los_dynload_gsymbol.o" >./dynload_sh/dynload_ld.mk

echo "LITEOS_DYNLOADOPTS := \\" >>./dynload_sh/dynload_ld.mk
sed -r -e 's/^.+$/-u&_symbol/' dynload_sh/symbol.list >a
sed -n '1h;1!H;${g;s/\n/ /g;p;}' a >>./dynload_sh/dynload_ld.mk
rm a
