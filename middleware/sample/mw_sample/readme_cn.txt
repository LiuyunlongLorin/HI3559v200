mw_sample使用说明
说明：是通过适时码流的方式，启动录制功能，rtspserver功能。
前置条件：
        1) liteos测需要烧写中间件liteos sample镜像，且确保启动成功。
        2）需要加载sd卡驱动，加载网络相关的驱动，并配置网络。
        3）需要单板安装默认适配过的sensor，如果需要适配其他sensor，请查看/middleware/media_adpt/hi35xx中相关代码。
        4）插入可用的sd卡。

运行步骤：
        Linux系统运行sample
        1）执行./mw_sample -p 只启动录像
        2）./mw_sample -p -s 554 启动录像和rtspserver.
        liteos系统运行sample
        烧写bin文件，输入sample命令开始录制。

 运行结果：
       会录制码流到sd卡上，用vlc播放验证即可。