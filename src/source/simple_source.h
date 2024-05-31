#ifndef IMPLAYER_SIMPLE_SOURCE_H
#define IMPLAYER_SIMPLE_SOURCE_H

#include <memory>

#include "ffmpeg/ffmpeg_base_demuxer.h"
#include "utils/thread_base.h"
#include "source/base_source.h"
#include "utils/waitable_event.h"
#include "utils/waitable_queue.h"
#include "core/i_player.h"

namespace implayer
{
  class SimpleSource : public BaseSource
  {
  public:
    SimpleSource() = delete;
    SimpleSource(IMPlayerSharedPtr player);
    ~SimpleSource();

  public:
    int Open(const std::string &path) override;
    int Seek(int64_t timestamp) override;
    std::shared_ptr<Frame> NextVideoFrame() override;
    std::shared_ptr<Frame> NextAudioFrame() override;
    MediaFileInfo media_info() const override;
    AVStream *stream(int stream_index) const override;
    int video_stream_index() override;
    int audio_stream_index() override;
    int64_t duration() override;
    int64_t position() override;
    std::pair<int, AVPacket *> ReadPacket() override;

  private:
    std::shared_ptr<Frame> tryPopAFrame(AVMediaType media_type);

  private:
    std::shared_ptr<FFmpegBaseDmuxer> demux_{nullptr};
    //   IMPlayerSharedPtr player_;
    //   std::shared_ptr<QueueType> video_frame_queue_ = nullptr;
    //   std::shared_ptr<QueueType> audio_frame_queue_{nullptr};

    //   std::atomic<int64_t> seek_timestamp_{0};
    //   std::atomic<bool> eof{false};
  };

}

#endif
