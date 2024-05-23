#ifndef IMPLAYER_SDL2_AUDIO_OUTPUT_H
#define IMPLAYER_SDL2_AUDIO_OUTPUT_H

#include <SDL.h>

#include "ffmpeg/ffmpeg_common_utils.h"
#include "output/base_audio_output.h"

namespace implayer
{
  class SDL2AudioOutput : public BaseAudioOutput
  {
  public:
    SDL2AudioOutput(IMPlayerSharedPtr player);
    ~SDL2AudioOutput();

  public:
    int prepare(const MediaFileInfo &media_info, const AudioOutputParameters &parameters) override;
    int play() override;
    int stop() override;

  private:
    static void sdl2AudioCallback(void *userdata, Uint8 *stream, int len);
    int initSDL2AudioSystem(const AudioOutputParameters &params);

  private:
    SDL_AudioDeviceID audio_device_id_{0};
    SDL_AudioSpec audio_spec_{};
  };

}

#endif
