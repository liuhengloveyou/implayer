#include "ffmpeg/ffmpeg_demuxer.h"

namespace implayer
{

  int FFmpegDmuxer::open(const std::string &file_path)
  {
    close();

    int ret = avformat_open_input(&format_ctx_, file_path.c_str(), NULL, NULL); // [2]
    RETURN_IF_ERROR(ret);

    ret = avformat_find_stream_info(format_ctx_, NULL);
    RETURN_IF_ERROR(ret);

    findFirstVideoStreamIndex();
    findFirstAudioStreamIndex();
    allocateInternalPacket();

    return 0;
  }

  std::pair<int, AVPacket *> FFmpegDmuxer::readPacket()
  {
    int ret = av_read_frame(format_ctx_, packet_);
    if (ret != 0)
    {
      av_packet_unref(packet_);
    }

    return {ret, packet_};
  }

  void FFmpegDmuxer::dumpFormat() const
  {
    av_dump_format(format_ctx_, 0, format_ctx_->url, 0);
  }

  AVFormatContext *FFmpegDmuxer::getFormatContext() { return format_ctx_; }

  int FFmpegDmuxer::getStreamCount() const
  {
    return static_cast<int>(format_ctx_->nb_streams);
  }

  int FFmpegDmuxer::getVideoStreamIndex() { return video_stream_index_; }

  AVStream *FFmpegDmuxer::getStream(int stream_index) const
  {
    if (stream_index < 0 || stream_index >= getStreamCount())
    {
      return nullptr;
    }
    return format_ctx_->streams[stream_index];
  }

  int FFmpegDmuxer::getAudioStreamIndex() { return audio_stream_index_; }

  int FFmpegDmuxer::seek(int64_t min_ts, int64_t ts, int64_t max_ts, int flags)
  {
    if (!format_ctx_)
    {
      return -1;
    }
    return avformat_seek_file(format_ctx_, -1, min_ts, ts, max_ts, flags);
  }

  void FFmpegDmuxer::close()
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
  void FFmpegDmuxer::findFirstVideoStreamIndex()
  {
    video_stream_index_ = findFirstStreamIndex(AVMEDIA_TYPE_VIDEO);
  }

  void FFmpegDmuxer::findFirstAudioStreamIndex()
  {
    audio_stream_index_ = findFirstStreamIndex(AVMEDIA_TYPE_AUDIO);
  }

  int FFmpegDmuxer::findFirstStreamIndex(AVMediaType target_type)
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

  void FFmpegDmuxer::allocateInternalPacket() { packet_ = av_packet_alloc(); }

}