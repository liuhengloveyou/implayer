#ifndef IMPLAYER_BASE_SOURCE_H
#define IMPLAYER_BASE_SOURCE_H

#include <memory>

#include "utils/scope_guard.h"
#include "core/i_player.h"
#include "source/i_source.h"
#include "utils/waitable_event.h"
#include "utils/waitable_queue.h"
#include "utils/thread_base.h"
#include <ffmpeg/ffmpeg_codec.h>

namespace implayer
{
  using QueueType = WaitableQueue<std::shared_ptr<Frame>>;
  constexpr int QueueSize = 10;

  class BaseSource : public ISource, public ThreadBase
  {
  public:
    BaseSource() = delete;
    BaseSource(IMPlayerSharedPtr player);
    ~BaseSource();

  public:
    void threadMain() override;

  protected:
    virtual std::pair<int, AVPacket *> ReadPacket();

  protected:
    int Open(const std::string &path) override;
    int Seek(int64_t timestamp) override;
    std::shared_ptr<Frame> SeekFrameQuick(int64_t timestamp) override;
    std::shared_ptr<Frame> SeekFramePrecise(int64_t timestamp) override;
    std::shared_ptr<Frame> NextVideoFrame() override;
    std::shared_ptr<Frame> NextAudioFrame() override;
    int64_t duration() override;
    int queue_size() override;
    int64_t position() override;

  protected:
    void UpdatePosition(const std::shared_ptr<Frame> &frame);
    int SeekToTargetPosition(int64_t seek_pos, int64_t seek_rel);
    int SeekDemuxerAndFlushCodecBuffer(int64_t timestamp, int64_t target_pos);

  protected:
    IMPlayerSharedPtr player_;
    std::shared_ptr<FFmpegCodec> video_codec_{nullptr};
    std::shared_ptr<FFmpegCodec> audio_codec_{nullptr};
    std::shared_ptr<QueueType> video_frame_queue_ = nullptr;
    std::shared_ptr<QueueType> audio_frame_queue_{nullptr};

    AVRational time_base_;
    std::atomic<int64_t> position_{AV_NOPTS_VALUE};
    std::atomic<int64_t> seek_timestamp_{0};
    std::atomic<bool> eof_{false};

  private:
    int initVideoCodec();
    int initAudioCodec();
    std::shared_ptr<Frame> decodeNextFrame();
  };

}

#endif
