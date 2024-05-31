#ifndef IMPLAYER_NETWORK_WEBSOCKET_H
#define IMPLAYER_NETWORK_WEBSOCKET_H

#include <memory>
#include <string>
#include <vector>
#include <chrono>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/val.h>
#include <emscripten/websocket.h>
#endif

#include "utils/thread_base.h"

namespace implayer
{
    class WebsocketAdapter
    {
    public:
        virtual int OnWebSocketMessage(uint8_t *data, uint32_t numBytes) = 0;
    };

    class EmscriptenWebsocket
    {
    public:
        EmscriptenWebsocket(){};
        ~EmscriptenWebsocket();

    public:
        int open(const std::string url);
        int SendText(std::string txt);
        int OnMessage(uint8_t *data, uint32_t numBytes);
        void attachAdapter(WebsocketAdapter* adapter)
        {
            adapter_ = adapter;
        }

    private:
        WebsocketAdapter* adapter_{nullptr};
        std::string url_;
        EMSCRIPTEN_WEBSOCKET_T socket_;
    };
}
#endif