#ifndef IMPLAYER_MEDIA_FILE_INFO_H
#define IMPLAYER_MEDIA_FILE_INFO_H

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
    std::string file_path;
    int width{0};
    int height{0};
    int64_t duration{0};
    int64_t bit_rate{0};
    double fps{0};
    int pixel_format{0}; // AVPixelFormat
    AVRational video_stream_timebase{AVRational{0, 0}};

    int sample_rate{0};
    int channels{0};
    int sample_format{0}; // AVSampleFormat
    int channel_layout{0};
    AVRational audio_stream_timebase{AVRational{0, 0}};
  };

} // namespace implayer

#endif // IMPLAYER_MEDIA_FILE_INFO_H
