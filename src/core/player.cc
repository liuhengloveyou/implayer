#include <thread>
#include <chrono>

#ifdef __EMSCRIPTEN__
#include "emscripten/emscripten_mainloop_stub.h"
#endif

#include "output/sdl2_audio_output.h"
#include "output/sdl2_video_output.h"
#include "output/imgui_sdl2_video_output.h"
#include "ffmpeg/ffmpeg_codec.h"
#include "ffmpeg/ffmpeg_base_demuxer.h"
#include "ffmpeg/ffmpeg_headers.h"
#include "ffmpeg/ffmpeg_decoder_context.h"
#include "core/player.h"
#include "network/emscripten_websocket.h"
#include "source/source_factory.h"

namespace implayer
{
  IMplayer::IMplayer()
      : clock_(std::make_shared<ClockManager>())
  {
    if (SDL_InitSubSystem(SDL_INIT_EVENTS | SDL_INIT_TIMER) != 0)
    {
      printf("SDL_InitSubSystem: %s\n", SDL_GetError());
    }

    startThread();
  }

  IMplayer::~IMplayer()
  {
    stopThread();
    SDL_QuitSubSystem(SDL_INIT_EVENTS | SDL_INIT_TIMER);
    audio_output_ = nullptr;
    video_output_ = nullptr;
    source_ = nullptr;
  }

  // webbrower main loop
  int IMplayer::Run()
  {

#ifdef __EMSCRIPTEN__
    EMSCRIPTEN_MAINLOOP_BEGIN
#else
    while (state() != PlayState::kStopped)
#endif
    {
      OnEvent();

      if (opened_)
      {
        video_output_->play();
      }
    }
#ifdef __EMSCRIPTEN__
    EMSCRIPTEN_MAINLOOP_END;
#endif
  }

  void IMplayer::threadMain()
  {
    while (m_thread_stop == false)
    {
      // fprintf(stderr, "IMplayer::threadMain: ~~~~~~\n");
      std::this_thread::sleep_for(std::chrono::milliseconds(10));

      if (opened_ == false)
      {
        if (waitable_.wait(10) == false)
        {
          continue;
        }

        printf("IMplayer::threadMain: [%s]\n", path_.c_str());
        if (source_type_ != SourceType::UNKNOWN && path_ != "")
        {
          if (open(source_type_, path_) == 0)
          {
            opened_ = true;
          }
        }
      }
    }
  }

  int IMplayer::open(SourceType source_type, const std::string &path)
  {
    int ret = 0;

    source_ = SourceFactory::getInstance().Create(source_type, shared_from_this());
    ret = source_->Open(path);
    RETURN_IF_ERROR_LOG(ret, "open source failed, exit");
    media_file_info_ = source_->media_info();
    media_file_info_.Dump();

    VideoOutputParameters video_output_param;
    video_output_param.width = media_file_info_.width;
    video_output_param.height = media_file_info_.height;
    video_output_param.pixel_format = AVPixelFormat::AV_PIX_FMT_YUV420P;

    AudioOutputParameters audio_output_param;
    audio_output_param.sample_rate = media_file_info_.sample_rate;
    audio_output_param.channels = media_file_info_.channels == 1 ? 1 : 2;
    audio_output_param.num_frames_of_buffer = 1024;

    video_output_ = std::make_shared<IMSDL2Output>(shared_from_this());
    audio_output_ = std::make_shared<SDL2AudioOutput>(shared_from_this());
    ret = prepareForOutput(media_file_info_, video_output_param, audio_output_param);
    fprintf(stdout, "prepare player: %d\n", ret);

    // play();

    return ret;
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
      ret |= source_->Seek(timestamp);
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

  void IMplayer::doSeekRelative(float sec)
  {
    auto current_pos = getCurrentPosition();
    auto target_pos = current_pos + static_cast<int64_t>(sec * AV_TIME_BASE);
    LOGE("seek to %lf\n", double(target_pos) / AV_TIME_BASE);
    seek(target_pos);
  }

  void IMplayer::doPauseOrPlaying()
  {
    auto is_playing = state() == PlayState::kPlaying;
    if (is_playing)
    {
      pause();
    }
    else
    {
      play();
    }
  };

  int64_t IMplayer::getDuration() const
  {
    int64_t v_duration = 0;
    int64_t a_duration = 0;
    if (source_)
    {
      v_duration = source_->duration();
    }
    return std::max(v_duration, a_duration);
  }

  int64_t IMplayer::getCurrentPosition()
  {
    if (source_)
    {
      return source_->position();
    }
    return 0;
  }

  int IMplayer::prepareForOutput(const MediaFileInfo &media_info, const VideoOutputParameters &v_out_params, const AudioOutputParameters &a_out_params)
  {
    int ret = 0;

    // if (audio_output_)
    // {
    //   ret = audio_output_->prepare(media_info, a_out_params);
    //   printf("prepare audio output: %d\n", ret);
    // }
    if (ret) {
      return ret;
    }

    if (video_output_)
    {
      ret = video_output_->prepare(media_info, v_out_params);
      printf("prepare video output: %d\n", ret);
    }

    return ret;
  }

  std::shared_ptr<Frame> IMplayer::dequeueVideoFrame()
  {
    return source_->NextVideoFrame();
  }
  std::shared_ptr<Frame> IMplayer::dequeueAudioFrame()
  {
    return source_->NextAudioFrame();
  }

  void IMplayer::OnEvent()
  {
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {

      // ImGui_ImplSDL2_ProcessEvent(&event);

      switch (event.type)
      {
      case SDL_WINDOWEVENT:
      {
        if (event.window.event == SDL_WINDOWEVENT_RESIZED)
        {
          // onResizeEvent();
        }
        else if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE) // && event.window.windowID == SDL_GetWindowID(m_sdlWindow)
        {
        }
      }
      break;
      case SDL_EVENT_OPEN:
      {
        HandleOpenEvent(event.user.data1);
      }
      break;
      case SDL_EVENT_PLAY:
      {
        HandlePlayEvent(event.user.data1);
      }
      break;
      case SDL_EVENT_REFRESH:
      {
        // onUpdateFrame(event.user.data1);
      }
      case SDL_KEYDOWN:
      {
        switch (event.key.keysym.sym)
        {
        case SDLK_LEFT:
        {
          doSeekRelative(-5.0);
          break;
        }

        case SDLK_RIGHT:
        {
          doSeekRelative(5.0);
          break;
        }

        case SDLK_DOWN:
        {
          doSeekRelative(-60.0);
          break;
        }

        case SDLK_UP:
        {
          doSeekRelative(60.0);
          break;
        }
        case SDLK_SPACE:
        {
          doPauseOrPlaying();
          break;
        }
        default:
          break;
        }
        break;
      }
      break;
      }
    }
  }

  void IMplayer::HandleOpenEvent(void *data)
  {
    Event *ev = (Event *)data;
    path_ = std::string(ev->path);
    source_type_ = ev->source_type;
    free(ev);

    waitable_.signal();

    return;
  }

  void IMplayer::HandlePlayEvent(void *data)
  {
    Event *ev = (Event *)data;
    printf("IMplayer::HandlePlayEvent: [%s]\n", ev->path);
    free(ev);

    return;
  }
}
