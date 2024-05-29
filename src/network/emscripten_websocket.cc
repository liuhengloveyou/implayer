#include "network/emscripten_websocket.h"
#include <unistd.h>

namespace implayer
{
    EM_BOOL onWebSocketOpen(int eventType, const EmscriptenWebSocketOpenEvent *e, void *userData)
    {
        printf("WebSocketOpen(eventType=%d, userData=%ld)\n", eventType, (long)userData);

        int data = 1;
        emscripten_websocket_send_binary(e->socket, (void *)&data, sizeof(data));
        // emscripten_websocket_send_utf8_text(e->socket, "play");
        // emscripten_websocket_close(e->socket, 0, 0);
        return 0;
    }

    EM_BOOL onWebSocketClose(int eventType, const EmscriptenWebSocketCloseEvent *e, void *userData)
    {
        printf("WebSocketClose(eventType=%d, wasClean=%d, code=%d, reason=%s, userData=%ld)\n", eventType, e->wasClean, e->code, e->reason, (long)userData);
        return 0;
    }

    EM_BOOL onWebSocketError(int eventType, const EmscriptenWebSocketErrorEvent *e, void *userData)
    {
        printf("WebSocketClose(eventType=%d, userData=%ld)\n", eventType, (long)userData);
        return 0;
    }

    EM_BOOL onWebSocketMessage(int eventType, const EmscriptenWebSocketMessageEvent *e, void *userData)
    {
        printf("WebSocketMessage(eventType=%d, numBytes=%d, isText=%d)\n", eventType, e->numBytes, e->isText);

        EmscriptenWebsocket *ws = (EmscriptenWebsocket *)userData;
        if (ws)
        {
            ws->OnWebSocketMessage(e->data, e->numBytes);
        }

        return 0;
    }

    EmscriptenWebsocket::~EmscriptenWebsocket()
    {
        stopThread();

        emscripten_websocket_close(socket_, 0, NULL);
        emscripten_websocket_delete(socket_);
    }

    int EmscriptenWebsocket::OnWebSocketMessage(uint8_t *data, uint32_t numBytes)
    {
        printf("EmscriptenWebsocket::OnWebSocketMessage: %d\n", numBytes);

        if (adapter_)
        {
            return adapter_->OnWebSocketMessage(data, numBytes);
        }

        return 0;
    };

    void EmscriptenWebsocket::threadMain()
    {
        EmscriptenWebSocketCreateAttributes attr;
        emscripten_websocket_init_create_attributes(&attr);
        attr.url = url_.c_str();
        attr.createOnMainThread = EM_FALSE;

        EMSCRIPTEN_WEBSOCKET_T socket = emscripten_websocket_new(&attr);
        if (socket <= 0)
        {
            printf("WebSocket creation failed, error code %d!\n", (EMSCRIPTEN_RESULT)socket);
            return;
        }

        emscripten_websocket_set_onopen_callback(socket, this, onWebSocketOpen);
        emscripten_websocket_set_onclose_callback(socket, this, onWebSocketClose);
        emscripten_websocket_set_onerror_callback(socket, this, onWebSocketError);
        emscripten_websocket_set_onmessage_callback(socket, this, onWebSocketMessage);
        // emscripten_exit_with_live_runtime();
        socket_ = socket;

        while (!m_thread_stop)
        {
            usleep(10);
        }

        return;
    }

    int EmscriptenWebsocket::init(const std::string url)
    {
        if (!emscripten_websocket_is_supported())
        {
            printf("WebSockets are not supported, cannot continue!\n");
            return -1;
        }
        url_ = url;
        printf("EmscriptenWebsocket::init: %s\n", url.c_str());

        startThread();

        return 0;
    }
}