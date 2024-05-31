#include "ffmpeg/ffmpeg_base_demuxer.h"

namespace implayer
{
  FFmpegBaseDmuxer::~FFmpegBaseDmuxer()
  {
    if (format_ctx_)
    {
      avformat_close_input(&format_ctx_);
    }

    if (packet_)
    {
      av_packet_unref(packet_);
      av_packet_free(&packet_);
    }
  }

  int FFmpegBaseDmuxer::Open(const std::string &path)
  {
    int ret = avformat_open_input(&format_ctx_, path.c_str(), NULL, NULL);
    RETURN_IF_ERROR(ret);

    ret = avformat_find_stream_info(format_ctx_, NULL);
    RETURN_IF_ERROR(ret);

    packet_ = av_packet_alloc();

    findFirstVideoStreamIndex();
    findFirstAudioStreamIndex();

    DumpFormat();
    
    return 0;
  }

  std::pair<int, AVPacket *> FFmpegBaseDmuxer::ReadFrame()
  {
    int ret = av_read_frame(format_ctx_, packet_);
    if (ret != 0)
    {
      av_packet_unref(packet_);
    }

    return {ret, packet_};
  }

  void FFmpegBaseDmuxer::DumpFormat()
  {
    av_dump_format(format_ctx_, 0, format_ctx_->url, 0);
  }

  AVFormatContext *FFmpegBaseDmuxer::format_context() const { return format_ctx_; }

  int FFmpegBaseDmuxer::stream_count()
  {
    return static_cast<int>(format_ctx_->nb_streams);
  }

  AVStream *FFmpegBaseDmuxer::stream(int stream_index) const
  {
    if (stream_index < 0)
    {
      return nullptr;
    }
    return format_ctx_->streams[stream_index];
  }
  int FFmpegBaseDmuxer::video_stream_index() { return video_stream_index_; }

  int FFmpegBaseDmuxer::audio_stream_index() { return audio_stream_index_; }

  int FFmpegBaseDmuxer::Seek(int64_t min_ts, int64_t ts, int64_t max_ts, int flags)
  {
    if (!format_ctx_)
    {
      return -1;
    }
    return avformat_seek_file(format_ctx_, -1, min_ts, ts, max_ts, flags);
  }

  void FFmpegBaseDmuxer::findFirstVideoStreamIndex()
  {
    video_stream_index_ = findFirstStreamIndex(AVMEDIA_TYPE_VIDEO);
  }

  void FFmpegBaseDmuxer::findFirstAudioStreamIndex()
  {
    audio_stream_index_ = findFirstStreamIndex(AVMEDIA_TYPE_AUDIO);
  }

  int FFmpegBaseDmuxer::findFirstStreamIndex(AVMediaType target_type)
  {
    for (auto i = 0u; i < format_ctx_->nb_streams; ++i)
    {
      if (format_ctx_->streams[i]->codecpar->codec_type == target_type)
      {
        return i;
      }
    }
    return -1;
  }

}