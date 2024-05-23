#ifndef FFMPEG_VIDEO_PLAYER_FFMPEG_HEADERS_H
#define FFMPEG_VIDEO_PLAYER_FFMPEG_HEADERS_H

#ifdef __cplusplus
extern "C" {
#endif

#define __STDC_CONSTANT_MACROS

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libavutil/time.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>

#ifdef __cplusplus
}
#endif

#endif
