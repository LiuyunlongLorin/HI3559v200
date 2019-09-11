#!/bin/sh

MPP_CFG_PATH=$1
CFG_OPT=$2

get_mpp_cfg()
{
	matched=n
	if test -e ${MPP_CFG_PATH}
	then
		cat ${MPP_CFG_PATH} | grep -Ev "^$|^#.*$" | grep ${CFG_OPT} > /dev/null
		if [ $? -eq 0 ]; then
			matched=y
		fi
	fi
	echo $matched
}

get_mpp_cfg