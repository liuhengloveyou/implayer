#ifndef IMGUI_SDL2_OUTPUT_H_
#define IMGUI_SDL2_OUTPUT_H_

#include <map>
#include <list>
#include <mutex>
#include <memory>

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"
#include <SDL2/SDL.h>

#if !SDL_VERSION_ATLEAST(2, 0, 17)
#error This backend requires SDL 2.0.17+ because of SDL_RenderGeometry() function
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#include <libavutil/avassert.h>
#include <libavutil/channel_layout.h>
#include <libavutil/opt.h>
#include <libavutil/mathematics.h>
#include <libavutil/timestamp.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavutil/time.h>
#include <libavutil/imgutils.h>

#ifdef __cplusplus
}
#endif

#include "core/media_file_info.h"
#include "core/frame.h"
#include "core/player.h"
#include "core/i_player.h"
#include "output/base_video_output.h"

#define SDL_EVENT_PLAY (SDL_USEREVENT + 1)
#define SDL_EVENT_REFRESH (SDL_USEREVENT + 2)
#define SDL_EVENT_REFRESH_AUDIO (SDL_USEREVENT + 3)
#define SDL_EVENT_RESIZE (SDL_USEREVENT + 4)

namespace implayer
{

    class IMSDL2Output : public BaseVideoOutput
    {
    public:
        IMSDL2Output() = delete;
        explicit IMSDL2Output(IMPlayerSharedPtr player);
        ~IMSDL2Output();

    public:
        int prepare(const MediaFileInfo &media_info, const VideoOutputParameters &parameters) override;
        int updateFrame(std::shared_ptr<Frame> frame) override;
        int play() override;

        void run();

    private:
        void onResizeEvent();
        void onPlayEvent(void *data);
        void onUpdateFrame(void *data);

    private:
        int InitImSDL2(const VideoOutputParameters &parameters);
        void onEvent();
        void render();

    private:
        SDL_Window *m_sdlWindow{nullptr};
        SDL_Renderer *m_sdlRender{nullptr};
        SDL_Texture *m_sdlTexture{nullptr};

    private:
        std::shared_ptr<Frame> frame_for_draw_{nullptr};
        int window_width{0};
        int window_height{0};
    };
}
#endif
