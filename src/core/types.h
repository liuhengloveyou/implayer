#ifndef IMPLAYER_TYPES_H
#define IMPLAYER_TYPES_H

namespace implayer
{
    enum class SourceType
    {
        UNKNOWN,
        FILE_SOURCE,
        WEBSOCKET_FMP4_SOURCE,
    };

    typedef struct Event
    {
        SourceType source_type = SourceType::UNKNOWN;
        char path[1024] = {0};
    } Event;

    enum class PlayState : int
    {
        kIdle = 1,
        kStopped,
        kPlaying,
        kSeeking,
        kPaused,
    };
}

#endif