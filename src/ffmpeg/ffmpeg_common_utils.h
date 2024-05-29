#ifndef IMPLAYER_FFMPEG_COMMON_UTILS_H
#define IMPLAYER_FFMPEG_COMMON_UTILS_H

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

#endif // IMPLAYER_FFMPEG_COMMON_UTILS_H
