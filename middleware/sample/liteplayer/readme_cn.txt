LitePlayer sample使用说明

一、player_sample 使用说明
1. Linux+Liteos系统player_sample使用说明：
    1). 需要将liteos测的镜像烧写为sample/liteos/media_app.bin(对应的压缩镜像为media_app_zip.bin)
        
    2).在单板执行sharefs&（仅执行一次),如果板端不存在该sharefs可执行文件，请从osdrv目录中寻找sharefs可执行程序，并在单板执行。
    （例如Hi3559v200的sharefs可执行程序位于osdrv/components/ipcm/ipcm/out/node_0路径下）
    3). 执行拷贝操作，把middleware/sample/liteplayer/player_sample/AAC/libhiaacdec.so拷贝到单板的/app/sharefs/下，
    如果没有sharefs目录，先执行mkdir /app/sharefs
    4). 将middleware/component/liteplayer/lib/目录下的 libffmpeg_demuxer.so 拷贝至单板的/usr/lib/下面
    5). 执行middleware/sample/liteplayer/player_sample下生成的sample_play
            格式：./sample_play test.mp4
    6).sample运行过程中当前支持的命令：quit, pause(pu), play(pl), seek(sk), ff [speed] , bw [speed]
            其中 ff [speed] 例如： ff 32 表示以32倍速快进
            bw [speed] 例如： bw 32 表示以32倍速快退

2. 单liteos系统player_sample使用说明
    1). 在middleware/sample/liteplayer/player_sample下执行make
    2). 生成的middleware/sample/liteplayer/player_sample/sample_play.bin进行烧写
    3). middleware/sample/liteplayer/player_sample/so下的动态库拷贝到
        liteos系统/home/目录下(如果/home不存在，可nfs挂载，或者修改player_sample.c中
        的三个动态库路径)
    4). 执行 sample /sdcard_dir/filename.mp4
        (文件路径请挂载sd卡后，确认sd卡上有该文件)
    5). 输入quit可退出程序运行

3. 当前player sample的输出默认通过HDMI/AV线在电视机显示，如果想通过其他输出，请通过修改sample和media_adpt\hi35xx修改输出通路。

二、demuxer_sample 使用说明
通过./sample_demux filename，将mp4文件解封装为音频和视频ES流。