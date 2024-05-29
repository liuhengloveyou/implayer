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

#include <unistd.h>
#endif

#include "core/player.h"

using namespace implayer;

IMPlayerSharedPtr player = nullptr;

static void *ThreadDemo(void *arg)
{
    for (;;) {
        sleep(1);
        printf("ThreadDemo\n");
    }
}

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
    int ret = player->open("ws://localhost:8080/ws");
    printf("open player %d\n", ret);

    // player->play();

    printf("main return\n");
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