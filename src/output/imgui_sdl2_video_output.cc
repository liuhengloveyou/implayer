#include <map>
#include <list>
#include <mutex>

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"
#include <SDL2/SDL.h>

#ifdef __EMSCRIPTEN__
#include <pthread.h>
#include <emscripten.h>
#include <emscripten/bind.h>
#include "emscripten/emscripten_mainloop_stub.h"
using namespace emscripten;
#endif

#include "core/frame.h"
#include "core/i_player.h"
#include "output/imgui_sdl2_video_output.h"
#include "utils/time_utils.h"

#if !SDL_VERSION_ATLEAST(2, 0, 17)
#error This backend requires SDL 2.0.17+ because of SDL_RenderGeometry() function
#endif

namespace implayer
{
  IMSDL2Output::IMSDL2Output(IMPlayerSharedPtr player)
      : BaseVideoOutput(player)
  {
  }

  IMSDL2Output::~IMSDL2Output()
  {
    frame_for_draw_ = nullptr;

    // Cleanup
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    if (m_sdlTexture)
    {
      SDL_DestroyTexture(m_sdlTexture);
      m_sdlTexture = nullptr;
    }

    if (m_sdlRender)
    {
      SDL_DestroyRenderer(m_sdlRender);
      m_sdlRender = nullptr;
    }

    if (m_sdlWindow)
    {
      SDL_DestroyWindow(m_sdlWindow);
      m_sdlWindow = nullptr;
    }

    SDL_QuitSubSystem(SDL_INIT_VIDEO);
    SDL_Quit();
  }

  int IMSDL2Output::prepare(const MediaFileInfo &media_info, const VideoOutputParameters &parameters)
  {
    auto ret = BaseVideoOutput::prepare(media_info, parameters);
    printf("BaseVideoOutput::prepare: %d\n", ret);

    ret = InitImSDL2(parameters);
    printf("initSDL2System: %d\n", ret);

    return ret;
  }

  int IMSDL2Output::play()
  {

    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    // Our state
    ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 1.00f);

    auto t1 = getTimestamp();
    //     // Main loop
    //     bool done = false;
    // #ifdef __EMSCRIPTEN__
    //     // For an Emscripten build we are disabling file-system access, so let's not attempt to do a fopen() of the imgui.ini file.
    //     // You may manually call LoadIniSettingsFromMemory() to load settings from your own storage.
    //     io.IniFilename = nullptr;
    //     EMSCRIPTEN_MAINLOOP_BEGIN
    // #else
    //     while (!done)
    // #endif
    //     {
    //       HandleEvent();

    // Start the Dear ImGui frame
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
    {
      static float f = 0.0f;
      static int counter = 0;

      ImGui::Begin("Player");                                  // Create a window called "Hello, world!" and append into it.
      ImGui::Text("This is some useful text.");                // Display some text (you can use a format strings too)
      ImGui::SliderFloat("float", &f, 0.0f, 1.0f);             // Edit 1 float using a slider from 0.0f to 1.0f
      ImGui::ColorEdit3("clear color", (float *)&clear_color); // Edit 3 floats representing a color

      if (ImGui::Button("Button")) // Buttons return true when clicked (most widgets return true when edited/activated)
        counter++;
      ImGui::SameLine();
      ImGui::Text("counter = %d", counter);

      ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
      ImGui::End();
    }

    // Rendering
    ImGui::Render();
    SDL_RenderSetScale(m_sdlRender, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
    SDL_SetRenderDrawColor(m_sdlRender, (Uint8)(clear_color.x * 255), (Uint8)(clear_color.y * 255), (Uint8)(clear_color.z * 255), (Uint8)(clear_color.w * 255));
    SDL_RenderClear(m_sdlRender);
    render();
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), m_sdlRender);
    SDL_RenderPresent(m_sdlRender);
    // auto t2 = getTimestamp();
    // printf(">>>>>>>>>>>>>>>>>>>>>%lld\n", t2 - t1);
    // t1 = t2;
    //     }
    // #ifdef __EMSCRIPTEN__
    //     EMSCRIPTEN_MAINLOOP_END;
    // #endif

    // Cleanup
    // ImGui_ImplSDLRenderer2_Shutdown();
    // ImGui_ImplSDL2_Shutdown();
    // ImGui::DestroyContext();

    // SDL_DestroyRenderer(m_sdlRender);
    // SDL_DestroyWindow(m_sdlWindow);
    // SDL_Quit();
  }

  void IMSDL2Output::HandleEvent()
  {
    SDL_Event event;
    while (SDL_PollEvent(&event))
    // while (SDL_WaitEventTimeout(&event, 20))
    {

      ImGui_ImplSDL2_ProcessEvent(&event);

      switch (event.type)
      {
      case SDL_WINDOWEVENT:
      {
        if (event.window.event == SDL_WINDOWEVENT_RESIZED)
        {
          onResizeEvent();
        }
        else if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(m_sdlWindow))
        {
        }
      }
      break;
      case SDL_EVENT_PLAY:
      {
        onPlayEvent(event.user.data1);
      }
      break;
      case SDL_EVENT_REFRESH:
      {
        onUpdateFrame(event.user.data1);
      }
      case SDL_KEYDOWN:
      {
        switch (event.key.keysym.sym)
        {
        case SDLK_LEFT:
        {
          player_->doSeekRelative(-5.0);
          break;
        }

        case SDLK_RIGHT:
        {
          player_->doSeekRelative(5.0);
          break;
        }

        case SDLK_DOWN:
        {
          player_->doSeekRelative(-60.0);
          break;
        }

        case SDLK_UP:
        {
          player_->doSeekRelative(60.0);
          break;
        }
        case SDLK_SPACE:
        {
          player_->doPauseOrPlaying();
          break;
        }
        default:
          break;
        }
        break;
      }
      break;
      }
    }
  }

  void IMSDL2Output::render()
  {
    if (frame_for_draw_ != nullptr)
    {
      AVFrame *pict = frame_for_draw_->f;
      SDL_UpdateYUVTexture(m_sdlTexture, nullptr,
                           pict->data[0], pict->linesize[0],
                           pict->data[1], pict->linesize[1],
                           pict->data[2], pict->linesize[2]);

      // SDL_RenderClear(m_sdlRender);
      SDL_RenderCopy(m_sdlRender,  // the rendering context
                     m_sdlTexture, // the source texture
                     NULL,         // the source SDL_Rect structure or NULL for the
                                   // entire texture
                     NULL          // the destination SDL_Rect structure or NULL for
                                   // the entire rendering target; the texture will
                                   // be stretched to fill the given rectangle
      );
      // SDL_RenderPresent(m_sdlRender);
      // doAVSync(frame_for_draw_->pts_d());
    }
  }

  int IMSDL2Output::updateFrame(FrameSharedPtr frame)
  {
    FrameEvent *data = new FrameEvent{};
    data->frame = frame;

    SDL_Event event;
    event.type = SDL_EVENT_REFRESH;
    event.user.data1 = data;
    SDL_PushEvent(&event);

    return 0;
  }

  void IMSDL2Output::onResizeEvent()
  {
    SDL_GetWindowSize(m_sdlWindow, &window_width, &window_height);
  }

  void IMSDL2Output::onPlayEvent(void *data)
  {
    // player_->play();

    return;
  }

  void IMSDL2Output::onUpdateFrame(void *data)
  {
    FrameEvent *ev = (FrameEvent *)data;
    frame_for_draw_ = std::move(ev->frame);
    delete ev;

    return;
  }

  int IMSDL2Output::InitImSDL2(const VideoOutputParameters &parameters)
  {
    // Setup SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
      printf("Error: %s\n", SDL_GetError());
      return -1;
    }

    // From 2.0.18: Enable native IME.
#ifdef SDL_HINT_IME_SHOW_UI
    SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

// Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    const char *glsl_version = "#version 100";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(__APPLE__)
    // GL 3.2 Core + GLSL 150
    const char *glsl_version = "#version 150";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
    // GL 3.0 + GLSL 130
    const char *glsl_version = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

    // From 2.0.18: Enable native IME.
#ifdef SDL_HINT_IME_SHOW_UI
    SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

    // Create window with graphics context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    // Create window with SDL_Renderer graphics context
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    m_sdlWindow = SDL_CreateWindow("implayer",
                                   SDL_WINDOWPOS_CENTERED,
                                   SDL_WINDOWPOS_CENTERED,
                                   parameters.width,
                                   parameters.height,
                                   window_flags);
    if (m_sdlWindow == nullptr)
    {
      printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
      return -1;
    }

    m_sdlRender = SDL_CreateRenderer(m_sdlWindow, -1, SDL_RENDERER_ACCELERATED);
    if (m_sdlRender == nullptr)
    {
      SDL_Log("Error creating SDL_Renderer!");
      return 0;
    }
    // SDL_RendererInfo info;
    // SDL_GetRendererInfo(renderer, &info);
    // SDL_Log("Current SDL_Renderer: %s", info.name);
    m_sdlTexture = SDL_CreateTexture(m_sdlRender,
                                     SDL_PIXELFORMAT_IYUV,
                                     SDL_TEXTUREACCESS_STREAMING,
                                     parameters.width,
                                     parameters.height);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForSDLRenderer(m_sdlWindow, m_sdlRender);
    ImGui_ImplSDLRenderer2_Init(m_sdlRender);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    // io.Fonts->AddFontDefault();
    // io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    // ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
    // IM_ASSERT(font != nullptr);

    return 0;
  }
}