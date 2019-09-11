NDK烧写参考(以Hi3559ADemo板为例)：

A.烧写multicore单系统版本：
  1.配置ndk的osdrv的编译环境：
    修改ndk根目录下Makefile.param文件，将OSDRV_ROOT路径修改为对应版本发布包的osdrv根目录
  2.在ndk根目录执行make linux编译
  3.单板拨码SW1[1:2]拨到0从SPI Flash启动, SW1.4拨到0从A53MP启动
  4.使用HiTool网口方式烧写发布包multicore小系统
	①烧录地址和分区：
	|     1M       |      9M       |      16M      |
	|--------------|---------------|---------------|
	|    uboot     |     kernel    |     rootfs    |
	②烧写镜像：
	uboot：u-boot-hi3559av100.bin
	kernel：uImage_hi3559av100_multi-core
	rootfs：rootfs_hi3559av100_2k_24bit.yaffs2
	③设置启动参数：
	setenv bootargs 'mem=512M console=ttyAMA0,115200 root=/dev/mtdblock2 rw rootfstype=yaffs2 mtdparts=hinand:1M(boot),9M(kernel),16M(rootfs)'
	setenv bootcmd 'nand read 0x44000000 0x100000 0x900000;bootm 0x44000000'
	saveenv
	reset
  5.加载ko驱动：
  	cd [SDK发布包路径]/mpp/out/linux/multi-core/ko/
  	./load3559av100_multicore -i -sensor0 imx477
  6.指定音频动态库路径：
  	export LD_LIBRARY_PATH=[SDK发布包路径]/mpp/out/linux/multi-core/lib/

B.烧写BigLittle双系统版本：
  1.配置ndk的osdrv的编译环境：
    修改ndk根目录下Makefile.param文件，将OSDRV_ROOT路径修改为对应版本发布包的osdrv根目录
  2.在ndk根目录执行make dual编译（编译前先make clean,清除之前的.o文件，因为server目录下单系统和双系统编译出来的.o文件不同）
  3.单板拨码SW1[1:2]拨到0从SPI Flash启动, SW1.4拨到0从A53MP启动
  4.使用HiTool网口方式烧写发布包BigLittle小系统
    ①烧录地址和分区：
	|     1M       |      9M       |      32M      |      16M      |      1M      |
	|--------------|---------------|---------------|---------------|--------------|
	|    uboot     |     kernel    |     rootfs    |   a53_liteos  |   m7_liteos  |
	②烧写镜像：
	uboot：u-boot-hi3559av100.bin
	kernel：uImage_hi3559av100_big-little
	rootfs：rootfs_hi3559av100_2k_24bit.yaffs2
	a53_liteos：[ndk根目录]/sample/HuaweiLite/LiteOS.bin
	m7_liteos：没用到也必须烧录，可以暂用m7Liteos的sample镜像，make步骤如下：
			   在SDK发布包osdrv目录下执行make BOOT_MEDIA=spi AMP_TYPE=linux_liteos all  再进入platform/liteos_m7/liteos/sample/sample_osdrv目录执行make生成sample.bin
	③设置启动参数：
	setenv bootargs 'mem=512M console=ttyAMA0,115200 clk_ignore_unused rw root=/dev/mtdblock2 rootfstype=yaffs2 mtdparts=hinand:1M(boot),9M(kernel),32M(rootfs)'
	setenv bootcmd  'nand read 0x45000000 0x2A00000 0x1000000; go_a53up 0x45000000; config_m7; nand read 0x19000000 0x3A00000 0x100000; go_m7; nand read 0x52000000 0x100000 0x900000;bootm 0x52000000'
	saveenv
	reset
  5.加载ko驱动：
	insmod /komod/hi_ipcm.ko
	cd [SDK发布包路径]/mpp/out/linux/big-little/ko/
	./loadhi3559av100_bl -i
  6.查看a53 LiteOS端日志：
	①先在Linux端依次运行：
	ifconfig eth0 配板端ip
	telnetd &
	insmod /komod/hi_virt-tty.ko
	②然后任启一本地命令行依次运行：
	telnet 板端IP (用户名root 密码空)
	virt-tty a53 (小概率会报错找不到a53dev，要重启板子)

C.陀螺仪配置说明
    ①当前仅支持multi-core版本
	
    ②将i2c_bus11 的时钟频率从100000 改成 400000
	   修改文件路径：osdrv/opensource/kernel/linux-4.9.y_multi-core/arch/arm64/boot/dts/hisilicon/hi3559av100.dtsi
	   改成如下
	   i2c_bus11: i2c@1211b000 {
			compatible = "hisilicon,hibvt-i2c";
			reg = <0x1211b000 0x1000>;
			clocks = <&clock HI3559AV100_I2C11_CLK>;
			clock-frequency = <400000>;
			dmas = <&hiedmacv310_1 0 32>, <&hiedmacv310_1 1 33>;
			dma-names = "tx","rx";
			status = "disabled";
	    }
	修改完成后重新编译镜像，然后烧录新的镜像即可
	
	③修改source\ndk\config.mak  的DIS和GYRO配置项为：
	  CFG_SUPPORT_DIS ?= y
	  CFG_SUPPORT_GYRO ?= y
	
	④加头文件，当前SDK版本不发布以下四个头文件，但是NDK的编译依赖这四个头文件，因此如需调试验证，则需要手动拷贝这个四个头文件到
	    “sdk\mpp\out\linux\multi-core\include”路径下：
		motionsensor_exe_ext.h
		MotionSensor_ext.h
		MotionSensor_cmd.h
		motionsensor_exe.h
	  
	⑤修改ko加载脚本：
	  将“sdk\mpp\out\linux\multi-core\ko\load3559av100_multicore”的以下三个KO的注释去掉，然后再运行脚本
	  或者手动加载以下三个KO也可以
	  
	  insmod extdrv/motionsensor_chip.ko
	  insmod extdrv/motionsensor_mng.ko
	  insmod hi3559av100_motionfusion.ko
	  
	
