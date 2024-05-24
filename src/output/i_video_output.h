#ifndef IMPLAYER_I_VIDEO_OUTPUT_H
#define IMPLAYER_I_VIDEO_OUTPUT_H

#include "ffmpeg/ffmpeg_image_converter.h"
#include "source/i_source.h"
#include "utils/clock_manager.h"

namespace implayer
{
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
