#ifndef IMPLAYER_PLAYER_H
#define IMPLAYER_PLAYER_H

#include <atomic>
#include <memory>
#include <thread>

#include "core/i_player.h"
#include "core/frame.h"
#include "output/i_video_output.h"
#include "output/i_audio_output.h"
#include "source/i_source.h"
#include "utils/av_synchronizer.h"
#include "utils/thread_base.h"
#include "network/emscripten_websocket.h"
#include "utils/waitable_event.h"

namespace implayer
{
  class IMplayer : public IIMplayer, public ThreadBase, public std::enable_shared_from_this<IMplayer>
  {
  public:
    IMplayer();
    ~IMplayer();

    // ThreadBase
  public:
    void threadMain() override;

    // IIMplayer
  public:
    int Run() override;
    int open(SourceType source_type, const std::string &path) override;
    int play() override;
    int pause() override;
    int stop() override;
    int seek(int64_t timestamp) override;
    PlayState state() override { return state_.load(); }
    int64_t getCurrentPosition() override;
    std::shared_ptr<Frame> dequeueVideoFrame() override;
    std::shared_ptr<Frame> dequeueAudioFrame() override;

  public:
    void updateAudioClock(double pts) override { clock_->setAudioClock(pts); }
    void updateVideoClock(double pts) override
    {
      clock_->setVideoClock(pts);
      auto real_delay_ms = (int)(av_sync_.computeTargetDelay(*clock_) * 1000);
      std::this_thread::sleep_for(std::chrono::milliseconds(real_delay_ms));
    }
    MediaFileInfo getMediaFileInfo();
    int prepareForOutput(const MediaFileInfo &media_file_info, const VideoOutputParameters &v_out_params, const AudioOutputParameters &a_out_params);
    int64_t getDuration() const;
    void doPauseOrPlaying();
    void doSeekRelative(float sec);

  private:
    void OnEvent();
    void HandleOpenEvent(void *data);
    void HandlePlayEvent(void *data);

  private:
    std::shared_ptr<EmscriptenWebsocket> ws_{nullptr};
    std::shared_ptr<IVideoOutput> video_output_{nullptr};
    std::shared_ptr<IAudioOutput> audio_output_{nullptr};
    std::shared_ptr<ISource> source_{nullptr};
    std::shared_ptr<ClockManager> clock_{nullptr};

    std::string path_{""};
    SourceType source_type_{SourceType::UNKNOWN};

    MediaFileInfo media_file_info_{};
    std::atomic<PlayState> state_{PlayState::kIdle};
    AVSynchronizer av_sync_;
    WaitableEvent waitable_;
    std::atomic<bool> opened_{false};
  };

}
#endif
