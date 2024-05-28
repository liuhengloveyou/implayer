#include <queue>
#include <stdio.h>
#include <thread>
#include <memory>
#include <chrono>

#ifdef __cplusplus
extern "C"
{
#endif

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavformat/avio.h>
#include <libavutil/file.h>
#include "SDL.h"

#ifdef __cplusplus
}
#endif

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/bind.h>
#endif

#include "core/player.h"
#include "source/emscripten_websocket.h"

using namespace implayer;

IMPlayerSharedPtr player = nullptr;

int avio_write_h264(const char *input_url, const char *output_file);
int avio_main(int argc, char *argv[]);

int main(int argc, char *argv[])
{
#ifdef __EMSCRIPTEN__
    std::string in_file = "/input.mp4";
#else
    std::string in_file = "D:/input.mp4";
#endif

    
    return avio_write_h264("D:/input.mp4", "D:/1.h264");
    return avio_main(argc, argv);

    player = std::make_shared<IMplayer>();
    int ret = player->open(player, in_file);
    printf("open player %d\n", ret);

    player->play();

    return 0;
}

#ifdef __EMSCRIPTEN__
int wasm_onPlay()
{
    player->play();

    // SDL_Event event;
    // event.type = SDL_EVENT_PLAY;
    // SDL_PushEvent(&event);

    return 0;
}

EMSCRIPTEN_BINDINGS(my_module)
{
    emscripten::function("wasm_onPlay", &wasm_onPlay);
}
#endif

struct buffer_data
{
    uint8_t *ptr;
    size_t size; ///< size left in the buffer
};

static int read_packet(void *opaque, uint8_t *buf, int buf_size)
{
    struct buffer_data *bd = (struct buffer_data *)opaque;
    printf("ptr:%p size:%zu %d\n", bd->ptr, bd->size, buf_size);

    buf_size = FFMIN(buf_size, bd->size);

    if (!buf_size)
        return AVERROR_EOF;

    // /* copy internal buffer data to buf */
    memcpy(buf, bd->ptr, buf_size);
    bd->ptr += buf_size;
    bd->size -= buf_size;

    return buf_size;
}

int avio_main(int argc, char *argv[])
{
    AVFormatContext *fmt_ctx = NULL;
    AVIOContext *avio_ctx = NULL;
    uint8_t *buffer = NULL, *avio_ctx_buffer = NULL;
    size_t buffer_size, avio_ctx_buffer_size = 4096;
    char *input_filename = NULL;
    int ret = 0;
    struct buffer_data bd = {0};

    if (argc != 2)
    {
        fprintf(stderr, "usage: %s input_file\n"
                        "API example program to show how to read from a custom buffer "
                        "accessed through AVIOContext.\n",
                argv[0]);
        return 1;
    }
    input_filename = argv[1];

    /* slurp file content into buffer */
    ret = av_file_map(input_filename, &buffer, &buffer_size, 0, NULL);
    if (ret < 0)
        goto end;
    printf(">>>>>>>>>>>>%d\n", buffer_size);

    /* fill opaque structure used by the AVIOContext read callback */
    bd.ptr = buffer;
    bd.size = buffer_size;

    if (!(fmt_ctx = avformat_alloc_context()))
    {
        ret = AVERROR(ENOMEM);
        goto end;
    }

    avio_ctx_buffer = (uint8_t *)av_malloc(avio_ctx_buffer_size);
    if (!avio_ctx_buffer)
    {
        ret = AVERROR(ENOMEM);
        goto end;
    }
    avio_ctx = avio_alloc_context(avio_ctx_buffer, avio_ctx_buffer_size,
                                  0, &bd, &read_packet, NULL, NULL);
    if (!avio_ctx)
    {
        ret = AVERROR(ENOMEM);
        goto end;
    }
    fmt_ctx->pb = avio_ctx;

    ret = avformat_open_input(&fmt_ctx, NULL, NULL, NULL);
    if (ret < 0)
    {
        fprintf(stderr, "Could not open input\n");
        goto end;
    }

    ret = avformat_find_stream_info(fmt_ctx, NULL);
    if (ret < 0)
    {
        fprintf(stderr, "Could not find stream information\n");
        goto end;
    }

    av_dump_format(fmt_ctx, 0, input_filename, 0);

end:
    avformat_close_input(&fmt_ctx);

    /* note: the internal buffer could have changed, and be != avio_ctx_buffer */
    if (avio_ctx)
        av_freep(&avio_ctx->buffer);
    avio_context_free(&avio_ctx);

    av_file_unmap(buffer, buffer_size);

    if (ret < 0)
    {
        fprintf(stderr, "Error occurred: %d\n", ret);
        return 1;
    }

    return 0;
}
