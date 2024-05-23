#include "output/base_audio_output.h"

namespace implayer
{
  BaseAudioOutput::BaseAudioOutput(IMPlayerSharedPtr player)
      : player_(player),
        resampler_(std::make_shared<FFmpegAudioResampler>()){
        };

  BaseAudioOutput::~BaseAudioOutput() {
    resampler_ = nullptr;
    player_ = nullptr;
  }

  int BaseAudioOutput::prepare(const MediaFileInfo &media_info, const AudioOutputParameters &parameters)
  {
    parameter_ = parameters;

    return prepareAudioResampler(media_info, parameters);
  }

  int BaseAudioOutput::prepareAudioResampler(const MediaFileInfo &src_media_file_info, const AudioOutputParameters &a_out_params)
  {
    int max_frame_size = a_out_params.num_frames_of_buffer * 4 * a_out_params.channels;
    int output_channel_layout = (a_out_params.channels == 1) ? AV_CH_LAYOUT_MONO : AV_CH_LAYOUT_STEREO;

    auto ret = resampler_->prepare(
        src_media_file_info.channels, a_out_params.channels,
        src_media_file_info.channel_layout, output_channel_layout,
        src_media_file_info.sample_rate, a_out_params.sample_rate,
        (AVSampleFormat)src_media_file_info.sample_format, AV_SAMPLE_FMT_S16,
        max_frame_size);

    printf("prepare audio resampler: %d\n", ret);
    return ret;
  }

  void BaseAudioOutput::pullAudioSamples(int16_t *stream, int num_samples)
  {
    int output_index = 0;
    int16_t sample;
    int64_t pts{-1};
    for (; output_index < num_samples;)
    {
      auto ok = sample_fifo_.pop(sample);
      if (ok)
      {
        if (pts == -1)
        {
          pts = last_frame_pts_;
        }
        stream[output_index++] = sample;
      }
      else
      {
        // deque a frame and resample it
        // then push to sample_fifo_
        auto frame = player_->dequeueAudioFrame();
        if (frame == nullptr)
        {
          break;
        }
        else
        {
          resampleAndPushToFIFO(frame);
        }
      }
    }

    // update audio clock
    if (player_)
    {
      double pts_d = static_cast<double>(pts) / AV_TIME_BASE;
      player_->updateAudioClock(pts_d);
    }
  }

  void BaseAudioOutput::resampleAndPushToFIFO(const std::shared_ptr<Frame> &frame)
  {
    last_frame_pts_ = frame->pts();

    if (resampler_)
    {
      auto *f = frame->f;
      auto num_samples_out_per_channel =
          resampler_->convert((const uint8_t **)f->data, f->nb_samples);
      auto num_total_samples =
          num_samples_out_per_channel * resampler_->out_num_channels();
      auto *int16_resample_data =
          reinterpret_cast<int16_t *>(resampler_->resample_data[0]);
      for (auto i = 0; i < num_total_samples; ++i)
      {
        sample_fifo_.push(std::move(int16_resample_data[i]));
      }
    }
    else
    {
      auto *f = frame->f;
      auto num_total_samples = f->nb_samples * f->channels;
      for (auto i = 0; i < num_total_samples; ++i)
      {
        sample_fifo_.push(std::move(f->data[0][i]));
      }
    }
  }

}