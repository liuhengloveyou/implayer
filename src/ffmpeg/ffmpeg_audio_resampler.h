#ifndef IMPLAYER_FFMPEG_AUDIO_RESAMPLER_H
#define IMPLAYER_FFMPEG_AUDIO_RESAMPLER_H

#include <vector>

#include "ffmpeg/ffmpeg_common_utils.h"
#include "ffmpeg/ffmpeg_format_utils.h"
#include "ffmpeg/ffmpeg_headers.h"


namespace implayer {
class FFmpegAudioResampler {
public:
  ~FFmpegAudioResampler() { close(); }
  int prepare(int in_num_channels, int out_num_channels, int in_channel_layout,
              int out_channel_layout, int in_sample_rate, int out_sample_rate,
              AVSampleFormat in_sample_format, AVSampleFormat out_sample_format,
              int max_frames_size) {
    if (swr != nullptr) {
      close();
    }

    swr = swr_alloc();
    if (swr == nullptr) {
      return -1;
    }
    av_opt_set_int(swr, "in_channel_count", in_num_channels, 0);
    av_opt_set_int(swr, "out_channel_count", out_num_channels, 0);
    av_opt_set_int(swr, "in_channel_layout", in_channel_layout, 0);
    av_opt_set_int(swr, "out_channel_layout", out_channel_layout, 0);
    av_opt_set_int(swr, "in_sample_rate", in_sample_rate, 0);
    av_opt_set_int(swr, "out_sample_rate", out_sample_rate, 0);
    av_opt_set_sample_fmt(swr, "in_sample_fmt", in_sample_format, 0);
    av_opt_set_sample_fmt(swr, "out_sample_fmt", out_sample_format, 0);
    int ret = swr_init(swr);
    RETURN_IF_ERROR_LOG(ret, "swr_init failed\n");

    auto channel_data_size = FormatUtils::getChannelDataSize(out_num_channels, out_sample_format);
    resample_data.resize(channel_data_size, nullptr);
    av_samples_alloc(resample_data.data(), NULL, out_num_channels,
                     max_frames_size, out_sample_format, 0);

    return ret;
  }

  int out_num_channels() const{
    int64_t out = 0;
    av_opt_get_int(swr, "out_channel_count", 0, &out);
    return out;
  }

  int convert(const uint8_t **in, int in_num_samples_per_channel) {
    return swr_convert(swr, resample_data.data(), in_num_samples_per_channel,
                       in, in_num_samples_per_channel);
  }

  void close() {
    if (swr != nullptr) {
      swr_free(&swr);
      swr = nullptr;
    }

    if (!resample_data.empty()) {
      av_freep(&resample_data[0]);
      resample_data.clear();
    }
  }

  std::vector<uint8_t *> resample_data;
  struct SwrContext *swr{nullptr};
};

} // namespace implayer

#endif // IMPLAYER_FFMPEG_AUDIO_RESAMPLER_H
