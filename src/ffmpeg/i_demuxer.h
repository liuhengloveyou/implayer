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
    virtual int open(const std::string &file_path) = 0;
    virtual void close() = 0;
    virtual std::pair<int, AVPacket *> readPacket() = 0;
    virtual void dumpFormat() const = 0;
    virtual AVFormatContext *getFormatContext() = 0;
    virtual int getStreamCount() const = 0;
    virtual int getVideoStreamIndex() = 0;
    virtual int getAudioStreamIndex() = 0;
    virtual AVStream *getStream(int stream_index) const = 0;
    virtual int seek(int64_t min_ts, int64_t ts, int64_t max_ts, int flags) = 0;
  };
}

#endif
