#ifndef IMPLAYER_WEBSOCKET_FMP4_SOURCE_H
#define IMPLAYER_WEBSOCKET_FMP4_SOURCE_H

#include <string>
#include <vector>
#include <chrono>
#include <memory>

#include "source/base_source.h"
#include "ffmpeg/i_demuxer.h"
#include "ffmpeg/ffmpeg_avio_demuxer.h"
#include "ffmpeg/ffmpeg_codec.h"
#include "network/emscripten_websocket.h"
#include "core/i_player.h"
#include "Ring-Buffer/ringbuffer.hpp"

namespace implayer
{
    class WebsocketFmp4Source : public DmuxerAdapter, public WebsocketAdapter, public BaseSource
    {

    public:
        WebsocketFmp4Source() = delete;
        WebsocketFmp4Source(IMPlayerSharedPtr player);
        ~WebsocketFmp4Source();

    public:
        // DmuxerAdapter
        int ReadPacket(void *opaque, uint8_t *buf, int buf_size) override;
        // WebsocketAdapter
        int OnWebSocketMessage(uint8_t *data, uint32_t numBytes) override;

        // ISource
    public:
        int Open(const std::string &path) override;
        AVStream *stream(int stream_index) const override;
        MediaFileInfo media_info() const override;
        int video_stream_index() override;
        int audio_stream_index() override;
        int64_t duration() override;

    private:
        jnk0le::Ringbuffer<uint8_t, 1024 * 1024> ring_buffer_;
        std::shared_ptr<EmscriptenWebsocket> ws_{nullptr};
        std::shared_ptr<FFmpegAvioDmuxer> demux_{nullptr};
    };
}

#endif