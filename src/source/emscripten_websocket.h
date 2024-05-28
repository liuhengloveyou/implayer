// emcc -std=c++11 -O2 -Wall -Werror -sASSERTIONS -sALLOW_MEMORY_GROWTH -sABORTING_MALLOC=0 -lembind -lworkerfs.js -lwebsocket.js --pre-js wasmws-pre.js --js-library wasmws-library.js -o dist/wasmws.js wasmws.cpp

#include <unistd.h>
#include <string>
#include <vector>
#include <chrono>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/val.h>
#include <emscripten/websocket.h>
#endif

using namespace std;
using namespace std::chrono;

class EmscriptenWebsocket
{
public:
    EmscriptenWebsocket() = default;
    ~EmscriptenWebsocket()
    {
        emscripten_websocket_close(socket_, 0, nullptr);
        emscripten_websocket_delete(EMSCRIPTEN_WEBSOCKET_T socket_);
    };

public:
    int run(const char *url)
    {
        if (!emscripten_websocket_is_supported())
        {
            printf("WebSockets are not supported, cannot continue!\n");
            exit(1);
        }

        EmscriptenWebSocketCreateAttributes attr;
        emscripten_websocket_init_create_attributes(&attr);
        attr.url = url;

        EMSCRIPTEN_WEBSOCKET_T socket = emscripten_websocket_new(&attr);
        if (socket <= 0)
        {
            printf("WebSocket creation failed, error code %d!\n", (EMSCRIPTEN_RESULT)socket);
            exit(1);
        }

        emscripten_websocket_set_onopen_callback(socket, (void *)42, WebSocketOpen);
        emscripten_websocket_set_onclose_callback(socket, (void *)43, WebSocketClose);
        emscripten_websocket_set_onerror_callback(socket, (void *)44, WebSocketError);
        emscripten_websocket_set_onmessage_callback(socket, (void *)45, WebSocketMessage);

        // emscripten_exit_with_live_runtime();
    }

    EM_BOOL WebSocketOpen(int eventType, const EmscriptenWebSocketOpenEvent *e, void *userData)
    {
        printf("open(eventType=%d, userData=%ld)\n", eventType, (long)userData);

        // emscripten_websocket_send_utf8_text(e->socket, "hello on the other side");

        // char data[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        // emscripten_websocket_send_binary(e->socket, data, sizeof(data));

        // emscripten_websocket_close(e->socket, 0, 0);
        return 0;
    }

    EM_BOOL WebSocketClose(int eventType, const EmscriptenWebSocketCloseEvent *e, void *userData)
    {
        printf("close(eventType=%d, wasClean=%d, code=%d, reason=%s, userData=%ld)\n", eventType, e->wasClean, e->code, e->reason, (long)userData);
        return 0;
    }

    EM_BOOL WebSocketError(int eventType, const EmscriptenWebSocketErrorEvent *e, void *userData)
    {
        printf("error(eventType=%d, userData=%ld)\n", eventType, (long)userData);
        return 0;
    }

    EM_BOOL WebSocketMessage(int eventType, const EmscriptenWebSocketMessageEvent *e, void *userData)
    {
        // printf("message(eventType=%d, userData=%ld, data=%p, numBytes=%d, isText=%d)\n", eventType, (long)userData, e->data, e->numBytes, e->isText);

        if (e->isText)
            printf("text data: \"%s\"\n", e->data);
        else
        {
            // printf("binary data:");
            // for (int i = 0; i < e->numBytes; ++i)
            //     printf(" %02X", e->data[i]);
            // printf("\n");

            auto now = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();

            FloatWaveDataToWasm *tmpData = (FloatWaveDataToWasm *)e->data;
            printf("cpp/WebSocketMessage::%d %d %lld \n", tmpData->rows, tmpData->columns, now - tmpData->t[0]);
            // for (int i = 0; i < 1088; i++) {
            //     for (int j = 0; j < 2000; j++) {
            //         printf("[%d %d %f]\t", i, j, tmpData->data[i * 1088 + j]);
            //     }
            //     printf("\n");
            // }

            postDataToJS(tmpData, sizeof(FloatWaveDataToWasm));

            // val v = getBytes();

            // EM_ASM({
            //     callFromC();
            // });

            // emscripten_websocket_delete(e->socket);
            // exit(0);
        }
        return 0;
    }

private:
    EMSCRIPTEN_WEBSOCKET_T socket_;
}