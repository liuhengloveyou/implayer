#ifndef IMPLAYER_SDL2_VIDEO_OUTPUT_H
#define IMPLAYER_SDL2_VIDEO_OUTPUT_H

#include <SDL2/SDL.h>

#include "ffmpeg/ffmpeg_common_utils.h"
#include "output/base_video_output.h"
#include "core/i_player.h"

namespace implayer
{
  class SDL2VideoOutput : public BaseVideoOutput
  {
  public:
    SDL2VideoOutput() = delete;
    explicit SDL2VideoOutput(IMPlayerSharedPtr player);
    ~SDL2VideoOutput();

  public:
    int prepare(const MediaFileInfo &media_info, const VideoOutputParameters &parameters) override;
    int drawFrame(std::shared_ptr<Frame> frame) override;

  private:
    int initSDL2System(const VideoOutputParameters &parameters);

  private:
    SDL_Window *window_{nullptr};
    SDL_Renderer *renderer_{nullptr};
    SDL_Texture *texture_{nullptr};
  };
}

#endif
