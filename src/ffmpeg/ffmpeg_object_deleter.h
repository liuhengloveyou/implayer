//
// Created by user on 7/26/22.
//

#ifndef IMPLAYER_FFMPEG_OBJECT_DELETER_H
#define IMPLAYER_FFMPEG_OBJECT_DELETER_H

#pragma once
#include "ffmpeg/ffmpeg_headers.h"

namespace implayer {
class FrameDeleter {
public:
  void operator()(AVFrame *f) const {
    if (f != nullptr) {
      av_frame_unref(f);
      av_frame_free(&f);
    }
  };
};

class PacketDeleter {
public:
  void operator()(AVPacket *p) const {
    if (p != nullptr) {
      av_packet_unref(p);
      av_packet_free(&p);
    }
  };
};

} // namespace implayer

#endif // IMPLAYER_FFMPEG_OBJECT_DELETER_H
