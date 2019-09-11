wav_muxer sample需要依赖于输入的音频PCM文件；

请使用者提供48k采样，16bit位深，2channel的PCM文件，命名为test.pcm
,2channel的数据使用interleave方式存储,放置在wav_muxer目录下

或者请对应修改sample以适应输入的PCM文件格式

运行sample命令如下：
./sample_wav_mux

运行后会在当前目录下生成./test.wav文件
