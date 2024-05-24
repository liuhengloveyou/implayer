#include <SDL.h>

#include "source/simple_source.h"
#include "source/ffmpeg_decoder.h"

namespace implayer
{
  SimpleSource::SimpleSource(IMPlayerSharedPtr player)
      : player_(player),
        decoder_(std::make_shared<FFmpegAVDecoder>()),
        video_frame_queue_(std::make_shared<QueueType>(QueueSize)),
        audio_frame_queue_(std::make_shared<QueueType>(QueueSize))
  {
  }

  SimpleSource::~SimpleSource()
  {
    stopThread();

    if (video_frame_queue_)
    {
      video_frame_queue_->flush();
      video_frame_queue_ = nullptr;
    }
    if (audio_frame_queue_)
    {
      audio_frame_queue_->flush();
      audio_frame_queue_ = nullptr;
    }

    decoder_ = nullptr;
  };

  int SimpleSource::open(const std::string &file_path)
  {
    startThread();

    return decoder_->open(file_path);
  }

  void SimpleSource::threadMain()
  {
    while (m_thread_stop == false)
    {
      if (eof.load())
      {
        SDL_Delay(10);
        continue;
      }

      PlayState state = player_->state();
      // fprintf(stdout, "SimpleSource::threadMain:: %d %d %d\n", state, video_frame_queue_->size(), audio_frame_queue_->size());
      if (state == PlayState::kPlaying)
      {
        auto frame = decoder_->decodeNextFrame();
        if (frame == nullptr || frame->f == nullptr)
        {
          eof.store(true);
          continue;
        }

        if (frame->isVideo())
        {
          video_frame_queue_->wait_and_push(std::move(frame));
        }
        else if (frame->isAudio())
        {
          audio_frame_queue_->wait_and_push(std::move(frame));
        }
      }
      else if (state == PlayState::kSeeking)
      {
        eof.store(false);
        video_frame_queue_->flush();
        audio_frame_queue_->flush();

        auto frame = decoder_->seekFramePrecise(seek_timestamp_);
        if (frame)
        {
          video_frame_queue_->wait_and_push(std::move(frame));
          state = PlayState::kPaused;
        }
        else
        {
          state = PlayState::kStopped;
        }
      }
      else if (state == PlayState::kPaused)
      {
        SDL_Delay(10);
        // wait_event_.wait(-1);
      }
      else if (state == PlayState::kStopped)
      {
        video_frame_queue_->flush();
        audio_frame_queue_->flush();
        SDL_Delay(10);
      }
      else if (state == PlayState::kIdle)
      {
        SDL_Delay(10);
      }
    }
  }

  MediaFileInfo SimpleSource::getMediaFileInfo()
  {
    return decoder_->getMediaFileInfo();
  }

  int SimpleSource::seek(int64_t timestamp)
  {
    seek_timestamp_ = timestamp;
    eof.store(false);

    return 0;
  }

  int64_t SimpleSource::getDuration()
  {
    return decoder_->getMediaFileInfo().duration;
  }

  int64_t SimpleSource::getCurrentPosition()
  {
    return decoder_->getPosition();
  }

  std::shared_ptr<Frame> SimpleSource::dequeueVideoFrame() { return tryPopAFrame(AVMEDIA_TYPE_VIDEO); }
  std::shared_ptr<Frame> SimpleSource::dequeueAudioFrame() { return tryPopAFrame(AVMEDIA_TYPE_AUDIO); }
  std::shared_ptr<Frame> SimpleSource::tryPopAFrame(AVMediaType media_type)
  {
    std::shared_ptr<Frame> f = nullptr;
    if (media_type == AVMEDIA_TYPE_VIDEO)
    {
      video_frame_queue_->try_pop(f);
    }
    else if (media_type == AVMEDIA_TYPE_AUDIO)
    {
      audio_frame_queue_->try_pop(f);
    }

    return f;
  }

}
