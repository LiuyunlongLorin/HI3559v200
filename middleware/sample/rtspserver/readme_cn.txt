rtspserver sample使用说明
说明：sample点播数据源是读文件。

前置条件
1.请在当前目录准备媒体数据源文件：1.h264 1.aac 2.h265 2.aac
   liteos系统：码流存放于sd卡下,并将sd卡插入单板。由于各芯片平台内存限制，请保证码流文件不要太大(推荐保持在1MB以内)，以免sample运行异常
   其他系统：码流文件存放于在当前目录
2. 为单板配置IP地址，且保证能够ping通可用。


运行步骤
Linux系统运行执行./sample_rtspserver
liteos系统运行执行sample命令
   输入 h  打印帮助信息如下
    List all testtool command
    h             list all command we provide
    q             quit sample test
    1             start one 264 stream and the rtspserver
    2             start one 265 stream and the rtspserver
	
执行 1 ：开启rtspserver ,可点播264码流， 点播url为 ： rtsp://ip:554/12
执行2：开启rtspserver ,可点播265码流, 点播url为 ： rtsp://ip:554/11


运行结果
请用PC端VLC点播工具点播url进行功能验证。