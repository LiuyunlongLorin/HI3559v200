#!/bin/bash
chmod 777 .config

MAKE()
{
	make -j16
	if [ -e $PWD/out/$cur_platform/vs_server.bin ]
	then
		echo "vs_server.bin is exist"
		make clean
	else
		echo "make error"
		exit 1
	fi
}
MAKEWOW()
{
	make wow
	make -j16
	if [ -e $PWD/out/$cur_platform/lib/obj/wow.O ]
	then
		echo "wow.O is exist"
		cd $PWD/tools
		git checkout scripts/
		cd -
		make clean
	else
		echo "make wow error"
		exit
	fi
}
thumb_decopule()
{
	echo "LOSCFG_THUMB=y" >> .config
	MAKE
}
switch_fulllibc()
{
	sed -i "s/LOSCFG_LIB_LIBCMINI=y/LOSCFG_LIB_LIBC=y/g" .config
	echo "LOSCFG_LIB_LIBM=y" >> .config
	echo "LOSCFG_COMPAT_POSIX=y" >> .config
	MAKE
}

libz_decouple()
{
	echo "LOSCFG_LIB_LIBZ=y" >> .config
	MAKE
}

wow_extkernel_decouple()
{
	echo "LOSCFG_KERNEL_CPPSUPPORT=y" >> .config
	echo "LOSCFG_KERNEL_CPUP=y"    >> .config
	echo "LOSCFG_KERNEL_DYNLOAD=y" >> .config
	echo "LOSCFG_KERNEL_RUNSTOP=y" >> .config
	echo "LOSCFG_KERNEL_SCATTER=y" >> .config
	MAKEWOW
}

nowow_extkernel_decouple()
{
	echo "LOSCFG_KERNEL_CPPSUPPORT=y" >> .config
	echo "LOSCFG_KERNEL_CPUP=y"    >> .config
#	echo "LOSCFG_KERNEL_DYNLOAD=y" >> .config
	MAKE
}

compat_decouple()
{
	echo "LOSCFG_COMPAT_LINUX=y" >> .config
	MAKE
	echo "LOSCFG_COMPAT_CMSIS=y" >> .config
	MAKE
}

common_decouple()
{
	thumb_decopule
	switch_fulllibc
	libz_decouple
	compat_decouple
}
support_platform=(
hi3516a
hi3518ev200
hi3519/cortex-a7
hi3519/cortex-a17
hi3911
him5v100
hi3731
)

if [ $# = 0 ]
then
	echo "######### please input parameter hi3516a #######"
	exit
fi


for cur_platform in ${support_platform[@]}
do
	cp ./tools/build/config/$1_litekernel.config ./.config
	if [ $cur_platform == "hi3516a" ]
	then
		MAKE
		common_decouple
		wow_extkernel_decouple
	fi

	if [ $cur_platform == "hi3518ev200" ]
	then
		sed -i "s/LOSCFG_PLATFORM_HI3516A=y/LOSCFG_PLATFORM_HI3518EV200=y/g" .config
		MAKE
		common_decouple
		wow_extkernel_decouple
	fi

	if [ $cur_platform == "hi3519/cortex-a7" ]
	then
		sed -i "s/LOSCFG_PLATFORM_HI3516A=y/LOSCFG_PLATFORM_HI3519=y/g" .config
		echo "LOSCFG_ARCH_CORTEX_A7=y" >> .config
		MAKE
		common_decouple
		nowow_extkernel_decouple
	fi

	if [ $cur_platform == "hi3519/cortex-a17" ]
	then
		sed -i "s/LOSCFG_PLATFORM_HI3516A=y/LOSCFG_PLATFORM_HI3519=y/g" .config
		echo "LOSCFG_ARCH_CORTEX_A17=y" >> .config
		MAKE
		common_decouple
		nowow_extkernel_decouple
	fi

	if [ $cur_platform == "hi3911" ]
	then
		sed -i "s/LOSCFG_PLATFORM_HI3516A=y/LOSCFG_PLATFORM_HI3911=y/g" .config
		MAKE
		common_decouple
		nowow_extkernel_decouple
	fi

	if [ $cur_platform == "him5v100" ]
	then
		sed -i "s/LOSCFG_PLATFORM_HI3516A=y/LOSCFG_PLATFORM_HIM5V100=y/g" .config
		MAKE
		common_decouple
		nowow_extkernel_decouple
	fi

	if [ $cur_platform == "hi3731" ]
	then
		sed -i "s/LOSCFG_PRODUCT_IPCAMERA=y/LOSCFG_PRODUCT_TV=y/g" .config
		sed -i "s/LOSCFG_PLATFORM_HI3516A=y/LOSCFG_PLATFORM_HI3731=y/g" .config
		MAKE
		common_decouple
		nowow_extkernel_decouple
	fi
done

