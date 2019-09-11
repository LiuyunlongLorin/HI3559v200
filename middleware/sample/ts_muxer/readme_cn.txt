ts_muxer sample 使用说明
说明：无
前置条件：
1）预分配功能依赖SD环境；
2）在单板根目录创建tmp；
3）依赖的相关资源文件如下：
1.aac 1.h264  2.aac 2.h265 以上都是ES数据，用户需要自己准备，由于各芯片平台内存限制，请保证码流文件不要太大(推荐保持在1MB以内)，以免sample运行异常。

linux系统 ：将资源文件准备在当前目录；
liteos系统：将资源文件准备在"/tmp/sample"目录

运行步骤：
输入命令行启动sample
linux/linux_liteos系统 ：./sample_ts_muxer
liteos系统：sample
  打印提示信息  
  please input 'h' to get help or 'q' to quit!
  <input cmd:>
3、根据命令行提示输入'h'
List all testtool command
h             list all command we provide
q             quit sample test
1             start h264 record from read file
2             start h265 record from read file
3             write no frame sample
<input cmd:>

选择'1'
  封装出的文件test_avc.ts在PC机上可以播放。
选择‘2’
  封装出的文件test_hevc.ts在PC机上可以播放
选择‘3’
  封装没有数据的ts文件test_avc_no_frame.ts，不可播放。
  
运行结果：
用例执行失败会有相应提示信息，否则为执行成功。
请查看sd卡sample目录下生成的ts文件，是否可以用ts解析工具查看播放，若工具无法解析也表示执行失败。
