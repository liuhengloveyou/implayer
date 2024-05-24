#include "source/ffmpeg_decoder.h"
#include "source/simple_source.h"
#include "output/sdl2_audio_output.h"
#include "output/sdl2_video_output.h"
#include "output/imgui_sdl2_video_output.h"
#include "ffmpeg/ffmpeg_codec.h"
#include "ffmpeg/ffmpeg_demuxer.h"
#include "ffmpeg/ffmpeg_headers.h"
#include "ffmpeg/ffmpeg_decoder_context.h"
#include "core/player.h"

namespace implayer
{
  IMplayer::IMplayer()
      : clock_(std::make_shared<ClockManager>())
  {
  }

  IMplayer::~IMplayer()
  {
    audio_output_ = nullptr;
    video_output_ = nullptr;
    source_ = nullptr;
  }

  int IMplayer::open(IMPlayerSharedPtr player, const std::string &in_file)
  {
    source_ = std::make_shared<SimpleSource>(player);

    auto ret = source_->open(in_file);
    RETURN_IF_ERROR_LOG(ret, "open source failed, exit");
    fprintf(stdout, "video_source_->open ok.\n");

    media_file_info_ = source_->getMediaFileInfo();
    VideoOutputParameters video_output_param;
    video_output_param.width = media_file_info_.width;
    video_output_param.height = media_file_info_.height;
    video_output_param.pixel_format = AVPixelFormat::AV_PIX_FMT_YUV420P;

    AudioOutputParameters audio_output_param;
    audio_output_param.sample_rate = media_file_info_.sample_rate;
    audio_output_param.channels = media_file_info_.channels == 1 ? 1 : 2;
    audio_output_param.num_frames_of_buffer = 1024;

    video_output_ = std::make_shared<IMSDL2Output>(player);
    audio_output_ = std::make_shared<SDL2AudioOutput>(player);
    ret = prepareForOutput(media_file_info_, video_output_param, audio_output_param);
    fprintf(stdout, "prepare player: %d\n", ret);

    return 0;
  }

  MediaFileInfo IMplayer::getMediaFileInfo() { return media_file_info_; }

  int IMplayer::play()
  {
    state_.store(PlayState::kPlaying);

    int ret = 0;
    if (audio_output_)
    {
      ret |= audio_output_->play();
    }

    if (video_output_)
    {
      ret |= video_output_->play();
    }
    if (ret != 0)
    {
      LOGE("play failed, exit");
      return -1;
    }

    printf("IMplayer::play\n");

    return 0;
  }

  int IMplayer::stop()
  {
    state_.store(PlayState::kStopped);
    int ret = 0;
    if (audio_output_)
    {
      ret |= audio_output_->stop();
    }
    if (ret != 0)
    {
      LOGE("stop failed, exit");
      return -1;
    }
    return 0;
  }

  int IMplayer::seek(int64_t timestamp)
  {
    int ret = 0;
    if (audio_output_)
    {
      // seek with no audio output
      ret |= audio_output_->stop();
    }
    if (source_)
    {
      ret |= source_->seek(timestamp);
    }
    if (ret != 0)
    {
      LOGE("seek failed, exit");
      return -1;
    }
    return 0;
  }

  int IMplayer::pause()
  {
    state_.store(PlayState::kPaused);

    return 0;
  }

  int64_t IMplayer::getDuration() const
  {
    int64_t v_duration = 0;
    int64_t a_duration = 0;
    if (source_)
    {
      v_duration = source_->getDuration();
    }
    return std::max(v_duration, a_duration);
  }

  int64_t IMplayer::getCurrentPosition()
  {
    if (source_)
    {
      return source_->getCurrentPosition();
    }
    return 0;
  }

  int IMplayer::prepareForOutput(const MediaFileInfo &media_info, const VideoOutputParameters &v_out_params, const AudioOutputParameters &a_out_params)
  {
    if (video_output_)
    {
      auto ret = video_output_->prepare(media_info, v_out_params);
      printf("prepare video output: %d\n", ret);
    }

    if (audio_output_)
    {
      auto ret = audio_output_->prepare(media_info, a_out_params);
      printf("prepare audio output: %d\n", ret);
    }

    return 0;
  }

  std::shared_ptr<Frame> IMplayer::dequeueVideoFrame()
  {
    return source_->dequeueVideoFrame();
  }
  std::shared_ptr<Frame> IMplayer::dequeueAudioFrame()
  {
    return source_->dequeueAudioFrame();
  }
}
