mp4muxer sample使用说明
说明：
前置条件：
mp4封装sample的编译运行依赖mp4封装解封装组件库libmp4 libfileformat
sample中封装所用数据源采用读文件方式，请准备以下文件：

1、在单板根目录创建tmp目录（如果不存在的话），该目录用于挂在sd卡；
2、码流文件：1.h264 1.aac 2.h264 1.h265 apco.prores准备：
   liteos系统：码流存放于sd的下。由于各芯片平台内存限制，请保证码流文件不要太大(推荐保持在1MB以内)，以免sample运行异常
   其他系统：码流文件存放于在当前目录

运行步骤：
linux/linux_liteos系统：执行./sample_mp4_muxer
liteos系统：执行 sample
   输入 h  打印帮助信息如下

    List all testtool command\n
    h             list all command we provide
    q             quit sample test
    1             mux h264 stream into mp4 file
    2             mux h264 stream into mp4 file
    3             mux h264 stream and h265 stream in one mp4 file
    4             mux h264 main stream and minor stream in one mp4 file
    5             mux prores stream into mp4 file
    6             repair broken mp4 file


执行1，封装h264码流的MP4文件；
执行2，封装h265码流的MP4文件；
执行3，将h264和h265码流封装在同一个MP4文件中；
执行4，将h264大小码流封装在同一个MP4文件中；
执行5，封装prores码流的MP4文件
执行6，修复损坏的MP4文件，需要前置条件：准备损坏的MP4文件（如录像过程中拔出SD卡，生成损坏MP4文件；也可通过执行脚本getRepairFiles.sh生成损坏MP4文件）。

运行结果：
用例执行失败会有相应提示信息，否则为执行成功。
请查看sd卡下生成的MP4文件，是否可以用mp4解析工具查看，若工具无法解析也表示执行失败。