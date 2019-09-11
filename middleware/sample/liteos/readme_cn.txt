liteos测sample使用说明
说明：该sample仅仅用做系统架构为Linux+liteos时liteos侧的sample
1. 当系统架构为Linux+liteos时使用该sample作为中间件在liteos的sample。
2. sample编译生成的media_app.bin，对应的压缩镜像为media_app_zip.bin。
3. 当前sample只适配了一种sensor和一种内存配置，如果有sensor类型、内存配置等与默认配置不一致，
   请修改../media_adpt/hi35xx/liteos 文件夹下的sdk_init.c中sensor以及内存配置，并重新编译
   media_adpt目录。然后再编译sample。
4. 烧写到单板上之后，请通过串口确保liteos端是正常启动的，如果非正常启动一般都是内存配置有误或者启动参数有误。
5. 请确保单板安装了和默认配置一致的sensor，保证媒体数据能够正常采集。

6. 不同芯片默认配置说明
SDK软件包Hi3559V200_MobileCam_SDK_xxx支持的config为hi3559v200_actioncam_demb_imx458_512M_config，即sensor为imx458，内存为512版本。