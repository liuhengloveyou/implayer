#ifndef IMPLAYER_FFMPEG_BASE_DEMUXER_H
#define IMPLAYER_FFMPEG_BASE_DEMUXER_H

#include <string>

#include "ffmpeg_common_utils.h"
#include "ffmpeg_headers.h"
#include "ffmpeg/i_demuxer.h"

namespace implayer
{
  class FFmpegBaseDmuxer : public IDmuxer
  {
  public:
    FFmpegBaseDmuxer() = default;
    ~FFmpegBaseDmuxer();

    void DumpFormat();

  public:
    int Open(const std::string &path) override;
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
    AVFormatContext *format_ctx_{nullptr};
    AVPacket *packet_{nullptr};
    int video_stream_index_{-1};
    int audio_stream_index_{-1};
  };
}

#endif
