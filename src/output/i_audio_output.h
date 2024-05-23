#ifndef IMPLAYER_I_AUDIO_OUTPUT_H
#define IMPLAYER_I_AUDIO_OUTPUT_H

#include "ffmpeg/ffmpeg_audio_resampler.h"
#include "source/i_source.h"
#include "utils/clock_manager.h"
#include "core/media_file_info.h"

namespace implayer
{
  class AudioOutputParameters
  {
  public:
    int sample_rate{44100};
    int channels{2};
    int num_frames_of_buffer{1024};

    bool isValid() const
    {
      return sample_rate > 0 && channels > 0 && num_frames_of_buffer > 0;
    }
  };

  class IAudioOutput
  {
  public:
    virtual int prepare(const MediaFileInfo &media_info, const AudioOutputParameters &parameters) = 0;
    virtual int play() = 0;
    virtual int stop() = 0;
  };
} // namespace implayer

#endif // IMPLAYER_I_AUDIO_OUTPUT_H
