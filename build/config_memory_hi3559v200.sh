#!/bin/sh

# path configure
sdk_build_path=`pwd`
sdk_path=`pwd`/..

osdrv_cfg_path=${sdk_path}/osdrv/osdrv_mem_cfg.sh
bakfile_suffix="_bak"

usage()
{
	echo "usage: $0 .config [ clean ]"
	echo "eg   : $0 .config"
	echo "eg   : $0 .config clean"
}

# backup config file, $1: config file path
config_file_backup()
{
	config_filepath=$1
	bakconfig_filepath=${config_filepath}${bakfile_suffix}
	#echo ${config_filepath}
	#echo ${bakconfig_filepath}

	if test ! -e ${bakconfig_filepath}
	then
	    cp ${config_filepath} ${bakconfig_filepath}
	fi
}

# restore config file, $1: config file path
config_file_restore()
{
	config_filepath=$1
	bakconfig_filepath=${config_filepath}${bakfile_suffix}
	if test -e ${bakconfig_filepath}
	then
	    cp ${bakconfig_filepath} ${config_filepath}
		echo ${bakconfig_filepath}
		rm ${bakconfig_filepath}
	fi
}

# osdrv memory configure
config_osdrv_mem()
{
	CONFIG_MEM_UBOOT_TEXT_BASE=$(awk 'BEGIN{printf("%#x",'$CONFIG_MEM_LINUX_SYS_BASE'+0x00800000)}')
	CONFIG_MEM_LINUX_MMZ_SIZE=$(awk 'BEGIN{printf("0x%x",'$CONFIG_MEM_LINUX_MMZ_ANONYMOUS_SIZE'+'$CONFIG_MEM_LINUX_MMZ_HIGO_SIZE')}')
	if [ -z "$CONFIG_MEM_IPCM_SIZE" ]; then
	CONFIG_MEM_IPCM_SIZE=0x0100000
	fi
	CONFIG_MEM_IPCM_SHM_1TO0_SIZE=$(awk 'BEGIN{printf("0x%x",'$CONFIG_MEM_IPCM_SIZE'/2)}')
	CONFIG_MEM_IPCM_SHM_0TO1_SIZE=$(awk 'BEGIN{printf("0x%x",'$CONFIG_MEM_IPCM_SIZE'/2 - '0x4000')}')

	sed -i "s/UBOOT_TEXT_BASE=.*0x.*$/UBOOT_TEXT_BASE=${CONFIG_MEM_UBOOT_TEXT_BASE}/g" ${1}
	sed -i "s/RAWPARAM_MEM_BASE=.*0x.*$/RAWPARAM_MEM_BASE=${CONFIG_MEM_PARAM_BASE}/g" ${1}

	sed -i "s/IPCM_SHM_PHYS_1TO0=$MEM_IPCM_SHM_PHYS_1TO0.*$/IPCM_SHM_PHYS_1TO0=${CONFIG_MEM_IPCM_BASE}/g" ${1}
	sed -i "s/IPCM_SHM_SIZE_1TO0=$MEM_IPCM_SHM_PHYS_1TO0.*$/IPCM_SHM_SIZE_1TO0=${CONFIG_MEM_IPCM_SHM_1TO0_SIZE}/g" ${1}
	sed -i "s/IPCM_SHM_SIZE_0TO1=$MEM_IPCM_SHM_PHYS_1TO0.*$/IPCM_SHM_SIZE_0TO1=${CONFIG_MEM_IPCM_SHM_0TO1_SIZE}/g" ${1}

	sed -i "s/LITEOS_SYS_MEM_BASE=$DDR_MEM_BASE.*$/LITEOS_SYS_MEM_BASE=${CONFIG_MEM_HUAWEILITE_SYS_BASE}/g" ${1}
	sed -i "s/LITEOS_SYS_MEM_SIZE=.*0x.*$/LITEOS_SYS_MEM_SIZE=${CONFIG_MEM_HUAWEILITE_SYS_SIZE}/g" ${1}
	sed -i "s/LITEOS_MMZ_MEM_BASE=$LITEOS_SYS_MEM_BASE.*$/LITEOS_MMZ_MEM_BASE=${CONFIG_MEM_HUAWEILITE_MMZ_BASE}/g" ${1}
	sed -i "s/LITEOS_MMZ_MEM_SIZE=.*0x.*$/LITEOS_MMZ_MEM_SIZE=${CONFIG_MEM_HUAWEILITE_MMZ_SIZE}/g" ${1}
	sed -i "s/LITEOS_TEXT_OFFSET=.*0x.*$/LITEOS_TEXT_OFFSET=0x00000000/g" ${1}
	sed -i "s/LITEOS_NOACCESS_MEM_BASE=.*0x.*$/LITEOS_NOACCESS_MEM_BASE=${CONFIG_MEM_LINUX_SYS_BASE}/g" ${1}
	sed -i "s/LITEOS_NOACCESS_MEM_SIZE=.*0x.*$/LITEOS_NOACCESS_MEM_SIZE=${CONFIG_MEM_LINUX_SYS_SIZE}/g" ${1}

	sed -i "s/LINUX_SYS_BASE=.*/LINUX_SYS_BASE=${CONFIG_MEM_LINUX_SYS_BASE}/g" ${1}
	sed -i "s/LINUX_MEM_SIZE=.*0x.*$/LINUX_MEM_SIZE=${CONFIG_MEM_LINUX_SYS_SIZE}/g" ${1}
	sed -i "s/LINUX_MMZ_SIZE=.*0x.*$/LINUX_MMZ_SIZE=${CONFIG_MEM_LINUX_MMZ_SIZE}/g" ${1}
	LINUX_ENTRY=$(awk 'BEGIN{printf("%#x",'$CONFIG_MEM_LINUX_SYS_BASE'+'0x8000')}')
	sed -i "s/LINUX_ENTRY=.*0x.*$/LINUX_ENTRY=${LINUX_ENTRY}/g" ${1}
}

check()
{
	CONFIG_MEM_LINUX_MMZ_SIZE=$(awk 'BEGIN{printf("0x%x",'$CONFIG_MEM_LINUX_MMZ_ANONYMOUS_SIZE'+'$CONFIG_MEM_LINUX_MMZ_HIGO_SIZE')}')
	HUAWEILITE_MMZ_SIZE=$(awk 'BEGIN{printf("%dM%dK",'$CONFIG_MEM_HUAWEILITE_MMZ_SIZE'/0x100000,'$CONFIG_MEM_HUAWEILITE_MMZ_SIZE'%0x100000/0x400)}')
	LINUX_SYS_MEM_SIZE=$(awk 'BEGIN{printf("%dM",'$CONFIG_MEM_LINUX_SYS_SIZE'/0x100000)}')
	LINUX_MMZ_MEM_SIZE=$(awk 'BEGIN{printf("%dM%dK",'$CONFIG_MEM_LINUX_MMZ_SIZE'/0x100000,'$CONFIG_MEM_LINUX_MMZ_SIZE'%0x100000/0x400)}')
	echo ${CONFIG_MEM_HUAWEILITE_MMZ_SIZE} $HUAWEILITE_MMZ_SIZE
	echo ${CONFIG_MEM_LINUX_SYS_SIZE} $LINUX_SYS_MEM_SIZE
	echo ${CONFIG_MEM_LINUX_MMZ_SIZE} $LINUX_MMZ_MEM_SIZE
}

build_default()
{
	config_file_backup ${osdrv_cfg_path}
	config_osdrv_mem ${osdrv_cfg_path}
	check
}
build_clean()
{
	config_file_restore ${osdrv_cfg_path}
}

[ $# -eq 0 ] && { usage; exit; }

source ${sdk_path}/$1

[ $# -eq 1 ] && { build_default; exit; }
[ $# -eq 2 ] && [ $2 == "clean" ] && { build_clean; exit;}

