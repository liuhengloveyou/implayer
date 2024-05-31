#include "ffmpeg/ffmpeg_avio_demuxer.h"

namespace implayer
{

  static int avio_read_packet(void *opaque, uint8_t *buf, int buf_size)
  {
    FFmpegAvioDmuxer *dmuxer = (FFmpegAvioDmuxer *)opaque;
    return dmuxer->ReadPacket(opaque, buf, buf_size);
  }

  FFmpegAvioDmuxer::~FFmpegAvioDmuxer()
  {
    adapter_ = nullptr;

    if (format_ctx_)
    {
      avformat_close_input(&format_ctx_);
    }

    /* note: the internal buffer could have changed, and be != avio_ctx_buffer */
    if (avio_ctx_)
    {
      av_freep(&avio_ctx_->buffer);
      avio_context_free(&avio_ctx_);
    }

    if (packet_)
    {
      av_packet_unref(packet_);
      av_packet_free(&packet_);
    }
  }

  int FFmpegAvioDmuxer::Open(const std::string &url)
  {
    format_ctx_ = avformat_alloc_context();
    if (format_ctx_ == NULL)
    {
      return AVERROR(ENOMEM);
    }

    avio_ctx_buffer_ = (uint8_t *)av_malloc(avio_ctx_buffer_size_);
    if (!avio_ctx_buffer_)
    {
      return AVERROR(ENOMEM);
    }
    avio_ctx_ = avio_alloc_context(avio_ctx_buffer_, avio_ctx_buffer_size_,
                                   0, this, &avio_read_packet, NULL, NULL);
    if (!avio_ctx_)
    {
      return AVERROR(ENOMEM);
    }
    format_ctx_->pb = avio_ctx_;

    int ret = avformat_open_input(&format_ctx_, NULL, NULL, NULL);
    if (ret < 0)
    {
      fprintf(stderr, "Could not open input\n");
      return ret;
    }

    ret = avformat_find_stream_info(format_ctx_, NULL);
    if (ret < 0)
    {
      fprintf(stderr, "Could not find stream information\n");
      return ret;
    }

    packet_ = av_packet_alloc();

    av_dump_format(format_ctx_, 0, url.c_str(), 0);

    findFirstVideoStreamIndex();
    findFirstAudioStreamIndex();

    return 0;
  }

  std::pair<int, AVPacket *> FFmpegAvioDmuxer::ReadFrame()
  {
    int ret = av_read_frame(format_ctx_, packet_);
    if (ret != 0)
    {
      av_packet_unref(packet_);
    }

    return {ret, packet_};
  }

  AVFormatContext *FFmpegAvioDmuxer::format_context() const { return format_ctx_; }

  int FFmpegAvioDmuxer::stream_count()
  {
    return static_cast<int>(format_ctx_->nb_streams);
  }

  int FFmpegAvioDmuxer::video_stream_index() { return video_stream_index_; }

  AVStream *FFmpegAvioDmuxer::stream(int stream_index) const
  {
    if (stream_index < 0)
    {
      return nullptr;
    }
    return format_ctx_->streams[stream_index];
  }

  int FFmpegAvioDmuxer::audio_stream_index() { return audio_stream_index_; }

  int FFmpegAvioDmuxer::Seek(int64_t min_ts, int64_t ts, int64_t max_ts, int flags)
  {
    if (!format_ctx_)
    {
      return -1;
    }
    return avformat_seek_file(format_ctx_, -1, min_ts, ts, max_ts, flags);
  }

  void FFmpegAvioDmuxer::findFirstVideoStreamIndex()
  {
    video_stream_index_ = findFirstStreamIndex(AVMEDIA_TYPE_VIDEO);
  }

  void FFmpegAvioDmuxer::findFirstAudioStreamIndex()
  {
    audio_stream_index_ = findFirstStreamIndex(AVMEDIA_TYPE_AUDIO);
  }

  int FFmpegAvioDmuxer::findFirstStreamIndex(AVMediaType target_type)
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