#ifndef IMPLAYER_I_SOURCE_H
#define IMPLAYER_I_SOURCE_H

#include <memory>

#include "core/frame.h"
#include "core/media_file_info.h"

namespace implayer
{
  class ISource
  {
  public:
    virtual int open(const std::string &file_path) = 0;
    virtual int seek(int64_t timestamp) = 0;
    virtual MediaFileInfo getMediaFileInfo() = 0;
    virtual int64_t getDuration() = 0;
    virtual int64_t getCurrentPosition() = 0;
    virtual int getQueueSize() = 0;

    virtual std::shared_ptr<Frame> dequeueVideoFrame() = 0;
    virtual std::shared_ptr<Frame> dequeueAudioFrame() = 0;
  };
}

#endif
