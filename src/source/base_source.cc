#include "source/base_source.h"

namespace implayer
{
  BaseSource::BaseSource(IMPlayerSharedPtr player)
      : player_(player),
        video_codec_(std::make_shared<FFmpegCodec>()),
        audio_codec_(std::make_shared<FFmpegCodec>()),
        video_frame_queue_(std::make_shared<QueueType>(QueueSize)),
        audio_frame_queue_(std::make_shared<QueueType>(QueueSize))
  {
  }

  BaseSource::~BaseSource()
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

    video_codec_ = nullptr;
    audio_codec_ = nullptr;
  };

  int BaseSource::Open(const std::string &path)
  {
    int ret = initVideoCodec();
    RETURN_IF_ERROR(ret);

    ret = initAudioCodec();
    RETURN_IF_ERROR(ret);

    startThread();

    return ret;
  }

  int BaseSource::initVideoCodec()
  {
    auto *av_stream = stream(video_stream_index());
    if (av_stream == nullptr)
    {
      printf("BaseSource::InitVideoCodec nullptr\n");
      return -1;
    }

    time_base_ = av_stream->time_base;
    int ret = video_codec_->prepare(av_stream->codecpar->codec_id, av_stream->codecpar);
    RETURN_IF_ERROR_LOG(ret, "BaseSource::InitVideoCodec failed\n");
    printf("BaseSource::InitVideoCodec ok: %d\n", ret);

    return ret;
  }

  int BaseSource::initAudioCodec()
  {
    auto *av_stream = stream(audio_stream_index());
    if (av_stream == nullptr)
    {
      printf("BaseSource::InitAudioCodec nullptr\n");
      return -1;
    }

    int ret = audio_codec_->prepare(av_stream->codecpar->codec_id, av_stream->codecpar);
    RETURN_IF_ERROR_LOG(ret, "BaseSource::InitAudioCodec failed\n");
    printf("BaseSource::InitAudioCodec ok: %d\n", ret);
    return ret;
  }

  std::shared_ptr<Frame> BaseSource::NextVideoFrame()
  {
    std::shared_ptr<Frame> f = nullptr;
    video_frame_queue_->try_pop(f);

    return f;
  }
  std::shared_ptr<Frame> BaseSource::NextAudioFrame()
  {
    std::shared_ptr<Frame> f = nullptr;
    audio_frame_queue_->try_pop(f);

    return f;
  }
  int64_t BaseSource::duration()
  {
  }
  int BaseSource::queue_size()
  {
  }

  int64_t BaseSource::position() { return position_; }

  void BaseSource::threadMain()
  {
    while (m_thread_stop == false)
    {
      if (eof_.load())
      {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        continue;
      }

      PlayState state = player_->state();
      // fprintf(stdout, "BaseSource::threadMain:: %d %d %d\n", state, video_frame_queue_->size(), audio_frame_queue_->size());
      if (state == PlayState::kPlaying)
      {
        auto frame = decodeNextFrame();
        if (frame == nullptr || frame->f == nullptr)
        {
          eof_.store(true);
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
        eof_.store(false);
        video_frame_queue_->flush();
        audio_frame_queue_->flush();

        auto frame = SeekFramePrecise(seek_timestamp_);
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
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
      }
      else if (state == PlayState::kStopped)
      {
        video_frame_queue_->flush();
        audio_frame_queue_->flush();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
      }
      else if (state == PlayState::kIdle)
      {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
      }
    }
  }

  std::shared_ptr<Frame> BaseSource::decodeNextFrame()
  {
    int ret = 0;

    for (;;)
    {
      {
        auto frame = std::make_shared<Frame>(time_base_);
        frame->media_type = AVMEDIA_TYPE_VIDEO;
        ret = video_codec_->receiveFrame(frame->f);
        if (ret == 0)
        {
          UpdatePosition(frame);
          return frame;
        }
      }

      {
        auto frame = std::make_shared<Frame>(time_base_);
        frame->media_type = AVMEDIA_TYPE_AUDIO;
        ret = audio_codec_->receiveFrame(frame->f);
        if (ret == 0)
        {
          UpdatePosition(frame);
          return frame;
        }
      }

      AVPacket *pkt = nullptr;
      std::tie(ret, pkt) = ReadPacket();
      ON_SCOPE_EXIT([&pkt]
                    { av_packet_unref(pkt); });
      if (pkt == nullptr || ret != 0)
      {
        fprintf(stderr, "demux_->readPacket failed: %c\n", ret);
        return nullptr;
      }

      // skip this packet if is not target stream
      if (video_stream_index() == pkt->stream_index)
      {
        auto frame = std::make_shared<Frame>(time_base_);
        frame->media_type = AVMEDIA_TYPE_VIDEO;

        ret = video_codec_->sendPacketToCodec(pkt);
        if (ret == AVERROR_EOF)
        {
          LOGE("sendPacketToCodec AVERROR_EOF\n");
          return frame;
        }

        ret = video_codec_->receiveFrame(frame->f);
        if (ret == 0)
        {
          UpdatePosition(frame);
          return frame;
        }
        else if (ret == AVERROR(EAGAIN))
        {
          continue;
        }
        else
        {
          LOGE("codec_->receiveFrame failed");
          return nullptr;
        }
      }
      else if (audio_stream_index() == pkt->stream_index)
      {
        auto frame = std::make_shared<Frame>(time_base_);
        frame->media_type = AVMEDIA_TYPE_AUDIO;

        ret = audio_codec_->sendPacketToCodec(pkt);
        if (ret == AVERROR_EOF)
        {
          LOGE("sendPacketToCodec AVERROR_EOF\n");
          return frame;
        }

        ret = audio_codec_->receiveFrame(frame->f);
        if (ret == 0)
        {
          UpdatePosition(frame);
          return frame;
        }
        else if (ret == AVERROR(EAGAIN))
        {
          continue;
        }
        else
        {
          LOGE("codec_->receiveFrame failed");
          return nullptr;
        }
      }
    }
  }

  int BaseSource::Seek(int64_t timestamp)
  {
    seek_timestamp_ = timestamp;
    eof_.store(false);

    return 0;
  }

  void BaseSource::UpdatePosition(const std::shared_ptr<Frame> &frame)
  {
    if (frame->f->pts != AV_NOPTS_VALUE)
    {
      position_ = av_rescale_q(frame->f->pts,
                               stream(video_stream_index())->time_base,
                               AV_TIME_BASE_Q);
    }
  }

  std::shared_ptr<Frame> BaseSource::SeekFrameQuick(int64_t timestamp)
  {

    auto ret = SeekDemuxerAndFlushCodecBuffer(timestamp, position_);
    if (ret != 0)
    {
      return nullptr;
    }

    return decodeNextFrame();
  }

  std::shared_ptr<Frame> BaseSource::SeekFramePrecise(int64_t timestamp)
  {

    int64_t position = position_;
    auto ret = SeekDemuxerAndFlushCodecBuffer(timestamp, position);
    if (ret != 0)
    {
      return nullptr;
    }

    for (;;)
    {
      auto frame = decodeNextFrame();
      if (frame == nullptr)
      {
        return frame;
      }

      auto frame_pts = av_rescale_q(frame->f->pts, time_base_, AV_TIME_BASE_Q);
      if (frame_pts >= timestamp)
      {
        return frame;
      }
    }
  }

  int BaseSource::SeekToTargetPosition(int64_t seek_pos, int64_t seek_rel)
  {
    // auto seek_flags = 0;
    // if (seek_rel < 0)
    // {
    //   seek_flags = AVSEEK_FLAG_BACKWARD;
    // }
    // auto min_ts = (seek_rel > 0) ? (seek_pos - seek_rel + 2) : (INT64_MIN);
    // auto max_ts = (seek_rel < 0) ? (seek_pos - seek_rel - 2) : (seek_pos);
    // return demux_->seek(min_ts, seek_pos, max_ts, seek_flags);
  }

  int BaseSource::SeekDemuxerAndFlushCodecBuffer(int64_t timestamp, int64_t target_pos)
  {
    auto ret = SeekToTargetPosition(timestamp, timestamp - target_pos);
    if (ret != 0)
    {
      LOGE("SeekToTargetPosition failed");
      return -1;
    }

    video_codec_->flush_buffers();
    audio_codec_->flush_buffers();

    return 0;
  }

  std::pair<int, AVPacket *> BaseSource::ReadPacket()
  {
    printf("BaseSource:: ReadPacket\n");
  }

  // std::shared_ptr<Frame> BaseSource::dequeueVideoFrame() { return tryPopAFrame(AVMEDIA_TYPE_VIDEO); }
  // std::shared_ptr<Frame> BaseSource::dequeueAudioFrame() { return tryPopAFrame(AVMEDIA_TYPE_AUDIO); }
  // std::shared_ptr<Frame> BaseSource::tryPopAFrame(AVMediaType media_type)
  // {
  //   std::shared_ptr<Frame> f = nullptr;
  //   if (media_type == AVMEDIA_TYPE_VIDEO)
  //   {
  //     video_frame_queue_->try_pop(f);
  //   }
  //   else if (media_type == AVMEDIA_TYPE_AUDIO)
  //   {
  //     audio_frame_queue_->try_pop(f);
  //   }

  //   return f;
  // }

}
