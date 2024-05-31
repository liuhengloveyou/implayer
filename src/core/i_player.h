#ifndef IMPLAYER_IPLAYER_H
#define IMPLAYER_IPLAYER_H

#include <atomic>
#include <memory>
#include <string>

#include "core/frame.h"
#include "core/types.h"

namespace implayer
{
  class IIMplayer
  {
  public:
    virtual int Run() = 0;
    virtual int open(SourceType source_type, const std::string &path) = 0;
    virtual int play() = 0;
    virtual int stop() = 0;
    virtual int seek(int64_t timestamp) = 0;
    virtual int pause() = 0;
    virtual PlayState state() = 0;
    virtual int64_t getCurrentPosition() = 0;
    virtual void doPauseOrPlaying() = 0;
    virtual void doSeekRelative(float sec) = 0;

    virtual void updateAudioClock(double pts) = 0;
    virtual void updateVideoClock(double pts) = 0;

    virtual std::shared_ptr<Frame> dequeueVideoFrame() = 0;
    virtual std::shared_ptr<Frame> dequeueAudioFrame() = 0;
  };

  using IMPlayerSharedPtr = std::shared_ptr<IIMplayer>;

} // namespace implayer

#endif // IMPLAYER_SIMPLE_PLAYER_H
