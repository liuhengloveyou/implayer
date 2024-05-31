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

#ifdef __cplusplus
}
#endif

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/bind.h>

#include <SDL2/SDL.h>
#endif

#include "core/player.h"

using namespace implayer;

IMPlayerSharedPtr player = nullptr;

// static void *ThreadDemo(void *arg)
// {
//     for (;;) {
//         sleep(1);
//         printf("ThreadDemo\n");
//     }
// }

int main(int argc, char *argv[])
{
#ifdef __EMSCRIPTEN__
    std::string in_file = "/input.mp4";
#else
    std::string in_file = "D:/input.mp4";
#endif

// #ifdef __EMSCRIPTEN_PTHREADS__
//     pthread_t tid;
//     pthread_create(&tid, NULL, ThreadDemo, NULL);
//     pthread_detach(tid);
// #endif

    player = std::make_shared<IMplayer>();
    return player->Run();
}

#ifdef __EMSCRIPTEN__
int wasm_open_websocket_fmp4(std::string path)
{
    if (path.size() >= 1024) {
        return -1;
    }

    Event *ev = (Event *)calloc(1, sizeof(Event));
    strncpy(&ev->path[0], path.c_str(), path.size());
    ev->source_type = SourceType::WEBSOCKET_FMP4_SOURCE;

    SDL_Event event;
    event.type = SDL_EVENT_OPEN;
    event.user.data1 = ev;
    SDL_PushEvent(&event);

    return 0;
}

int wasm_open_file(std::string path)
{
    if (path.size() >= 1024) {
        return -1;
    }

    Event *ev = (Event *)calloc(1, sizeof(Event));
    strncpy(&ev->path[0], path.c_str(), path.size());
    ev->source_type = SourceType::FILE_SOURCE;

    SDL_Event event;
    event.type = SDL_EVENT_OPEN;
    event.user.data1 = ev;
    SDL_PushEvent(&event);

    return 0;
}

int wasm_play()
{
    SDL_Event event;
    event.type = SDL_EVENT_PLAY;
    SDL_PushEvent(&event);

    return 0;
}

EMSCRIPTEN_BINDINGS(my_module)
{
    emscripten::function("wasm_open_file", &wasm_open_file);
    emscripten::function("wasm_open_websocket_fmp4", &wasm_open_websocket_fmp4);
    emscripten::function("wasm_play", &wasm_play);
}
#endif