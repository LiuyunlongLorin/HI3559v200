exif sample使用说明
说明：

前置条件：
1)在单板根目录创建tmp目录（如果不存在的话），该目录用于挂载sd卡，sample生成的文件将会保存在tmp目录下
2)在单板可访问的路径下放置相应的JPEG文件。

运行步骤：
1）linux系统运行，进入到当前目录，在当前目录下输入命令启动sample
标签编辑执行命令：  ./sample_exif 0 xxx.jpeg
EXIF图片解封装命令: ./sample_exif 1 xxx.jpeg

2）liteos系统运行，烧写对应的bin文件
标签编辑执行命令：  sample 0 xxx.jpeg
EXIF图片解封装命令: sample 1 xxx.jpeg

运行结果
1）用例执行失败会有相应提示信息，否则为执行成功。
2）执行成功后。liteos系统生成的文件存放于sd卡根目录，其他系统则存放于当前目录
