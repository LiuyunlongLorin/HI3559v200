本文档简要说明将内核的补丁打到v4.9.37的linux kernel上。

1、从linux开源社区下载v4.9.37版本的内核：
        1)进入网站：www.kernel.org
        2)选择HTTP协议资源的https://www.kernel.org/pub/选项,进入子页面
        3)选择linux/菜单项，进入子页面
        4)选择kernel/菜单项，进入子页面
        5)选择v4.x/菜单项，进入子页面
        6)下载linux-4.9.37.tar.gz（或linux-4.9.37.tar.xz）

2、打补丁
        1)将下载的 linux-4.9.37.tar.gz 存放到 osdrv的opensource/kernel目录中
		2)在linux服务器中进入 osdrv 的根目录,执行如下命令：
		cd opensource/kernel
		tar -zxf linux-4.9.37.tar.gz
		mv linux-4.9.37 linux-4.9.y
		cd linux-4.9.y
		patch -p1 < ../linux-4.9.37.patch
		cd ../
		tar -czf linux-4.9.y.tgz linux-4.9.y
		cd ../../
	或
		1)将下载的 linux-4.9.37.tar.gz 存放到 osdrv的opensource/kernel目录中
		2)在linux服务器中进入 osdrv 的根目录,执行如下命令：
			make hikernel

注意：
	若下载的内核格式为linux-4.9.37.tar.xz，
	第一步先用：
		xz -d linux-4.9.37.tar.xz
	命令将linux-4.9.37.tar.xz解压为linux-4.9.37.tar。
	第二步再用：
		tar -xvf linux-4.9.37.tar
	解压linux-4.9.37.tar。
