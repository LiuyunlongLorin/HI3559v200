#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "hi_type.h"

#include "libavformat/avformat.h"
#include "libavutil/avstring.h"
#include "libavutil/buffer.h"
#include "libavutil/mem.h"
#include "libavutil/timestamp.h"

#define SAMPLES_PER_FRAME (1024)
#define AUD_BITWIDTH (16)
//only support channel num 1 or 2
#define AUD_CHANNELS (2)
#define AUD_SAMPLERATE (48000)
#define PER_CHN_LEN (SAMPLES_PER_FRAME*(AUD_BITWIDTH/8))
#define PER_FRAME_LEN ((SAMPLES_PER_FRAME*(AUD_BITWIDTH/8))*AUD_CHANNELS)

static HI_U32 u32frameCnt = 0;
static FILE* pPcmfile = NULL;
static HI_U8 au8AudBuf[PER_FRAME_LEN] = {0};

static void print_help()
{
    printf("List all testtool command\n");
    printf("h             list all command we provide\n");
    printf("q             quit sample test\n");
    printf("1             start wav mux from read file\n");
}

static void log_packet(const AVFormatContext *fmt_ctx, const AVPacket *pkt)
{
    AVRational *time_base = &fmt_ctx->streams[pkt->stream_index]->time_base;

    printf("pts:%s pts_time:%s dts:%s dts_time:%s duration:%s duration_time:%s stream_index:%d\n",
           av_ts2str(pkt->pts), av_ts2timestr(pkt->pts, time_base),
           av_ts2str(pkt->dts), av_ts2timestr(pkt->dts, time_base),
           av_ts2str(pkt->duration), av_ts2timestr(pkt->duration, time_base),
           pkt->stream_index);
}

static HI_S32 write_frame(AVFormatContext *fmt_ctx, const AVRational *time_base, AVStream *st, AVPacket *pkt)
{
    av_packet_rescale_ts(pkt, *time_base, st->time_base);
    pkt->stream_index = st->index;

    log_packet(fmt_ctx, pkt);

    return av_interleaved_write_frame(fmt_ctx, pkt);
}

static HI_S32 readPacketFromBuffer(AVPacket* pstPkt)
{
    HI_U8 aszPcmBuf[AUD_CHANNELS][PER_CHN_LEN];
    HI_U8* pdstBuf = &au8AudBuf[0];

    //read from audio input to pcm buffer need be implement

    if(AUD_CHANNELS == 2)
    {
        //interleave two channel data
        int i = 0;
        int j = 0;
        for(i=0; i<PER_CHN_LEN;i++)
        {
            for(j=0; j<AUD_CHANNELS;j++)
            {
                *pdstBuf = aszPcmBuf[j][i];
                pdstBuf++;
            }
        }
    }

    u32frameCnt++;
    pstPkt->data = au8AudBuf;
    pstPkt->size = PER_FRAME_LEN;

    pstPkt->pts = SAMPLES_PER_FRAME*u32frameCnt;
    pstPkt->dts= pstPkt->pts;

    pstPkt->duration = SAMPLES_PER_FRAME;
    return HI_SUCCESS;
}

static HI_S32 readPacketFromFile(AVPacket* pstPkt)
{
    if(!pPcmfile)
    {
        pPcmfile = fopen("./test.pcm", "rb");
        if(!pPcmfile)
        {
            fprintf(stderr, "open pcm file failed\n");
            return HI_FAILURE;
        }
    }

    fread(au8AudBuf, 1, PER_FRAME_LEN, pPcmfile);
    if(feof(pPcmfile))
    {
        fprintf(stderr, "read pcm file end\n");
        return HI_FAILURE;
    }

    u32frameCnt++;
    pstPkt->data = au8AudBuf;
    pstPkt->size = PER_FRAME_LEN;

    pstPkt->pts = SAMPLES_PER_FRAME*u32frameCnt;
    pstPkt->dts= pstPkt->pts;

    pstPkt->duration = SAMPLES_PER_FRAME;
    return HI_SUCCESS;
}


void SAMPLE_WAV_Muxer()
{
    HI_CHAR* pOutFilename = "./test.wav";

    av_register_all();

    AVFormatContext *pOutCtx;
    AVStream *stream;

    avformat_alloc_output_context2(&pOutCtx, NULL, NULL, pOutFilename);
    if (!pOutCtx)
    {
        printf("Could not deduce output format from file extension: wav.\n");
        return;
    }


    stream = avformat_new_stream(pOutCtx, NULL);
    if (!stream)
    {
        fprintf(stderr, "Could not allocate stream\n");
        goto FreeCtx;
    }

    stream->id = pOutCtx->nb_streams-1;

    stream->codecpar->bits_per_coded_sample = AUD_BITWIDTH;
    stream->codecpar->bits_per_raw_sample = stream->codecpar->bits_per_coded_sample;

    stream->codecpar->channels = AUD_CHANNELS;
    stream->codecpar->sample_rate = AUD_SAMPLERATE;

    stream->codecpar->codec_type          = AVMEDIA_TYPE_AUDIO;
    stream->codecpar->codec_id            = AV_CODEC_ID_PCM_S16LE;
    stream->time_base = (AVRational){ 1, AUD_SAMPLERATE };

    //av_dump_format(pOutCtx, 0, pOutFilename, 1);

    int ret = 0;
    ret = avio_open(&pOutCtx->pb, pOutFilename, AVIO_FLAG_WRITE);
    if (ret < 0)
    {
        fprintf(stderr, "Could not open '%s': %s\n", pOutFilename,
                av_err2str(ret));
        goto FreeCtx;
    }

    /* Write the stream header, if any. */
    ret = avformat_write_header(pOutCtx, NULL);
    if (ret < 0) {
        fprintf(stderr, "Error occurred when opening output file: %s\n",
                av_err2str(ret));
        goto FreeAvio;
    }

    AVPacket pkt = { 0 }; // data and size must be 0;
    av_init_packet(&pkt);
    AVRational time_base = (AVRational){ 1, AUD_SAMPLERATE };

    while(1)
    {
        //could change read source from ai pcm buffer, but have to interleave
        //to S16LE rather than plannar eg:
        ret = readPacketFromFile(&pkt);
        if(ret != HI_SUCCESS)
        {
            break;
        }

        //write S16LE data
        ret = write_frame(pOutCtx, &time_base, stream, &pkt);
        if (ret < 0)
        {
            fprintf(stderr, "Error while writing audio frame: %s\n",
                    av_err2str(ret));
            break;
        }
    }
    if(pPcmfile != NULL)
    {
        fclose(pPcmfile);
        pPcmfile = NULL;
    }
    av_write_trailer(pOutCtx);

FreeAvio:
    avio_closep(&pOutCtx->pb);
FreeCtx:
    /* free the stream */
    avformat_free_context(pOutCtx);
}


#ifdef __HuaweiLite__
int app_main(int argc, char* argv[])
#else
int main(int argc, char* argv[])
#endif
{
    HI_CHAR   InputCmd[32];
    HI_S32 s32InputNumber = 0, i = 0;


    printf("please input 'h' to get help or 'q' to quit!\n");

    while (1)
    {
        memset(InputCmd, 0, 30);

        printf("<input cmd:>");
        if (NULL == fgets((char*)InputCmd, 30, stdin))
        {
            continue;
        }

        /*filter backspace key*/
        for (i = 0; InputCmd[i] != '\0' && InputCmd[i] != 0xa; i++);

        if (InputCmd[i] == 0xa) { InputCmd[i] = '\0'; }

        if ( (0 == strcmp("q", (char*)InputCmd))
             || (0 == strcmp("Q", (char*)InputCmd))
             || (0 == strcmp("quit", (char*)InputCmd))
             || (0 == strcmp("QUIT", (char*)InputCmd)) )
        {
            printf("quit the program!\n");
            break;
        }

        if ( (0 == strcmp("help", (char*)InputCmd)) ||
             (0 == strcmp("h", (char*)InputCmd) ))
        {
            print_help();
            continue;
        }

        s32InputNumber = atoi(InputCmd);

        printf("[%s.%d]\n", __FUNCTION__, __LINE__);

        switch (s32InputNumber)
        {

            case 1:/* sample avc muxer  */
                SAMPLE_WAV_Muxer();
                break;


            default:
            {
                printf("input cmd: %s is error \n", InputCmd);
                break;
            }
        }
    }

    return HI_SUCCESS;
}
