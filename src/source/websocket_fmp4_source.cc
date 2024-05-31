#include <thread>
#include <chrono>

#include "source/websocket_fmp4_source.h"

namespace implayer
{
    WebsocketFmp4Source::WebsocketFmp4Source(IMPlayerSharedPtr player)
        : BaseSource::BaseSource(player)
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
        uint32_t to_write = numBytes;

        while (to_write > 0)
        {
            size_t available = ring_buffer_.writeAvailable();
            if (available <= 0)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }

            auto writed = ring_buffer_.writeBuff(data, to_write < available ? to_write : available);
            to_write -= writed;
        }

        return numBytes;
    }

    int WebsocketFmp4Source::ReadPacket(void *opaque, uint8_t *buf, int buf_size)
    {
        while (true)
        {
            size_t available = ring_buffer_.readAvailable();
            if (available > 0)
            {
                break;
            }
            else
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(7));
            }
        }

        /* copy internal buffer data to buf */
        return ring_buffer_.readBuff(buf, buf_size);
    }

    int WebsocketFmp4Source::Open(const std::string &path)
    {
        int ret = ws_->open(path);
        if (ret != 0)
        {
            return ret;
        }

        ret = demux_->Open(path);
        if (ret != 0)
        {
            return ret;
        }
        printf("WebsocketFmp4Source::Open OK\n");

        BaseSource::Open(path);

        return 0;
    }

    AVStream *WebsocketFmp4Source::stream(int stream_index) const
    {
        return demux_->stream(stream_index);
    }

    MediaFileInfo WebsocketFmp4Source::media_info() const
    {
        MediaFileInfo info;
        info.path = demux_->format_context()->url;
        info.width = video_codec_->codec_context()->width;
        info.height = video_codec_->codec_context()->height;

        info.duration = demux_->format_context()->duration;
        info.bit_rate = demux_->format_context()->bit_rate;

        auto *video_stream = demux_->stream(demux_->video_stream_index());
        if (video_stream != nullptr)
        {
            info.fps = av_q2d(video_stream->avg_frame_rate);
            info.pixel_format = video_stream->codecpar->format;
            info.video_stream_timebase = video_stream->time_base;
            printf("!!!!!!!!!!!!!!!!!!!!! %d %d %d\n", info.pixel_format, video_codec_->codec_context()->width, video_codec_->codec_context()->height);
        }

        auto *audio_stream = demux_->stream(demux_->audio_stream_index());
        if (audio_stream != nullptr)
        {
            info.sample_rate = audio_stream->codecpar->sample_rate;
            info.channels = audio_stream->codecpar->channels;
            info.sample_format = audio_stream->codecpar->format;
            info.channel_layout = audio_stream->codecpar->channel_layout;
            info.audio_stream_timebase = audio_stream->time_base;
        }

        return info;
    }

    int WebsocketFmp4Source::video_stream_index()
    {
        return demux_->video_stream_index();
    }
    int WebsocketFmp4Source::audio_stream_index()
    {
        return demux_->audio_stream_index();
    }

    int64_t WebsocketFmp4Source::duration()
    {
        return 0;
    }
}
