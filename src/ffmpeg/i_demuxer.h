#ifndef IMPLAYER_DEMUXER_I_H
#define IMPLAYER_DEMUXER_I_H

#include "ffmpeg_common_utils.h"
#include "ffmpeg_headers.h"
#include <string>

namespace implayer
{
  class IDmuxer
  {
  public:
    virtual int Open(const std::string &path) = 0;
    virtual int Seek(int64_t min_ts, int64_t ts, int64_t max_ts, int flags) = 0;
    virtual std::pair<int, AVPacket *> ReadFrame() = 0;
    virtual AVFormatContext *format_context() const = 0;
    virtual AVStream *stream(int stream_index) const = 0;
    virtual int stream_count() = 0;
    virtual int video_stream_index() = 0;
    virtual int audio_stream_index() = 0;
  };
}

#endif
