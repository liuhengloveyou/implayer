#include "source/simple_source.h"

namespace implayer
{
  SimpleSource::SimpleSource(IMPlayerSharedPtr player)
      : BaseSource::BaseSource(player),
        demux_(std::make_shared<FFmpegBaseDmuxer>())
  {
  }

  SimpleSource::~SimpleSource()
  {
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
  };

  int SimpleSource::Open(const std::string &path)
  {
    int ret = demux_->Open(path);
    printf("SimpleSource::Open demux_->open: %d\n", ret);
    if (ret)
    {
      return ret;
    }

    return BaseSource::Open(path);
  }

  std::pair<int, AVPacket *> SimpleSource::ReadPacket()
  {
    return demux_->ReadFrame();
  }

  AVStream *SimpleSource::stream(int stream_index) const
  {
    return demux_->stream(stream_index);
  }
  int SimpleSource::video_stream_index()
  {
    return demux_->video_stream_index();
  }
  int SimpleSource::audio_stream_index()
  {
    return demux_->audio_stream_index();
  }

  // void SimpleSource::threadMain()
  // {
  // while (m_thread_stop == false)
  // {
  //   if (eof.load())
  //   {
  //     std::this_thread::sleep_for(std::chrono::milliseconds(10));
  //     continue;
  //   }

  //   PlayState state = player_->state();
  //   // fprintf(stdout, "SimpleSource::threadMain:: %d %d %d\n", state, video_frame_queue_->size(), audio_frame_queue_->size());
  //   if (state == PlayState::kPlaying)
  //   {
  //     // auto frame = decoder_->decodeNextFrame();
  //     // if (frame == nullptr || frame->f == nullptr)
  //     // {
  //     //   eof.store(true);
  //     //   continue;
  //     // }

  //     // if (frame->isVideo())
  //     // {
  //     //   video_frame_queue_->wait_and_push(std::move(frame));
  //     // }
  //     // else if (frame->isAudio())
  //     // {
  //     //   audio_frame_queue_->wait_and_push(std::move(frame));
  //     // }
  //   }
  //   else if (state == PlayState::kSeeking)
  //   {
  //     // eof.store(false);
  //     // video_frame_queue_->flush();
  //     // audio_frame_queue_->flush();

  //     // auto frame = decoder_->seekFramePrecise(seek_timestamp_);
  //     // if (frame)
  //     // {
  //     //   video_frame_queue_->wait_and_push(std::move(frame));
  //     //   state = PlayState::kPaused;
  //     // }
  //     // else
  //     // {
  //     //   state = PlayState::kStopped;
  //     // }
  //   }
  //   else if (state == PlayState::kPaused)
  //   {
  //     std::this_thread::sleep_for(std::chrono::milliseconds(10));
  //     // wait_event_.wait(-1);
  //   }
  //   else if (state == PlayState::kStopped)
  //   {
  //     video_frame_queue_->flush();
  //     audio_frame_queue_->flush();
  //     std::this_thread::sleep_for(std::chrono::milliseconds(10));
  //   }
  //   else if (state == PlayState::kIdle)
  //   {
  //     std::this_thread::sleep_for(std::chrono::milliseconds(10));
  //   }
  // }
  // }

  MediaFileInfo SimpleSource::media_info() const
  {
    // return decoder_->getMediaFileInfo();
  }

  int SimpleSource::Seek(int64_t timestamp)
  {
    seek_timestamp_ = timestamp;
    eof_.store(false);

    return 0;
  }

  int64_t SimpleSource::duration()
  {
    // return decoder_->getMediaFileInfo().duration;
  }

  int64_t SimpleSource::position()
  {
    // return decoder_->getPosition();
  }

  std::shared_ptr<Frame> SimpleSource::NextVideoFrame() { return tryPopAFrame(AVMEDIA_TYPE_VIDEO); }
  std::shared_ptr<Frame> SimpleSource::NextAudioFrame() { return tryPopAFrame(AVMEDIA_TYPE_AUDIO); }
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
