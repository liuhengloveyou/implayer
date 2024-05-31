#ifndef IMPLAYER_I_SOURCE_H
#define IMPLAYER_I_SOURCE_H

#include <memory>

#include "core/frame.h"
#include "core/media_file_info.h"
#include "core/i_player.h"

namespace implayer
{
  class ISource
  {
  public:
    virtual int Open(const std::string &path) = 0;
    virtual std::shared_ptr<Frame> NextVideoFrame() = 0;
    virtual std::shared_ptr<Frame> NextAudioFrame() = 0;
    virtual int Seek(int64_t timestamp) = 0;
    virtual std::shared_ptr<Frame> SeekFrameQuick(int64_t timestamp) = 0;
    virtual std::shared_ptr<Frame> SeekFramePrecise(int64_t timestamp) = 0;

    virtual MediaFileInfo media_info() const = 0;
    virtual AVStream *stream(int stream_index) const = 0;
    virtual int video_stream_index() = 0;
    virtual int audio_stream_index() = 0;
    virtual int64_t duration() = 0;
    virtual int64_t position() = 0;
    virtual int queue_size() = 0;
  };
}

#endif
