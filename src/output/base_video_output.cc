
#include "output/base_video_output.h"

#include <SDL.h>

namespace implayer
{
  BaseVideoOutput::BaseVideoOutput(IMPlayerSharedPtr player)
      : player_(player),
        image_converter_(std::make_shared<FFMPEGImageConverter>()){};

  BaseVideoOutput::~BaseVideoOutput()
  {
    stopThread();
    player_ = nullptr;
    image_converter_ = nullptr;
  }

  int BaseVideoOutput::prepare(const MediaFileInfo &media_info, const VideoOutputParameters &parameters)
  {
    if (parameters.width <= 0 || parameters.height <= 0)
    {
      LOGE("invalid width or height");
      return -1;
    }
    parameters_ = parameters;

    startThread();

    return prepareImageConverter(media_info, parameters);
  }

  int BaseVideoOutput::prepareImageConverter(const MediaFileInfo &src_media_info, const VideoOutputParameters &v_out_params)
  {
    int expected_width = v_out_params.width;
    int expected_height = v_out_params.height;
    int expected_pixel_format = v_out_params.pixel_format;

    auto ret = image_converter_->prepare(
        src_media_info.width, src_media_info.height,
        (AVPixelFormat)src_media_info.pixel_format, expected_width,
        expected_height, (AVPixelFormat)expected_pixel_format, 0, nullptr,
        nullptr, nullptr);
    printf("prepare image converter: %d\n", ret);

    return 0;
  }

  void BaseVideoOutput::threadMain()
  {
    while (m_thread_stop == false)
    {
      PlayState state = player_->state();
      if (state == PlayState::kPlaying)
      {
        auto frame = player_->dequeueVideoFrame();
        if (frame == nullptr)
        {
          SDL_Delay(100);
          continue;
        }

        std::shared_ptr<Frame> frame_for_draw = convertFrame(frame);
        if (frame_for_draw != nullptr)
        {
          drawFrame(frame_for_draw);
          doAVSync(frame_for_draw->pts_d());
        }
      }
      else if (state == PlayState::kStopped || state == PlayState::kIdle || state == PlayState::kPaused)
      {
        SDL_Delay(100);
        continue;
      }
    }
  }

  std::shared_ptr<Frame> BaseVideoOutput::convertFrame(std::shared_ptr<Frame> frame)
  {
    if (converter_)
    {
      return converter_->convert2(frame);
    }
    return frame;
  }

  void BaseVideoOutput::doAVSync(double pts_d)
  {
    if (player_)
    {
      player_->updateVideoClock(pts_d);
    }
    else
    {
      auto delay_ms = parameters_.fps > 0 ? (int)(1000 / parameters_.fps) : 30;
      std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
    }
  }

}