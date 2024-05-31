#ifndef IMPLAYER_I_VIDEO_OUTPUT_H
#define IMPLAYER_I_VIDEO_OUTPUT_H

#include "ffmpeg/ffmpeg_image_converter.h"
#include "source/i_source.h"
#include "utils/clock_manager.h"
#include <SDL2/SDL.h>

namespace implayer
{
#define SDL_EVENT_OPEN (SDL_USEREVENT + 1)
#define SDL_EVENT_PLAY (SDL_USEREVENT + 2)
#define SDL_EVENT_REFRESH (SDL_USEREVENT + 3)
#define SDL_EVENT_REFRESH_AUDIO (SDL_USEREVENT + 4)
#define SDL_EVENT_RESIZE (SDL_USEREVENT + 5)

  class VideoOutputParameters
  {
  public:
    int width{0};
    int height{0};
    int fps{0};
    int pixel_format{0}; // AVPixelFormat
  };

  class IVideoOutput
  {
  public:
    virtual int prepare(const MediaFileInfo &media_info, const VideoOutputParameters &parameters) = 0;
    virtual int play() = 0;
  };

} // namespace implayer

#endif // IMPLAYER_I_VIDEO_OUTPUT_H
