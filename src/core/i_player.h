#ifndef IMPLAYER_IPLAYER_H
#define IMPLAYER_IPLAYER_H

#include <atomic>
#include <memory>
#include <string>

#include "core/frame.h"

namespace implayer
{
  enum class PlayState : int
  {
    kIdle = 1,
    kStopped,
    kPlaying,
    kSeeking,
    kPaused,
  };

  class IIMplayer
  {
  public:
    virtual int open(std::shared_ptr<IIMplayer> player, const std::string &in_file) = 0;
    virtual int play() = 0;
    virtual int stop() = 0;
    virtual int seek(int64_t timestamp) = 0;
    virtual int pause() = 0;
    virtual PlayState state() = 0;
    virtual int64_t getCurrentPosition() = 0;

    virtual void updateAudioClock(double pts) = 0;
    virtual void updateVideoClock(double pts) = 0;

    virtual std::shared_ptr<Frame> dequeueVideoFrame() = 0;
    virtual std::shared_ptr<Frame> dequeueAudioFrame() = 0;
  };

  using IMPlayerSharedPtr = std::shared_ptr<IIMplayer>;

} // namespace implayer

#endif // IMPLAYER_SIMPLE_PLAYER_H
