#!/bin/bash
chmod 777 .config

############### hi3516a compile ##############
cp ./tools/build/config/hi3516a.config ./.config
make clean
make -j16
sed -i "s/LOSCFG_DRIVERS_WIFI_QRD=y/LOSCFG_DRIVERS_WIFI_BCM=y/g" .config
make -j16
#make clean

############## hi3518ev200 compile #############
cp ./tools/build/config/hi3516a.config ./.config
sed -i "s/LOSCFG_PLATFORM_HI3516A=y/LOSCFG_PLATFORM_HI3518EV200=y/g" .config
sed -i "s/LOSCFG_KERNEL_RUNSTOP=y/LOSCFG_KERNEL_RUNSTOP=n/g" .config
sed -i "s/LOSCFG_DRIVERS_HIGMAC=y/LOSCFG_DRIVERS_HIETH_SF=y/g" .config
sed -i "s/LOSCFG_DRIVERS_MMC_HIMMCV100=y/LOSCFG_DRIVERS_MMC_HIMMCV200=y/g" .config
sed -i "s/LOSCFG_DRIVERS_MTD_NAND_HINFC620=y/LOSCFG_DRIVERS_MTD_NAND_HIFMC100=y/g" .config
sed -i "s/LOSCFG_DRIVERS_MTD_SPI_NOR_HISFC350=y/LOSCFG_DRIVERS_MTD_SPI_NOR_HIFMC100=y/g" .config
make -j16
sed -i "s/LOSCFG_DRIVERS_WIFI_QRD=y/LOSCFG_DRIVERS_WIFI_BCM=y/g" .config
make -j16
#make clean

############## hi3519 compile #################
####### cortex a7 #####
cp ./tools/build/config/hi3516a.config ./.config
sed -i "s/LOSCFG_PLATFORM_HI3516A=y/LOSCFG_PLATFORM_HI3519=y/g" .config
echo "LOSCFG_ARCH_CORTEX_A7=y" >> .config
sed -i "s/LOSCFG_KERNEL_RUNSTOP=y/LOSCFG_KERNEL_RUNSTOP=n/g" .config
sed -i "s/LOSCFG_DRIVERS_MTD_NAND_HINFC620=y/LOSCFG_DRIVERS_MTD_NAND_HIFMC100=y/g" .config
sed -i "s/LOSCFG_DRIVERS_MTD_SPI_NOR_HISFC350=y/LOSCFG_DRIVERS_MTD_SPI_NOR_HIFMC100=y/g" .config
make -j16
sed -i "s/LOSCFG_DRIVERS_WIFI_QRD=y/LOSCFG_DRIVERS_WIFI_BCM=y/g" .config
make -j16
###### cortex a17 #####
cp ./tools/build/config/hi3516a.config ./.config
sed -i "s/LOSCFG_PLATFORM_HI3516A=y/LOSCFG_PLATFORM_HI3519=y/g" .config
echo "LOSCFG_ARCH_CORTEX_A17=y" >> .config
sed -i "s/LOSCFG_KERNEL_RUNSTOP=y/LOSCFG_KERNEL_RUNSTOP=n/g" .config
sed -i "s/LOSCFG_DRIVERS_MTD_NAND_HINFC620=y/LOSCFG_DRIVERS_MTD_NAND_HIFMC100=y/g" .config
sed -i "s/LOSCFG_DRIVERS_MTD_SPI_NOR_HISFC350=y/LOSCFG_DRIVERS_MTD_SPI_NOR_HIFMC100=y/g" .config
make -j16
sed -i "s/LOSCFG_DRIVERS_WIFI_QRD=y/LOSCFG_DRIVERS_WIFI_BCM=y/g" .config
make -j16

############ hi3911 compile ##############
cp ./tools/build/config/hi3516a.config ./.config
sed -i "s/LOSCFG_PLATFORM_HI3516A=y/LOSCFG_PLATFORM_HI3911=y/g" .config
sed -i "s/LOSCFG_KERNEL_RUNSTOP=y/LOSCFG_KERNEL_RUNSTOP=n/g" .config
sed -i "s/LOSCFG_DRIVERS_HIGMAC=y/LOSCFG_DRIVERS_HIETH_SF=y/g" .config
sed -i "s/LOSCFG_DRIVERS_MMC_HIMMCV100=y/LOSCFG_DRIVERS_MMC_HIMMCV200=y/g" .config
sed -i "s/LOSCFG_DRIVERS_MTD_NAND_HINFC620=y/LOSCFG_DRIVERS_MTD_NAND_HIFMC100=y/g" .config
sed -i "s/LOSCFG_DRIVERS_MTD_SPI_NOR_HISFC350=y/LOSCFG_DRIVERS_MTD_SPI_NOR_HIFMC100=y/g" .config
make -j16
sed -i "s/LOSCFG_DRIVERS_WIFI_QRD=y/LOSCFG_DRIVERS_WIFI_BCM=y/g" .config
make -j16
#make clean

############### him5v100 compile ##############
cp ./tools/build/config/hi3516a.config ./.config
sed -i "s/LOSCFG_PLATFORM_HI3516A=y/LOSCFG_PLATFORM_HIM5V100=y/g" .config
sed -i "s/LOSCFG_KERNEL_RUNSTOP=y/LOSCFG_KERNEL_RUNSTOP=n/g" .config
sed -i "s/LOSCFG_DRIVERS_HIGMAC=y/LOSCFG_DRIVERS_HIETH_SF=y/g" .config
sed -i "s/LOSCFG_DRIVERS_MMC_HIMMCV100=y/LOSCFG_DRIVERS_MMC_HIMMCV200=y/g" .config
sed -i "s/LOSCFG_DRIVERS_MTD_NAND_HINFC620=y/LOSCFG_DRIVERS_MTD_NAND_HIFMC100=y/g" .config
sed -i "s/LOSCFG_DRIVERS_MTD_SPI_NOR_HISFC350=y/LOSCFG_DRIVERS_MTD_SPI_NOR_HIFMC100=y/g" .config
make -j16
sed -i "s/LOSCFG_DRIVERS_WIFI_QRD=y/LOSCFG_DRIVERS_WIFI_BCM=y/g" .config
make -j16
#make clean

