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
    ~FFmpegAvioDmuxer()
    {
      close();
      adapter_ = nullptr;
    }

    void attachAdapter(DmuxerAdapter* adapter)
    {
      adapter_ = adapter;
    }

  public:
    int open(const std::string &file_path) override;
    void close() override;
    std::pair<int, AVPacket *> readPacket() override;
    void dumpFormat() const override;
    AVFormatContext *getFormatContext() override;
    int getStreamCount() const override;
    int getVideoStreamIndex() override;
    int getAudioStreamIndex() override;
    AVStream *getStream(int stream_index) const override;
    int seek(int64_t min_ts, int64_t ts, int64_t max_ts, int flags) override;

  private:
    void findFirstVideoStreamIndex();
    void findFirstAudioStreamIndex();
    int findFirstStreamIndex(AVMediaType target_type);
    void allocateInternalPacket();

  private:
    DmuxerAdapter* adapter_{nullptr};
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
