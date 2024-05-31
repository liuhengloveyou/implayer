#ifndef IMPLAYER_MEDIA_FILE_INFO_H
#define IMPLAYER_MEDIA_FILE_INFO_H

#include <iostream>
#include <string>

#ifdef __cplusplus
extern "C"
{
#endif

#include <libavformat/avformat.h>

#ifdef __cplusplus
}
#endif

namespace implayer
{
  class MediaFileInfo
  {
  public:
    std::string path;
    int pixel_format{0}; // AVPixelFormat
    int width{0};
    int height{0};
    int64_t duration{0};
    int64_t bit_rate{0};
    double fps{0};
    AVRational video_stream_timebase{AVRational{0, 0}};

    int sample_rate{0};
    int channels{0};
    int sample_format{0}; // AVSampleFormat
    int channel_layout{0};
    AVRational audio_stream_timebase{AVRational{0, 0}};

    void Dump()
    {
      std::cout << std::endl
                << "pixel_format:" << pixel_format << std::endl
                << "width:" << width << std::endl
                << "height:" << height << std::endl
                << "bit_rate:" << bit_rate << std::endl
                << "duration: " << duration << std::endl
                << "fps: " << fps << std::endl
                << "sample_rate: " << sample_rate << std::endl
                << "channels: " << channels << std::endl
                << "sample_format: " << sample_format << std::endl
                << "channel_layout: " << channel_layout << std::endl
                << "audio_stream_timebase: " << audio_stream_timebase.den << std::endl;
    }
  };

}

#endif
