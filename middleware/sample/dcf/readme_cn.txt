DCF sample使用说明
说明：无
前置条件：
1)插入SD卡，且处于未mount状态
2)在单板根目录创建tmp目录（如果不存在的话），该目录用于挂在sd卡

运行步骤：
linux系统运行命令格式如下：
1、sample运行格式：./sample_dcf [-MODE] [-SET]；查看./dcf_sample -h可查看sample使用规则
liteos系统运行，烧写bin文件执行命令 sample [-MODE] [-SET]
2、举例：测试普通录像命名规则，板端执行命令 ./dcf_sample -R -NORMMP4；输入“new”则新建一个对象

运行结果：
查看相关打印。

