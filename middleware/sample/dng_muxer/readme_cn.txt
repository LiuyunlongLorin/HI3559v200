dng_muxer 使用说明
说明：无

前置条件：
1）sample中封装所用数据源采用读文件方式，请准备一个test.raw thm.jpg screennail.jpg在当前目录下
；如果liteos准备数据，请用nfs mount后，将原始文件放到home目录下。
2）如果实际图片和sample中所需图片属性参数有差异，请根据图片实际情况修改sample中图片属性参数配置。

运行步骤
1）Linux系统上运行执行./sample_dngmuxer
   Liteos系统运行执行 sample

2）输入 h  打印帮助信息如下：
List all testtool command
h             list all command we provide
q             quit sample test
1             dng single photo sample with thm and screennail
2             dng multi photo sample
3             dng with no thm

执行 1 :封装一张 带有thm 和screennail 以及一张raw的 dng图片
执行2: dng图片多拍业务（带有thm 和screennail）
执行3: 封装一张没有缩略图的dng文件

运行结果：
用例执行失败会有相应提示信息，否则为执行成功。
请查看当前目录下生成的dng文件，是否可以用DNG解析工具（lightroom photoshop dngCodec等）查看，若工具无法解析也表示执行失败。