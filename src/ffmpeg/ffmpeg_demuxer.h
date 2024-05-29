#ifndef IMPLAYER_FFMPEG_DEMUXER_H
#define IMPLAYER_FFMPEG_DEMUXER_H

#include "ffmpeg_common_utils.h"
#include "ffmpeg_headers.h"
#include <string>

namespace implayer
{
  class FFmpegDmuxer
  {
  public:
    FFmpegDmuxer() = default;
    ~FFmpegDmuxer() { close(); }

  public:
    int open(const std::string &file_path);
    void close();
    std::pair<int, AVPacket *> readPacket();
    void dumpFormat() const;
    AVFormatContext *getFormatContext();
    int getStreamCount() const;
    int getVideoStreamIndex();
    int getAudioStreamIndex();
    AVStream *getStream(int stream_index) const;
    int seek(int64_t min_ts, int64_t ts, int64_t max_ts, int flags);

  private:
    void findFirstVideoStreamIndex();
    void findFirstAudioStreamIndex();
    int findFirstStreamIndex(AVMediaType target_type);
    void allocateInternalPacket();

  private:
    AVFormatContext *format_ctx_{nullptr};
    AVPacket *packet_{nullptr};
    int video_stream_index_{-1};
    int audio_stream_index_{-1};
  };
}

#endif
