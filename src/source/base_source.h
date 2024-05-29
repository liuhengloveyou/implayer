#ifndef IMPLAYER_SIMPLE_SOURCE_H
#define IMPLAYER_SIMPLE_SOURCE_H

#include <memory>

#include "source/i_decoder.h"
#include "source/i_source.h"
#include "utils/waitable_event.h"
#include "utils/waitable_queue.h"
#include "core/i_player.h"
#include "utils/thread_base.h"

namespace implayer
{
  using QueueType = WaitableQueue<std::shared_ptr<Frame>>;
  constexpr int QueueSize = 10;

  class SimpleSource : public ISource, public ThreadBase
  {
  public:
    SimpleSource() = delete;
    SimpleSource(IMPlayerSharedPtr player);
    ~SimpleSource();

  public:
    void threadMain() override;
    int open(const std::string &file_path) override;
    int seek(int64_t timestamp) override;

    MediaFileInfo getMediaFileInfo() override;
    int64_t getDuration() override;
    int64_t getCurrentPosition() override;
    int getQueueSize() override { return video_frame_queue_->size(); }
    std::shared_ptr<Frame> dequeueVideoFrame() override;
    std::shared_ptr<Frame> dequeueAudioFrame() override;

  private:
    std::shared_ptr<Frame> tryPopAFrame(AVMediaType media_type);

  private:
    IMPlayerSharedPtr player_;
    std::shared_ptr<IDecoder> decoder_;
    std::shared_ptr<QueueType> video_frame_queue_ = nullptr;
    std::shared_ptr<QueueType> audio_frame_queue_{nullptr};

    std::atomic<int64_t> seek_timestamp_{0};
    std::atomic<bool> eof{false};
  };

}

#endif
