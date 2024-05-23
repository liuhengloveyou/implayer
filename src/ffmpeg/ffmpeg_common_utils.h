#ifndef FFMPEG_VIDEO_PLAYER_FFMPEG_COMMON_UTILS_H
#define FFMPEG_VIDEO_PLAYER_FFMPEG_COMMON_UTILS_H

#define LOGE(...) printf(__VA_ARGS__)
#define LOGW(...) printf(__VA_ARGS__)
#define LOGD(...) printf(__VA_ARGS__)

#define RETURN_IF_ERROR(ret)                                                   \
  if ((ret) < 0) {                                                             \
    return (ret);                                                              \
  }

#define RETURN_IF_ERROR_LOG(ret, ...)                                          \
  if ((ret) < 0) {                                                             \
    LOGE(__VA_ARGS__);                                                         \
    return (ret);                                                              \
  }

#endif // FFMPEG_VIDEO_PLAYER_FFMPEG_COMMON_UTILS_H
