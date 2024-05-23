#ifndef IMPLAYER_BASE_AUDIO_OUTPUT_H
#define IMPLAYER_BASE_AUDIO_OUTPUT_H

#include "output/i_audio_output.h"
#include "utils/simple_fifo.h"
#include "core/i_player.h"

namespace implayer
{
  class BaseAudioOutput : public IAudioOutput
  {
  public:
    BaseAudioOutput() = delete;
    explicit BaseAudioOutput(IMPlayerSharedPtr player);
    ~BaseAudioOutput();

  public:
    int prepare(const MediaFileInfo &media_info, const AudioOutputParameters &parameters) override;
    int prepareAudioResampler(const MediaFileInfo &src_media_file_info, const AudioOutputParameters &a_out_params);

  protected:
    void pullAudioSamples(int16_t *stream, int num_samples);
    void resampleAndPushToFIFO(const std::shared_ptr<Frame> &frame);

    std::shared_ptr<FFmpegAudioResampler> resampler_{nullptr};
    int64_t last_frame_pts_{-1};
    constexpr static int kFIFOSize = 44100;
    SimpleFIFO<int16_t> sample_fifo_{kFIFOSize};
    IMPlayerSharedPtr player_{nullptr};
    AudioOutputParameters parameter_;
  };
}

#endif
