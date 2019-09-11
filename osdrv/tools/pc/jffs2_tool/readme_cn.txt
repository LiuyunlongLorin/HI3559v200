一、mtd-utils2.0.2依赖以下几个库，以ubuntu为例，安装方式请参考下面命令：
sudo apt-get install zlib1g-dev liblzo2-dev uuid-dev pkg-config

二、由于mtd-utils2.0.2通过pkg-config工具检查各个库是否正常安装，因此请参考如下方式设置pkg-config工具搜索路径：
export PKG_CONFIG_PATH="$PKG_CONFIG_PATH:/usr/lib/x86_64-linux-gnu/pkgconfig"

三、mtd-utils2.0.2依赖于automake1.15.1版本，请参考如下方式安装automake1.15.1版本：
	1、删除原有automake版本
	sudo apt-get autoremove automake
	2、下载1.15.1版本
		1）Windows系统中使用浏览器访问automake工程url并下载对应源码包：
		https://lists.gnu.org/archive/html/info-gnu/2017-06/msg00007.html
		2）linux系统中可以使用下面命令下载：
		wget ftp://ftp.gnu.org/gnu/automake/automake-1.15.1.tar.gz
	3、编译安装automake1.15.1
		tar zxvf automake-1.15.1.tar.gz
		cd automake-1.15.1/
		./configure
		make
		make install

四、编译mtd-utils2.0.2
	
