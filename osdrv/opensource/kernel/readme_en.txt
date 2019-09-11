This document provides a brief description of the kernel patch applied to
v4.9.37 from the linux kernel.

1, in the linux open source community to download v4.9.37 version of the kernel:
	1) Go to the website: www.kernel.org
	2) Select HTTP protocol resources https://www.kernel.org/pub/ option,
	enter the sub-page
	3) Select linux/ menu item, enter the sub-page
	4) Select the kernel/ menu item, enter the sub-page
	5) Select v4.x/ menu item, enter sub-page
	6) Download linux-4.9.37.tar.gz (or linux-4.9.37.tar.xz)

2, patching
	1) The linux-4.9.37.tar.gz stored to osdrv the directory 
	opensource/kernel
	2) Into the root directory osdrv In the linux server,
	execute the following command:
		cd opensource/kernel
		tar -zxf linux-4.9.37.tar.gz
		mv linux-4.9.37 linux-4.9.y
		cd linux-4.9.y
		patch -p1 < ../linux-4.9.37.patch
		cd ../
		tar -czf linux-4.9.y.tgz linux-4.9.y
		cd ../../
	or：
	1) The linux-4.9.37.tar.gz stored to osdrv the directory 
	opensource/kernel
	2) Into the root directory osdrv In the linux server,
	execute the following command:
	make hikernel

Notes：
       If choose the linux-4.9.37.tar.xz,
	Step 1, use the xz command to compress linux-4.9.37.tar.xz to
	linux-4.9.37.tar:
               xz -d linux-4.9.37.tar.xz
	Step 2, compress the linux-4.9.37.tar:
               tar -xvf linux-4.9.37.tar
