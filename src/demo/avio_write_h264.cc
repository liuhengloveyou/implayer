#include <stdio.h>
#include <stdlib.h>


#ifdef __cplusplus
extern "C"
{
#endif

#define __STDC_CONSTANT_MACROS

#include <libavutil/avutil.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavformat/avio.h>
#include <libavutil/file.h>

#ifdef __cplusplus
}
#endif

typedef struct {
    FILE* fp;
} OutputContext;

static int write_packet(void* opaque, uint8_t* buf, int buf_size)
{
    OutputContext* output_ctx = (OutputContext*)opaque;
    FILE* fp = output_ctx->fp;

    fwrite(buf, 1, buf_size, fp);

    return buf_size;
}

int avio_write_h264(const char *input_url, const char *output_file)
{
    AVFormatContext* input_ctx = NULL;
    const AVOutputFormat* output_fmt = NULL;
    AVFormatContext* output_ctx = NULL;
    OutputContext* output_opaque = NULL;

    int ret = avformat_open_input(&input_ctx, input_url, NULL, NULL);
    if (ret < 0) {
        fprintf(stderr, "Error: Could not open input file: %d.\n", (ret));
        goto end;
    }

    ret = avformat_find_stream_info(input_ctx, NULL);
    if (ret < 0) {
        fprintf(stderr, "Error: Could not find stream information: %d.\n", (ret));
        goto end;
    }

    output_fmt = av_guess_format("h264", NULL, NULL);
    if (!output_fmt) {
        fprintf(stderr, "Error: Could not guess output format.\n");
        ret = AVERROR_MUXER_NOT_FOUND;
        goto end;
    }

    ret = avformat_alloc_output_context2(&output_ctx, output_fmt, NULL, output_file);
    if (ret < 0) {
        fprintf(stderr, "Error: Could not allocate output context: %d.\n", (ret));
        goto end;
    }

    AVStream* in_video_stream = NULL;
    AVCodecParameters* in_codec_params = NULL;
    for (int i = 0; i < input_ctx->nb_streams; i++) {
        AVStream* stream = input_ctx->streams[i];
        if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            in_video_stream = stream;
            in_codec_params = stream->codecpar;
            break;
        }
    }
    if (!in_video_stream) {
        fprintf(stderr, "Error: Could not find video stream.\n");
        ret = AVERROR(ENOSYS);
        goto end;
    }

    AVStream* out_video_stream = avformat_new_stream(output_ctx, NULL);
    if (!out_video_stream) {
        fprintf(stderr, "Error: Could not create new stream.\n");
        ret = AVERROR(ENOMEM);
        goto end;
    }

    ret = avcodec_parameters_copy(out_video_stream->codecpar, in_codec_params);
    if (ret < 0) {
        fprintf(stderr, "Error: Could not copy codec parameters: %d.\n", (ret));
        goto end;
    }

    out_video_stream->codecpar->codec_tag = 0;

    output_opaque = (OutputContext *)av_malloc(sizeof(OutputContext));
    if (!output_opaque) {
        fprintf(stderr, "Error: Could not allocate output context.\n");
        ret = AVERROR(ENOMEM);
        goto end;
    }

    fopen_s(&output_opaque->fp, output_file, "wb");
    if (!output_opaque->fp) {
        fprintf(stderr, "Error: Could not open output file.\n");
        ret = AVERROR(ENOENT);
        goto end;
    }

    AVIOContext* pb = NULL;
    pb = avio_alloc_context((unsigned char*)av_malloc(32768), 32768, 1, output_opaque, NULL, &write_packet, NULL);
    if (!pb) {
        fprintf(stderr, "Error: Could not allocate output IO context.\n");
        ret = AVERROR(ENOMEM);
        goto end;
    }
    output_ctx->pb = pb;

    ret = avformat_write_header(output_ctx, NULL);
    if (ret < 0) {
        fprintf(stderr, "Error: Could not write header: %d.\n", (ret));
        goto end;
    }

    AVPacket packet = { 0 };
    while (av_read_frame(input_ctx, &packet) >= 0) {
        if (packet.stream_index == in_video_stream->index) {
            av_packet_rescale_ts(&packet, in_video_stream->time_base, out_video_stream->time_base);
            packet.stream_index = out_video_stream->index;

            ret = av_interleaved_write_frame(output_ctx, &packet);
            if (ret < 0) {
                fprintf(stderr, "Error: Could not write frame: %d.\n", (ret));
                goto end;
            }
        }

        av_packet_unref(&packet);
    }

    ret = av_write_trailer(output_ctx);
    if (ret < 0) {
        fprintf(stderr, "Error: Could not write trailer: %d.\n", (ret));
        goto end;
    }

    printf("Conversion complete!\n");

end:
    if (input_ctx) {
        avformat_close_input(&input_ctx);
    }
    if (output_ctx) {
        if (output_ctx->pb) {
            av_freep(&output_ctx->pb->buffer);
            avio_context_free(&output_ctx->pb);
        }
        if (output_opaque->fp) {
            fclose(output_opaque->fp);
        }
        avformat_free_context(output_ctx);
        av_free(output_opaque);
    }
    return ret;
}