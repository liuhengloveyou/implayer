#ifndef IMPLAYER_FFMPEG_AVIO_DEMUXER_H
#define IMPLAYER_FFMPEG_AVIO_DEMUXER_H

#include <string>
#include <memory>

#include "ffmpeg_common_utils.h"
#include "ffmpeg_headers.h"
#include "ffmpeg/i_demuxer.h"

namespace implayer
{
  class DmuxerAdapter
  {
  public:
    virtual int ReadPacket(void *opaque, uint8_t *buf, int buf_size) = 0;
  };

  class FFmpegAvioDmuxer : public IDmuxer
  {
  public:
    FFmpegAvioDmuxer(){};
    ~FFmpegAvioDmuxer();

    void attachAdapter(DmuxerAdapter *adapter)
    {
      adapter_ = adapter;
    }

    int ReadPacket(void *opaque, uint8_t *buf, int buf_size)
    {
      if (adapter_)
      {
        return adapter_->ReadPacket(opaque, buf, buf_size);
      }

      return 0;
    }

  public:
    int Open(const std::string &file_path) override;
    int Seek(int64_t min_ts, int64_t ts, int64_t max_ts, int flags) override;
    std::pair<int, AVPacket *> ReadFrame() override;
    AVFormatContext *format_context() const override;
    AVStream *stream(int stream_index) const override;
    int stream_count() override;
    int video_stream_index() override;
    int audio_stream_index() override;

  private:
    void findFirstVideoStreamIndex();
    void findFirstAudioStreamIndex();
    int findFirstStreamIndex(AVMediaType target_type);

  private:
    DmuxerAdapter *adapter_{nullptr};
    AVFormatContext *format_ctx_{nullptr};
    AVPacket *packet_{nullptr};
    int video_stream_index_{-1};
    int audio_stream_index_{-1};

  private:
    AVIOContext *avio_ctx_ = NULL;
    uint8_t *avio_ctx_buffer_ = NULL;
    size_t avio_ctx_buffer_size_ = 4096;
  };
}

#endif
