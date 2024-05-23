#ifndef IMPLAYER_BASE_VIDEO_OUTPUT_H
#define IMPLAYER_BASE_VIDEO_OUTPUT_H

#include "core/i_player.h"
#include "output/i_video_output.h"
#include "ffmpeg/ffmpeg_image_converter.h"
#include "utils/thread_base.h"
#include "utils/av_synchronizer.h"

namespace implayer
{
  class BaseVideoOutput : public IVideoOutput, public ThreadBase
  {
  public:
    BaseVideoOutput() = delete;
    explicit BaseVideoOutput(IMPlayerSharedPtr player);
    ~BaseVideoOutput();

  public:
    void threadMain() override;
    int prepare(const MediaFileInfo &media_info, const VideoOutputParameters &parameters) override;

  protected:
    int prepareImageConverter(const MediaFileInfo &src_media_info, const VideoOutputParameters &v_out_params);
    virtual int drawFrame(std::shared_ptr<Frame> frame) = 0;
    std::shared_ptr<Frame> convertFrame(std::shared_ptr<Frame> frame);
    void doAVSync(double pts_d);

    IMPlayerSharedPtr player_{nullptr};
    std::shared_ptr<FFMPEGImageConverter> image_converter_{nullptr};
    std::shared_ptr<FFMPEGImageConverter> converter_;
    VideoOutputParameters parameters_;
  };

}

#endif
