#include <memory>

#include "source/ffmpeg_decoder.h"
#include "core/frame.h"

namespace implayer
{
  FFmpegAVDecoder::~FFmpegAVDecoder()
  {
    demux_ = nullptr;
    video_codec_ = nullptr;
    audio_codec_ = nullptr;
    position_ = AV_NOPTS_VALUE;
  }

  int FFmpegAVDecoder::open(const std::string &file_path)
  {
    int ret = initDemuxer(file_path);
    RETURN_IF_ERROR(ret);

    ret = initCodec(AVMEDIA_TYPE_VIDEO);
    RETURN_IF_ERROR(ret);

    ret = initCodec(AVMEDIA_TYPE_AUDIO);
    RETURN_IF_ERROR(ret);

    return ret;
  }

  std::shared_ptr<Frame> FFmpegAVDecoder::decodeNextFrame()
  {
    int ret = 0;

    for (;;)
    {
      int ret = 0;

      // there may remains frames, let's try to get frame from codec first,
      {
        auto frame = std::make_shared<Frame>(video_time_base_);
        frame->media_type = AVMEDIA_TYPE_VIDEO;
        ret = video_codec_->receiveFrame(frame->f);
        if (ret == 0)
        {
          updatePosition(frame);
          return frame;
        }
      }

      {
        auto frame = std::make_shared<Frame>(audio_time_base_);
        frame->media_type = AVMEDIA_TYPE_AUDIO;
        ret = audio_codec_->receiveFrame(frame->f);
        if (ret == 0)
        {
          updatePosition(frame);
          return frame;
        }
      }

      AVPacket *pkt = nullptr;
      std::tie(ret, pkt) = demux_->readPacket();
      ON_SCOPE_EXIT([&pkt] { av_packet_unref(pkt); });
      if (pkt == nullptr || ret != 0)
      {
        fprintf(stderr, "demux_->readPacket failed: %c\n", ret);
        return nullptr;
      }

      // skip this packet if is not target stream
      if (video_stream_index_ == pkt->stream_index)
      {
        auto frame = std::make_shared<Frame>(video_time_base_);
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
          updatePosition(frame);
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
      else if (audio_stream_index_ == pkt->stream_index)
      {
        auto frame = std::make_shared<Frame>(audio_time_base_);
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
          updatePosition(frame);
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

  std::shared_ptr<Frame> FFmpegAVDecoder::seekFrameQuick(int64_t timestamp)
  {

    auto ret = seekDemuxerAndFlushCodecBuffer(timestamp, position_);
    if (ret != 0)
    {
      return nullptr;
    }

    return decodeNextFrame();
  }

  std::shared_ptr<Frame> FFmpegAVDecoder::seekFramePrecise(int64_t timestamp)
  {

    int64_t position = position_;
    auto ret = seekDemuxerAndFlushCodecBuffer(timestamp, position);
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

      auto frame_pts = av_rescale_q(frame->f->pts, video_time_base_, AV_TIME_BASE_Q);
      if (frame_pts >= timestamp)
      {
        return frame;
      }
    }
  }

  int64_t FFmpegAVDecoder::getPosition() { return position_; }

  MediaFileInfo FFmpegAVDecoder::getMediaFileInfo()
  {

    MediaFileInfo info;
    info.file_path = demux_->getFormatContext()->url;
    info.width = video_codec_->getCodecContext()->width;
    info.height = video_codec_->getCodecContext()->height;
    info.duration = demux_->getFormatContext()->duration;
    info.bit_rate = demux_->getFormatContext()->bit_rate;

    auto video_index = demux_->getVideoStreamIndex();
    auto *video_stream = demux_->getStream(video_index);
    if (video_stream != nullptr)
    {
      info.fps = av_q2d(video_stream->avg_frame_rate);
      info.pixel_format = video_stream->codecpar->format;
      info.video_stream_timebase = video_stream->time_base;
    }

    auto audio_index = demux_->getAudioStreamIndex();
    auto *audio_stream = demux_->getStream(audio_index);
    if (audio_stream != nullptr)
    {
      info.sample_rate = audio_stream->codecpar->sample_rate;
      info.channels = audio_stream->codecpar->channels;
      info.sample_format = audio_stream->codecpar->format;
      info.channel_layout = audio_stream->codecpar->channel_layout;
      info.audio_stream_timebase = audio_stream->time_base;
    }

    return info;
  }

  int FFmpegAVDecoder::initDemuxer(const std::string &file_path)
  {
    demux_ = std::make_shared<FFmpegDmuxer>();
    int ret = demux_->open(file_path);
    RETURN_IF_ERROR_LOG(ret, "demux_->openFile failed\n");
    printf("demux_->openFile ok\n");
    return ret;
  }

  int FFmpegAVDecoder::initCodec(AVMediaType media_type)
  {
    std::shared_ptr<FFmpegCodec> codec;

    if (media_type == AVMEDIA_TYPE_VIDEO)
    {
      video_codec_ = std::make_shared<FFmpegCodec>();
      video_stream_index_ = demux_->getVideoStreamIndex();
      codec = video_codec_;
      video_time_base_ = demux_->getStream(video_stream_index_)->time_base;
    }
    else if (media_type == AVMEDIA_TYPE_AUDIO)
    {
      audio_codec_ = std::make_shared<FFmpegCodec>();
      audio_stream_index_ = demux_->getAudioStreamIndex();
      codec = audio_codec_;
      audio_time_base_ = demux_->getStream(audio_stream_index_)->time_base;
    }
    else
    {
      abort();
    }

    auto *av_stream = demux_->getStream(media_type == AVMEDIA_TYPE_VIDEO ? video_stream_index_ : audio_stream_index_);
    if (av_stream == nullptr)
    {
      printf("av_stream is nullptr\n");
      return -1;
    }

    int ret = codec->prepare(av_stream->codecpar->codec_id, av_stream->codecpar);
    RETURN_IF_ERROR_LOG(ret, "codec_->prepare failed\n");

    return ret;
  }

  void FFmpegAVDecoder::updatePosition(const std::shared_ptr<Frame> &frame)
  {
    if (frame->f->pts != AV_NOPTS_VALUE)
    {
      position_ = av_rescale_q(frame->f->pts,
                               demux_->getStream(video_stream_index_)->time_base,
                               AV_TIME_BASE_Q);
    }
  }

  int FFmpegAVDecoder::seekDemuxerAndFlushCodecBuffer(int64_t timestamp, int64_t target_pos)
  {
    auto ret = seekToTargetPosition(timestamp, timestamp - target_pos);
    if (ret != 0)
    {
      LOGE("seekToTargetPosition failed");
      return -1;
    }

    video_codec_->flush_buffers();
    audio_codec_->flush_buffers();

    return 0;
  }

  int FFmpegAVDecoder::seekToTargetPosition(int64_t seek_pos, int64_t seek_rel)
  {
    auto seek_flags = 0;
    if (seek_rel < 0)
    {
      seek_flags = AVSEEK_FLAG_BACKWARD;
    }
    auto min_ts = (seek_rel > 0) ? (seek_pos - seek_rel + 2) : (INT64_MIN);
    auto max_ts = (seek_rel < 0) ? (seek_pos - seek_rel - 2) : (seek_pos);
    return demux_->seek(min_ts, seek_pos, max_ts, seek_flags);
  }

}