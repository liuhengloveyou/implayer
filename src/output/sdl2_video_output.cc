#include "output/sdl2_video_output.h"

namespace implayer
{
  SDL2VideoOutput::SDL2VideoOutput(IMPlayerSharedPtr player)
      : BaseVideoOutput(player)
  {
  }

  SDL2VideoOutput::~SDL2VideoOutput()
  {
    if (texture_)
    {
      SDL_DestroyTexture(texture_);
      texture_ = nullptr;
    }

    if (renderer_)
    {
      SDL_DestroyRenderer(renderer_);
      renderer_ = nullptr;
    }

    if (window_)
    {
      SDL_DestroyWindow(window_);
      window_ = nullptr;
    }

    SDL_QuitSubSystem(SDL_INIT_VIDEO);
  }

  int SDL2VideoOutput::prepare(const MediaFileInfo &media_info, const VideoOutputParameters &parameters)
  {
    auto ret = BaseVideoOutput::prepare(media_info, parameters);
    printf("BaseVideoOutput::prepare: %d\n", ret);

    ret = initSDL2System(parameters);
    printf("initSDL2System: %d\n", ret);

    return ret;
  }

  int SDL2VideoOutput::initSDL2System(const VideoOutputParameters &parameters)
  {
    if (parameters.pixel_format != AVPixelFormat::AV_PIX_FMT_YUV420P)
    {
      LOGE("only support AV_PIX_FMT_YUV420P");
      return -1;
    }

    if (SDL_InitSubSystem(SDL_INIT_VIDEO))
    {
      printf("Could not initialize SDL - %s\n", SDL_GetError());
      return -1;
    }

    if (window_ != nullptr)
    {
      LOGW("window_ is not null, please call cleanup first");
      return 0;
    }

    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    window_ = SDL_CreateWindow("implayer",
                               SDL_WINDOWPOS_UNDEFINED,
                               SDL_WINDOWPOS_UNDEFINED,
                               parameters.width,
                               parameters.height,
                               window_flags);
    if (!window_)
    {
      LOGE("SDL: could not create window - exiting:%s\n", SDL_GetError());
      return -1;
    }

    SDL_GL_SetSwapInterval(1);

    renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE);
    if (!renderer_)
    {
      LOGE("SDL: could not create renderer - exiting:%s\n", SDL_GetError());
      return -1;
    }

    texture_ = SDL_CreateTexture(renderer_,
                                 SDL_PIXELFORMAT_IYUV,
                                 SDL_TEXTUREACCESS_STREAMING,
                                 parameters.width, parameters.height);
    return 0;
  }

  int SDL2VideoOutput::updateFrame(std::shared_ptr<Frame> frame)
  {
    AVFrame *pict = frame->f;
    SDL_UpdateYUVTexture(texture_, nullptr,
                         pict->data[0], pict->linesize[0],
                         pict->data[1], pict->linesize[1],
                         pict->data[2], pict->linesize[2]);

    SDL_RenderClear(renderer_);
    SDL_RenderCopy(renderer_, // the rendering context
                   texture_,  // the source texture
                   NULL,      // the source SDL_Rect structure or NULL for the
                              // entire texture
                   NULL       // the destination SDL_Rect structure or NULL for
                              // the entire rendering target; the texture will
                              // be stretched to fill the given rectangle
    );
    SDL_RenderPresent(renderer_);

    return 0;
  }

}
