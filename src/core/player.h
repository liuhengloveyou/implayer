#ifndef IMPLAYER_PLAYER_H
#define IMPLAYER_PLAYER_H

#include <atomic>
#include <memory>
#include <thread>

#include "output/i_video_output.h"
#include "output/i_audio_output.h"
#include "source/i_source.h"
#include "i_player.h"
#include "core/frame.h"
#include "utils/av_synchronizer.h"

namespace implayer
{
  class IMplayer : public IIMplayer, public std::enable_shared_from_this<IMplayer>
  {
  public:
    IMplayer();
    ~IMplayer();

  public:
    int open(const std::string &path) override;
    int play() override;
    int stop() override;
    int seek(int64_t timestamp) override;
    int pause() override;
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
    std::shared_ptr<IVideoOutput> video_output_{nullptr};
    std::shared_ptr<IAudioOutput> audio_output_{nullptr};
    std::shared_ptr<ISource> source_{nullptr};
    std::shared_ptr<ClockManager> clock_{nullptr};

    MediaFileInfo media_file_info_{};
    std::atomic<PlayState> state_{PlayState::kIdle};
    AVSynchronizer av_sync_;
  };

} // namespace implayer

#endif // IMPLAYER_SIMPLE_PLAYER_H
