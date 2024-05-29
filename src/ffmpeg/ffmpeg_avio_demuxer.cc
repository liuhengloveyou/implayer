#include "ffmpeg/ffmpeg_avio_demuxer.h"

namespace implayer
{
  struct buffer_data
  {
    uint8_t *ptr;
    size_t size; ///< size left in the buffer
  };

  static int read_packet(void *opaque, uint8_t *buf, int buf_size)
  {
    struct buffer_data *bd = (struct buffer_data *)opaque;
    printf("ptr:%p size:%zu %d\n", bd->ptr, bd->size, buf_size);

    buf_size = FFMIN(buf_size, bd->size);

    if (!buf_size)
      return AVERROR_EOF;

    // /* copy internal buffer data to buf */
    memcpy(buf, bd->ptr, buf_size);
    bd->ptr += buf_size;
    bd->size -= buf_size;

    return buf_size;
  }

  int FFmpegAvioDmuxer::open(const std::string &url)
  {
    close();

    struct buffer_data bd = {0};
    uint8_t *buffer = NULL;
    size_t buffer_size = 0;

    /* slurp file content into buffer */
    int ret = av_file_map(url.c_str(), &buffer, &buffer_size, 0, NULL);
    if (ret < 0)
    {
      printf("av_file_map err: %d\n", ret);
      return ret;
    }
    printf(">>>>>>>>>>>>%ld\n", buffer_size);

    /* fill opaque structure used by the AVIOContext read callback */
    bd.ptr = buffer;
    bd.size = buffer_size;

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
                                   0, &bd, &read_packet, NULL, NULL);
    if (!avio_ctx_)
    {
      return AVERROR(ENOMEM);
    }
    format_ctx_->pb = avio_ctx_;

    ret = avformat_open_input(&format_ctx_, NULL, NULL, NULL);
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

    av_dump_format(format_ctx_, 0, url.c_str(), 0);

    findFirstVideoStreamIndex();
    findFirstAudioStreamIndex();
    allocateInternalPacket();

    return 0;
  }

  std::pair<int, AVPacket *> FFmpegAvioDmuxer::readPacket()
  {
    int ret = av_read_frame(format_ctx_, packet_);
    if (ret != 0)
    {
      av_packet_unref(packet_);
    }

    return {ret, packet_};
  }

  void FFmpegAvioDmuxer::dumpFormat() const
  {
    av_dump_format(format_ctx_, 0, format_ctx_->url, 0);
  }

  AVFormatContext *FFmpegAvioDmuxer::getFormatContext() { return format_ctx_; }

  int FFmpegAvioDmuxer::getStreamCount() const
  {
    return static_cast<int>(format_ctx_->nb_streams);
  }

  int FFmpegAvioDmuxer::getVideoStreamIndex() { return video_stream_index_; }

  AVStream *FFmpegAvioDmuxer::getStream(int stream_index) const
  {
    if (stream_index < 0 || stream_index >= getStreamCount())
    {
      return nullptr;
    }
    return format_ctx_->streams[stream_index];
  }

  int FFmpegAvioDmuxer::getAudioStreamIndex() { return audio_stream_index_; }

  int FFmpegAvioDmuxer::seek(int64_t min_ts, int64_t ts, int64_t max_ts, int flags)
  {
    if (!format_ctx_)
    {
      return -1;
    }
    return avformat_seek_file(format_ctx_, -1, min_ts, ts, max_ts, flags);
  }

  void FFmpegAvioDmuxer::close()
  {
    if (format_ctx_)
    {
      avformat_close_input(&format_ctx_);
    }

    /* note: the internal buffer could have changed, and be != avio_ctx_buffer */
    if (avio_ctx_)
      av_freep(&avio_ctx_->buffer);
    avio_context_free(&avio_ctx_);

    if (packet_)
    {
      av_packet_unref(packet_);
      av_packet_free(&packet_);
    }
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

  void FFmpegAvioDmuxer::allocateInternalPacket() { packet_ = av_packet_alloc(); }

}