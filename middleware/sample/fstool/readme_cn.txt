FSTOOL sample使用说明
说明：FSTOOL的sample由两个工具组成：hifsck.vfat 和 himkfs.vfat。

hifsck.vfat（文件系统检查工具）
前置条件：插入SD卡，且处于未mount状态

运行步骤：
1）Linux运行操作步骤：
./hifsck.vfat partition;
举例：
./hifsck.vfat /dev/mmcblk0p1;


2）Liteos运行操作步骤：
烧写bin文件，系统启动后执行sample partition;
举例：
sample /dev/mmcblk0p0;

运行结果：
输出sd卡文件系统检查结果；


himkfs.vfat（文件系统格式化工具）

前置条件：插入SD卡，且处于未mount状态

运行步骤：
1）Linux运行操作步骤：
./himkfs.vfat clusterSize(KB) mode partition;
举例：
./himkfs.vfat 64 -a /dev/mmcblk0p1;


2）Liteos运行操作步骤：
烧写bin文件，系统启动后执行sample clusterSize(KB) mode partition;
举例：
sample 64 -a /dev/mmcblk0p0;

运行结果
sd卡被格式化成簇大小为64KB的FAT32文件系统；