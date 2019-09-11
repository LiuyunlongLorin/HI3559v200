#####################################################################################################
#                                              Sample运行指南
#####################################################################################################

Widget Sample运行指导：

1. sample适用芯片类型
   Hi3559V200/Hi3556AV100

2. 触摸屏依赖
  默认支持240 * 320分辨率的触摸屏(竖屏)，其他分辨率触摸屏需要适配，适配请参考sample_utils.h文件，修改其中的屏幕及屏幕分辨率相关信息。

3. 加载驱动
（1）客户根据实际情况适配触摸屏驱动和LCD屏幕驱动程序，需要在系统启动后使用insmod命令加载, 驱动获取路径：middleware/media_adpt/hixxx/ft6236和middleware/media_adpt/hixxx/st7789
（2）依赖的其他驱动包含TDE, HIFB和VO的驱动KO文件，这些驱动单板启动后系统默认会加载，不需单独加载。

4. 适配字库
  sample默认使用矢量字库simhei.ttf，由于版权限制，发布包中不包含任何字库，如需演示sample到“C:\Windows\Fonts”目录获取，字库存在路径是：./res/font/ttf。

5. 编译触摸的Sample
（1）进入到middleware/sample/higv/widget_sample目录下面，执行make命令，编译生成widget_sample可执行文件。
（2）把widget_sample, higv.bin, res目录，拷贝到单板的某个位置
（3）使能可执行文件，chmod +x *
（4）执行widget_sample文件，即可看到屏幕有图像显示。

6. sample运行
  widget sample运行后，在首界面显示的是listbox控件，手指在屏幕上下拖动，listbox可跟随手指进行位移，表示sample已经运行成功。

#####################################################################################################

Scene Sample运行指导：

1. sample适用芯片类型
   Hi3559V200/Hi3556AV100

2. 触摸屏依赖
  默认支持240 * 320分辨率的触摸屏(竖屏)，其他分辨率触摸屏需要适配，适配请参考sample_utils.h文件，修改其中的屏幕及屏幕分辨率相关信息。

3. 加载驱动
（1）客户根据实际情况适配触摸屏驱动和LCD屏幕驱动程序，需要在系统启动后使用insmod命令加载, 驱动获取路径：middleware/media_adpt/hixxx/ft6236和middleware/media_adpt/hixxx/st7789
（2）依赖的其他驱动包含TDE, HIFB和VO的驱动KO文件，这些驱动单板启动后系统默认会加载，不需单独加载。

4. 适配字库
  sample默认使用矢量字库simhei.ttf，由于版权限制，发布包中不包含任何字库，如需演示sample到“C:\Windows\Fonts”目录获取，字库存在路径是：./res/font/ttf。

5. 编译触摸的Sample
（1）进入到middleware/sample/higv/scene_sample目录下面，执行make命令，编译生成scene_sample可执行文件。
（2）把scene_sample, higv.bin, res目录，拷贝到单板的某个位置
（3）使能可执行文件，chmod +x *
（4）执行scene_sample文件，即可看到屏幕有图像显示。

6. sample运行
  scene sample运行后，在首界面显示的是listbox控件，手指在屏幕上下拖动，listbox可跟随手指进行位移，表示sample已经运行成功。

#####################################################################################################

Auto Sample运行指导：

1. sample适用芯片类型
   Hi3516CV300/Hi3518EV300

2. LCD屏依赖(不支持触摸功能)
  默认支持320 * 240分辨率的触摸屏(横屏)，其他分辨率触摸屏需要适配，适配请参考sample_hifb.h文件，修改其中的屏幕及屏幕分辨率相关信息。

3. 加载驱动
   加载TDE, HIFB和VO的驱动KO文件。

4. 适配字库
  sample默认使用矢量字库simhei.ttf，由于版权限制，发布包中不包含任何字库，如需演示sample到“C:\Windows\Fonts”目录获取，字库存在路径是：./res/font/ttf。

5. 编译触摸的Sample
（1）进入到middleware/sample/higv/auto_sample目录下面，执行make命令，编译生成auto_sample.bin文件。
（2）使用tftp工具烧写auto_sample.bin文件到单板。
（3）单板新建home/test目录(liteos仅支持设置绝对路径，sample的资源文件都使用home/test为根路径)，挂载auto_sample文件夹到home/test目录，建议使用SD卡。
（4）在串口运行sample命令，即可看到屏幕有图像显示。

6. sample运行
  auto sample运行后，可以看到控件界面在自动切换的情况，首页面是button控件显示。
#####################################################################################################

3559a Sample运行指导：

1. sample适用芯片类型
   Hi3559AV100

2. 输出显示依赖(不支持触摸功能)
  默认支持1920 * 1080分辨率的HDMI输出，其他分辨率屏幕需要适配，适配请参考sample_hifb.h文件，修改其中的屏幕及屏幕分辨率相关信息。

3. 加载驱动
   加载TDE, HIFB和VO的驱动KO文件。

4. 适配字库
  sample默认使用矢量字库simhei.ttf，由于版权限制，发布包中不包含任何字库，如需演示sample到“C:\Windows\Fonts”目录获取，字库存在路径是：./res/font/ttf。

5. 编译触摸的Sample
（1）进入到middleware/sample/higv/3559a_sample目录下面，执行make命令，编译生成3559a_sample可执行文件。
（2）把3559a_sample, higv.bin, res目录，拷贝到单板的某个位置
（3）使能可执行文件，chmod +x *
（4）执行3559a_sample文件，即可看到屏幕有图像显示。

6. sample运行
  3559a sample运行后，可以看到控件界面在自动切换的情况。
