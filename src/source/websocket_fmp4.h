#ifndef IMPLAYER_WEBSOCKET_FMP4_SOURCE_H
#define IMPLAYER_WEBSOCKET_FMP4_SOURCE_H

#include <string>
#include <vector>
#include <chrono>
#include <memory>

#include "source/i_source.h"
#include "ffmpeg/i_demuxer.h"
#include "ffmpeg/ffmpeg_avio_demuxer.h"
#include "ffmpeg/ffmpeg_codec.h"
#include "network/emscripten_websocket.h"
#include "core/i_player.h"
#include "Ring-Buffer/ringbuffer.hpp"

namespace implayer
{
    class WebsocketFmp4Source : public DmuxerAdapter, public WebsocketAdapter, public ISource
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
        int open(const std::string &file_path) override;
        int seek(int64_t timestamp) override;
        MediaFileInfo getMediaFileInfo() override;
        int64_t getDuration() override;
        int64_t getCurrentPosition() override;
        int getQueueSize() override;
        std::shared_ptr<Frame> dequeueVideoFrame() override;
        std::shared_ptr<Frame> dequeueAudioFrame() override;

    private:
        jnk0le::Ringbuffer<uint8_t, 4096> ring_buffer_;
        std::shared_ptr<EmscriptenWebsocket> ws_{nullptr};
        std::shared_ptr<FFmpegAvioDmuxer> demux_{nullptr};
        std::shared_ptr<FFmpegCodec> video_codec_{nullptr};
        std::shared_ptr<FFmpegCodec> audio_codec_{nullptr};
    };
}

#endif