sample_recorder_pro 使用说明
说明：recorder_pro sample是通过读取h264/aac文件中帧的方式，完成录制功能。
前置条件：
        1) 准备两个裸码流文件mainstream.h264、slavestream.h264，文件分辨率为1024*576，时长大概10s左右（总大小2M左右）,码流文件大小需要考虑系统可用内存。如果是其他分辨率的码流，请修改sample代码中配置参数。若出现运行不起来或者报错内存申请失败，请更换为更小的码流文件。
        2) 裸码流文件放置。
        Linux系统上运行将裸码流文件放在sample同级目录下。
        liteos系统上运行需要通过nfs将服务器mount到单板的home目录下，同时将裸码流放到单板home目录下。
        3）如果开启录制音频，需要准备mainstream.aac、slavestream.aac，采样率48K,位宽16bit，同时将录制的stRecAttr.astStreamAttr[u32StreamIdx].u32TrackCnt 改为2.
        4）在单板插入一张可用的sd卡。
运行步骤：
        1）创建录像
        Linux系统上运行程序执行./sample_recorder_pro 创建录像。
        liteos系统上运行，请烧写sample_recorder.bin，烧写成功启动后，输入sample命令创建录像。
        2）执行s，启动录像
        3）执行p，关闭录像

运行结果：
        录制后的码流会录制到sd卡上，用vlc播放验证即可。