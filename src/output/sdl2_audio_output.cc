#include "output/sdl2_audio_output.h"

namespace implayer
{

  SDL2AudioOutput::SDL2AudioOutput(IMPlayerSharedPtr player)
      : BaseAudioOutput(player) {}

  SDL2AudioOutput::~SDL2AudioOutput()
  {
    if (audio_device_id_ != 0)
    {
      SDL_CloseAudioDevice(audio_device_id_);
      audio_device_id_ = 0;
    }
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
  }

  int SDL2AudioOutput::prepare(const MediaFileInfo &media_info, const AudioOutputParameters &parameters)
  {
    if (!parameters.isValid())
    {
      LOGE("Invalid audio output parameters");
      return -1;
    }

    int ret = BaseAudioOutput::prepare(media_info, parameters);
    printf("BaseAudioOutput::prepare: %d\n", ret);

    ret = initSDL2AudioSystem(parameters);
    printf("initSDL2AudioSystem: %d\n", ret);

    return ret;
  }

  int SDL2AudioOutput::play()
  {
    if (audio_device_id_ > 0)
    {
      SDL_PauseAudioDevice(audio_device_id_, 0);
    }

    return 0;
  }

  int SDL2AudioOutput::stop()
  {
    if (audio_device_id_ > 0)
    {
      SDL_PauseAudioDevice(audio_device_id_, 1);
    }

    return 0;
  }

  void SDL2AudioOutput::sdl2AudioCallback(void *userdata, Uint8 *stream, int len)
  {
    auto *audio_output = static_cast<SDL2AudioOutput *>(userdata);
    PlayState stat = audio_output->player_->state();
    if (stat == PlayState::kStopped)
    {
      return;
    }
    auto *short_stream = reinterpret_cast<short int *>(stream);
    const int num_samples_of_stream = len / sizeof(int16_t);

    std::fill_n(short_stream, num_samples_of_stream, 0);

    audio_output->pullAudioSamples(short_stream, num_samples_of_stream);
  }

  int SDL2AudioOutput::initSDL2AudioSystem(const AudioOutputParameters &params)
  {

    if (SDL_InitSubSystem(SDL_INIT_AUDIO))
    {
      printf("Could not initialize SDL - %s\n", SDL_GetError());
      return -1;
    }

    SDL_AudioSpec wanted_specs;
    wanted_specs.freq = params.sample_rate;
    wanted_specs.format = AUDIO_S16SYS;
    wanted_specs.channels = params.channels;
    wanted_specs.silence = 0;
    wanted_specs.samples = params.num_frames_of_buffer;
    wanted_specs.callback = SDL2AudioOutput::sdl2AudioCallback;
    wanted_specs.userdata = this;

    audio_device_id_ = SDL_OpenAudioDevice( // [1]
        NULL, 0, &wanted_specs, &audio_spec_, 0);

    if (audio_device_id_ == 0)
    {
      printf("Failed to open audio: %s", SDL_GetError());
      return -1;
    }
    return 0;

    return 0;
  }

}