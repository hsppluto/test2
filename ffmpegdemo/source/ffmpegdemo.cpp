#include <stdio.h>

#define __STDC_CONSTANT_MACROS

#ifdef _WIN32
//Windows
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavdevice/avdevice.h"
#include "libavformat/avformat.h"
#include "libavfilter/avfilter.h"
#include "libavutil/avutil.h"
#include "libswscale/swscale.h"
//#include "SDL2/SDL.h"
};
#else
//Linux...
#ifdef __cplusplus
extern "C"
{
#endif
#include <libavcodec/avcodec.h>
#include <libavdevice/avdevice.h>
#include <libavformat/avformat.h>
#include <libavfilter/avfilter.h>
#include <libavutil/avutil.h>
#include <libswscale/swscale.h>
#include <SDL2/SDL.h>
#ifdef __cplusplus
};
#endif
#endif


AVFormatContext *i_fmt_ctx;
AVStream *i_video_stream;
AVStream *i_audio_stream;

AVFormatContext *o_fmt_ctx;
AVStream *o_video_stream;
AVStream *o_audio_stream;

int out_audio_index = -1;
int out_video_index = -1;


int main(int argc, char **argv)
{
	avcodec_register_all();
	av_register_all();
	avformat_network_init();

	char cName[256] = "a.MP4";
	//CLS_H264toMP4 clsH264;
	//clsH264.OpenOutput(cName, sizeof(cName));

    /* should set to NULL so that avformat_open_input() allocate a new one */
    i_fmt_ctx = NULL;

	i_fmt_ctx = avformat_alloc_context();  //-add
	if (!i_fmt_ctx)
	{
		return -1;
	}

	i_fmt_ctx->flags |= AVFMT_FLAG_NONBLOCK;


    char rtspUrl[] = "rtsp://192.168.3.21:554";
    //char rtspUrl[] = "rtsp://192.168.1.124:554/Streaming/Channels/1";
    const char *filename = "1.MP4";
    if (avformat_open_input(&i_fmt_ctx, rtspUrl, NULL, NULL)!=0)
    {
        fprintf(stderr, "could not open input file\n");
        return -1;
    }

    if (avformat_find_stream_info(i_fmt_ctx, NULL)<0)
    {
        fprintf(stderr, "could not find stream info\n");
        return -1;
    }


    /* find first video stream */
    for (unsigned i=0; i<i_fmt_ctx->nb_streams; i++)
    {
        if (i_fmt_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            i_video_stream = i_fmt_ctx->streams[i];
            printf("video streams nb_streams = %d,i = %d.\n", i_fmt_ctx->nb_streams,i);
            break;
        }
    }

    for (unsigned i=0; i<i_fmt_ctx->nb_streams; i++)
	{
    	if (i_fmt_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
		{
			i_audio_stream = i_fmt_ctx->streams[i];
			printf("audio streams nb_streams = %d,i = %d.\n", i_fmt_ctx->nb_streams,i);
			break;
		}
	}

   if (i_video_stream == NULL)
    {
        fprintf(stderr, "didn't find any video stream\n");
        return -1;
    }
   if (i_audio_stream == NULL)
   {
	   fprintf(stderr, "didn't find any audio stream\n");
	   //return -1;
   }

    avformat_alloc_output_context2(&o_fmt_ctx, NULL, NULL, filename);
    if (!o_fmt_ctx)
    {
           printf( "Could not create output context\n");
           return -1;
     }
    printf("------------------------------------------------------\n");

	if (i_audio_stream != NULL)
	{
		//创建音频输出流
		AVCodec *codec = avcodec_find_encoder(CODEC_ID_AAC);
		o_audio_stream = avformat_new_stream(o_fmt_ctx, codec);
		o_audio_stream->id = o_fmt_ctx->nb_streams-1;
		out_audio_index = o_fmt_ctx->nb_streams-1;
		printf("o_fmt_ctx->nb_streams = %d, o_audio_stream->id = %d.\n", o_fmt_ctx->nb_streams, o_audio_stream->id);
		{
		    AVCodecContext *c;
		    c = o_audio_stream->codec;
		    c->sample_fmt  = (codec)->sample_fmts ?
		                (codec)->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
		            c->bit_rate    = 64000;//比特率
		            c->sample_rate = 44100;//采样率
		            if ((codec)->supported_samplerates) {
		                c->sample_rate = (codec)->supported_samplerates[0];
		                for (int i = 0; (codec)->supported_samplerates[i]; i++) {
		                    if ((codec)->supported_samplerates[i] == 44100)
		                        c->sample_rate = 44100;
		                }
		            }
		            c->channels        = av_get_channel_layout_nb_channels(c->channel_layout);
		            c->channel_layout = AV_CH_LAYOUT_STEREO;
		            if ((codec)->channel_layouts) {
		                c->channel_layout = (codec)->channel_layouts[0];
		                for (int i = 0; (codec)->channel_layouts[i]; i++) {
		                    if ((codec)->channel_layouts[i] == AV_CH_LAYOUT_STEREO)
		                        c->channel_layout = AV_CH_LAYOUT_STEREO;
		                }
		            }
		            c->channels        = av_get_channel_layout_nb_channels(c->channel_layout);
		            //o_audio_stream->time_base = (AVRational){ 1, c->sample_rate };
					o_audio_stream->time_base.num = 1;
					o_audio_stream->time_base.den = c->sample_rate;
		}

		//根据输入流创建输出流
//		o_audio_stream = avformat_new_stream(o_fmt_ctx, i_audio_stream->codec->codec);
//		if (avcodec_copy_context(o_audio_stream->codec, i_audio_stream->codec) < 0)
//		{
//			printf( "Failed to copy context from input to output stream codec context\n");
//		}

//		o_audio_stream->codec->codec_id = AV_CODEC_ID_AAC;
//		o_audio_stream->codec->codec_type = AVMEDIA_TYPE_AUDIO;

	}

	if (i_video_stream != NULL)
	{
		//根据输入流创建输出流
	   o_video_stream = avformat_new_stream(o_fmt_ctx, i_video_stream->codec->codec);
	   o_video_stream->id = o_fmt_ctx->nb_streams-1;
	   out_video_index = o_fmt_ctx->nb_streams-1;
	   printf("o_fmt_ctx->nb_streams = %d, o_video_stream->id = %d.\n", o_fmt_ctx->nb_streams, o_video_stream->id);

		if (avcodec_copy_context(o_video_stream->codec, i_video_stream->codec) < 0)
		{
			printf( "Failed to copy context from input to output stream codec context\n");
		}
		o_video_stream->codec->codec_tag = 0;
		if (o_fmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
		{
			o_video_stream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
		}
		{
			AVCodecContext *c;
			c = o_video_stream->codec;
			c->bit_rate = 400000;
			c->codec_id = i_video_stream->codec->codec_id;//AV_CODEC_ID_H264
			c->codec_type = i_video_stream->codec->codec_type;//AVMEDIA_TYPE_VIDEO
			printf("codec_id = %d, codec_type = %d\n", c->codec_id, c->codec_type);

			c->time_base.num = i_video_stream->time_base.num;	//1
			c->time_base.den = i_video_stream->time_base.den;	//90000
			printf("time_base.num = %d, time_base.den = %d\n", c->time_base.num, c->time_base.den);

			c->width = i_video_stream->codec->width;
			c->height = i_video_stream->codec->height;
			c->pix_fmt = i_video_stream->codec->pix_fmt;	//AV_PIX_FMT_YUV420P
			printf("width = %d, height = %d, pix_fmt = %d\n", c->width, c->height, c->pix_fmt);

			c->flags = i_video_stream->codec->flags;	//0
			c->flags |= CODEC_FLAG_GLOBAL_HEADER;
			printf("flags = %d\n", i_video_stream->codec->flags);

			c->me_range = i_video_stream->codec->me_range;	//0
			c->max_qdiff = i_video_stream->codec->max_qdiff;	//3
			printf("me_range = %d, max_qdiff = %d\n", c->me_range, c->max_qdiff);

			c->qmin = i_video_stream->codec->qmin;	//2
			c->qmax = i_video_stream->codec->qmax;	//31
			printf("qmin = %d, qmax = %d\n", c->qmin, c->qmax);

			c->qcompress = i_video_stream->codec->qcompress;	//0.5
			printf("qcompress = %f\n", c->qcompress);
		}
	}


    avio_open(&o_fmt_ctx->pb, filename, AVIO_FLAG_WRITE);
    avformat_write_header(o_fmt_ctx, NULL);

	int64_t last_pts = 0;
	int64_t last_dts = 0;

	int64_t pts = 0, dts = 0;
	int num = 1;
   while (1)
    {
		AVPacket i_pkt;
		av_init_packet(&i_pkt);
		i_pkt.size = 0;
		i_pkt.data = NULL;
		if (av_read_frame(i_fmt_ctx, &i_pkt) <0 )
		{
			break;
		}
		printf("size = %d, stream_index = %d, type = %d\n", i_pkt.size, i_pkt.stream_index, i_fmt_ctx->streams[i_pkt.stream_index]->codec->codec_type);

//		if(i_fmt_ctx->streams[i_pkt.stream_index]->codec->codec_type)
//		{
//			printf("i_pkt.pts = %lld\n", i_pkt.pts);
//		}
//		if(i_fmt_ctx->streams[i_pkt.stream_index]->codec->codec_type)
//		{
//			writefile((char*)i_pkt.data, i_pkt.size);
//		}

		//clsH264.PutAVStream(i_pkt.data, i_pkt.size, i_fmt_ctx->streams[i_pkt.stream_index]->codec->codec_type);

        /*
        * pts and dts should increase monotonically
        * pts should be >= dts
        */
        i_pkt.flags |= AV_PKT_FLAG_KEY;
//        pts = i_pkt.pts;
//        i_pkt.pts += last_pts;
//        dts = i_pkt.dts;
//        i_pkt.dts += last_dts;
        //i_pkt.stream_index = 0;

        //printf("%lld %lld\n", i_pkt.pts, i_pkt.dts);

        //printf("frame %d\n", num);
        num++;
        av_interleaved_write_frame(o_fmt_ctx, &i_pkt);

        //av_free_packet(&i_pkt);
        //av_init_packet(&i_pkt);

		if(num > 1000)
		{
			break;
		}
    }
    last_dts += dts;
    last_pts += pts;

    //clsH264.CloseOutput();

    avformat_close_input(&i_fmt_ctx);

    av_write_trailer(o_fmt_ctx);

	if(-1 != out_audio_index)
	{
		avcodec_close(o_fmt_ctx->streams[out_audio_index]->codec);
		av_freep(&o_fmt_ctx->streams[out_audio_index]->codec);
		av_freep(&o_fmt_ctx->streams[out_audio_index]);
	}

	if(-1 != out_video_index)
	{
		avcodec_close(o_fmt_ctx->streams[out_video_index]->codec);
		av_freep(&o_fmt_ctx->streams[out_video_index]->codec);
		av_freep(&o_fmt_ctx->streams[out_video_index]);
	}



    avio_close(o_fmt_ctx->pb);
    av_free(o_fmt_ctx);

    return 0;
}