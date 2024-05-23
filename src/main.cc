#include <queue>
#include <stdio.h>
#include <thread>
#include <memory>

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
using namespace std::chrono_literals;

int main(int argc, char *argv[]) {
  std::string in_file = "D:/input.mp4";

  IMPlayerSharedPtr player = std::make_shared<IMplayer>();
  int ret = player->open(player, in_file);
  printf("open player %d\n", ret);
  
  player->play();

  SDL_Event event;
  auto doSeekRelative = [&](float sec) {
    auto current_pos = player->getCurrentPosition();
    auto target_pos = current_pos + static_cast<int64_t>(sec * AV_TIME_BASE);
    LOGE("seek to %lf\n", double(target_pos) / AV_TIME_BASE);
    player->seek(target_pos);
  };
  auto doPauseOrPlaying = [&]() {
    auto is_playing = player->state() == PlayState::kPlaying;
    if (is_playing) {
      player->pause();
    } else {
      player->play();
    }
  };

  for (;;) {
    SDL_PollEvent(&event);
    switch (event.type) {
    case SDL_KEYDOWN: {
      switch (event.key.keysym.sym) {
      case SDLK_LEFT: {
        doSeekRelative(-5.0);
        break;
      }

      case SDLK_RIGHT: {
        doSeekRelative(5.0);
        break;
      }

      case SDLK_DOWN: {
        doSeekRelative(-60.0);
        break;
      }

      case SDLK_UP: {
        doSeekRelative(60.0);
        break;
      }
      case SDLK_SPACE: {
        doPauseOrPlaying();
        break;
      }
      default:
        break;
      }
      break;
    }
    case SDL_QUIT:
      player->stop();
      return 0;
    }
  }
}