#ifndef IMPLAYER_FRAME_H
#define IMPLAYER_FRAME_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <libavformat/avformat.h>

#ifdef __cplusplus
}
#endif

namespace implayer
{
  class Frame
  {
  public:
    explicit Frame(AVRational tb) : f(av_frame_alloc()), time_base(tb) {}
    explicit Frame(AVFrame *frame, AVRational tb)
        : f(av_frame_clone(frame)), time_base(tb) {}

    ~Frame()
    {
      if (f)
      {
        av_frame_unref(f);
        av_frame_free(&f);
      }
    }

    /**
     * get the pts of the frame, based on AV_TIME_BASE
     * @return the pts of the frame
     */
    int64_t pts() const
    {
      return av_rescale_q(f->pts, time_base, AV_TIME_BASE_Q);
    }

    /**
     * get the pts(seconds) of the frame
     */
    double pts_d() const
    {
      return pts() / (double)AV_TIME_BASE;
    }

    bool isVideo() { return media_type == AVMEDIA_TYPE_VIDEO; }
    bool isAudio() { return media_type == AVMEDIA_TYPE_AUDIO; }

    AVFrame *f{nullptr};
    AVRational time_base;
    AVMediaType media_type;
  };

} // namespace implayer

#endif // IMPLAYER_FRAME_H
