#include <thread>
#include <chrono>

#include "source/websocket_fmp4.h"

namespace implayer
{
    WebsocketFmp4Source::WebsocketFmp4Source(IMPlayerSharedPtr player)
    {
        ws_ = std::make_shared<EmscriptenWebsocket>();
        ws_->attachAdapter(this);
        demux_ = std::make_shared<FFmpegAvioDmuxer>();
        demux_->attachAdapter(this);
    }

    WebsocketFmp4Source::~WebsocketFmp4Source()
    {
        ws_ = nullptr;
        demux_ = nullptr;
    }

    int WebsocketFmp4Source::OnWebSocketMessage(uint8_t *data, uint32_t numBytes)
    {
        while (numBytes > 0)
        {
            size_t available = ring_buffer_.writeAvailable();
            printf("WebsocketFmp4Source::OnWebSocketMessage::%d %d\n", numBytes, available);
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            if (available <= 0)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }
            else
            {
                ring_buffer_.writeBuff(data, numBytes < available ? numBytes : available);
                numBytes -= available;
            }
        }
    }

    int WebsocketFmp4Source::ReadPacket(void *opaque, uint8_t *buf, int buf_size)
    {
        printf("WebsocketFmp4Source::ReadPacket::%d\n", buf_size);
    }

    int WebsocketFmp4Source::open(const std::string &path)
    {
        printf("WebsocketFmp4Source::open %s\n", path.c_str());

        ws_->init(path);
        return 0;
    }

    int WebsocketFmp4Source::seek(int64_t timestamp)
    {
    }
    MediaFileInfo WebsocketFmp4Source::getMediaFileInfo()
    {
    }
    int64_t WebsocketFmp4Source::getDuration()
    {
    }
    int64_t WebsocketFmp4Source::getCurrentPosition()
    {
    }
    int WebsocketFmp4Source::getQueueSize()
    {
    }
    std::shared_ptr<Frame> WebsocketFmp4Source::dequeueVideoFrame()
    {
    }
    std::shared_ptr<Frame> WebsocketFmp4Source::dequeueAudioFrame()
    {
    }
}