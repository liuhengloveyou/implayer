#ifndef FFMPEG_AV_DECODER_H_
#define FFMPEG_AV_DECODER_H_

#include <memory>
#include <iostream>
#include <atomic>

#include "source/i_decoder.h"
#include "utils/scope_guard.h"
#include "ffmpeg/ffmpeg_codec.h"
#include "ffmpeg/ffmpeg_demuxer.h"

namespace implayer
{
  class FFmpegAVDecoder : public IDecoder
  {
  public:
    FFmpegAVDecoder() = default;
    ~FFmpegAVDecoder();

  public:
    int open(const std::string &file_path) override;
    std::shared_ptr<Frame> decodeNextFrame() override;
    std::shared_ptr<Frame> seekFrameQuick(int64_t timestamp) override;
    std::shared_ptr<Frame> seekFramePrecise(int64_t timestamp) override;
    int64_t getPosition() override;
    MediaFileInfo getMediaFileInfo() override;

  private:
    int initDemuxer(const std::string &path);
    int initCodec(AVMediaType media_type);
    void updatePosition(const std::shared_ptr<Frame> &frame);
    int seekToTargetPosition(int64_t seek_pos, int64_t seek_rel);
    int seekDemuxerAndFlushCodecBuffer(int64_t timestamp, int64_t target_pos);

  private:
    std::shared_ptr<FFmpegDmuxer> demux_{nullptr};
    std::shared_ptr<FFmpegCodec> video_codec_{nullptr};
    std::shared_ptr<FFmpegCodec> audio_codec_{nullptr};

    int video_stream_index_{-1};
    int audio_stream_index_{-1};
    AVRational video_time_base_;
    AVRational audio_time_base_;
    std::atomic<int64_t> position_{AV_NOPTS_VALUE};
  };
}

#endif
