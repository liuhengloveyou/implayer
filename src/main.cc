#include <queue>
#include <stdio.h>
#include <thread>
#include <memory>
#include <chrono>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/bind.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#include "SDL.h"

#ifdef __cplusplus
}
#endif

#include "core/player.h"

using namespace implayer;

IMPlayerSharedPtr player = std::make_shared<IMplayer>();

int main(int argc, char *argv[])
{
#ifdef __EMSCRIPTEN__
  std::string in_file = "/input.mp4";
#else
  std::string in_file = "D:/input.mp4";
#endif

  
  int ret = player->open(player, in_file);
  printf("open player %d\n", ret);
}


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