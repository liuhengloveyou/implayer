#include "network/emscripten_websocket.h"
#include <unistd.h>

namespace implayer
{
    EM_BOOL onWebSocketOpen(int eventType, const EmscriptenWebSocketOpenEvent *e, void *userData)
    {
        printf("WebSocketOpen(eventType=%d, userData=%ld)\n", eventType, (long)userData);

        return EM_TRUE;
    }

    EM_BOOL onWebSocketClose(int eventType, const EmscriptenWebSocketCloseEvent *e, void *userData)
    {
        printf("WebSocketClose(eventType=%d, wasClean=%d, code=%d, reason=%s, userData=%ld)\n", eventType, e->wasClean, e->code, e->reason, (long)userData);

        return EM_TRUE;
    }

    EM_BOOL onWebSocketError(int eventType, const EmscriptenWebSocketErrorEvent *e, void *userData)
    {
        printf("WebSocketClose(eventType=%d, userData=%ld)\n", eventType, (long)userData);

        return EM_TRUE;
    }

    EM_BOOL onWebSocketMessage(int eventType, const EmscriptenWebSocketMessageEvent *e, void *userData)
    {
        // printf("WebSocketMessage(eventType=%d, numBytes=%d, isText=%d)\n", eventType, e->numBytes, e->isText);

        EmscriptenWebsocket *ws = (EmscriptenWebsocket *)userData;
        if (ws)
        {
            ws->OnMessage(e->data, e->numBytes);
        }

        return EM_TRUE;
    }

    EmscriptenWebsocket::~EmscriptenWebsocket()
    {
        emscripten_websocket_close(socket_, 0, NULL);
        emscripten_websocket_delete(socket_);
    }

    int EmscriptenWebsocket::OnMessage(uint8_t *data, uint32_t numBytes)
    {
        if (adapter_)
        {
            adapter_->OnWebSocketMessage(data, numBytes);
        }

        return EM_TRUE;
    };

    int EmscriptenWebsocket::SendText(std::string txt)
    {
        emscripten_websocket_send_utf8_text(socket_, txt.c_str());
      
        return 0;
    }
    
    int EmscriptenWebsocket::open(const std::string url)
    {
        if (!emscripten_websocket_is_supported())
        {
            printf("WebSockets are not supported, cannot continue!\n");
            return -1;
        }
        url_ = url;
        printf("EmscriptenWebsocket::open: %s\n", url.c_str());

        EmscriptenWebSocketCreateAttributes attr;
        emscripten_websocket_init_create_attributes(&attr);
        attr.url = url_.c_str();
        // attr.createOnMainThread = EM_TRUE;

        EMSCRIPTEN_WEBSOCKET_T socket = emscripten_websocket_new(&attr);
        if (socket <= 0)
        {
            printf("WebSocket creation failed, error code %d!\n", (EMSCRIPTEN_RESULT)socket);
            return -1;
        }

        emscripten_websocket_set_onopen_callback(socket, this, onWebSocketOpen);
        emscripten_websocket_set_onclose_callback(socket, this, onWebSocketClose);
        emscripten_websocket_set_onerror_callback(socket, this, onWebSocketError);
        emscripten_websocket_set_onmessage_callback(socket, this, onWebSocketMessage);
        // emscripten_exit_with_live_runtime();
        socket_ = socket;

        return 0;
    }
}