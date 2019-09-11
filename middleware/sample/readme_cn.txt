middleware sample 使用说明
1.sample目录结构说明
sample
├── common        从AAC、H264、H265 读取帧的库源码，供rtspserver,mp4muxer,tsmuxer,recorder_pro 使用
├── dcf           DCF文件管理sample           
├── dng_demuxer   dng解封装sample
├── dng_muxer     dng封装sample
├── dtcf          时间日期文件管理sample
├── eventhub      事件路由sample
├── exif          EXIF标签编辑和解封装sample
├── fstool        文件系统检查和格式化工具sample
├── higv          图形组件HIGV sample
├── hioscserver   OSC组件
├── liteos        liteos端sample，当调测litePlayer、higv以及mw_sample时需要在liteos测烧写该镜像
├── liteplayer    播放器组件LitePlayer解封装和播放的sample
├── livestream    rtspserver源码
├── Makefile      根目录sample
├── mp4_demuxer   mp4解封装sample
├── mp4_muxer     mp4封装sample
├── mw_sample     中间件sample，综合录像，rtspserver
├── oscserver      oscserver sample
├── readme_cn.txt  middleware sample说明
├── recorder_pro   录像组件sample，从ES流文件读取帧，然后录制到MP4文件中。
├── rtspserver     rtspserver组件sample
├── statemachine   层次状态机sample
├── storage        存储管理sample
├── ts_muxer       TS封装sample
├── wav_muxer      单音频wav封装sample
└── xmp            xmp组件sample

2. 中间件sample对系统诉求
依赖SD卡mount和unmount，要求系统加载相关的驱动。
依赖网络以及网络配置相关的驱动。
依赖nfs，需要内核支持nfs，用于mount。
图形组件sample运行需要加载屏幕相关的驱动。


3. 具体sample的执行参照相关sample内部的readme说明。

4. 对于Linux+liteos的系统，需要在liteos端烧写liteos下的bin文件，然后在Linux测执行各个组件的sample。

5. 对于单liteos的系统，直接烧写各个组件sample目录下的bin文件作为liteos的镜像。
