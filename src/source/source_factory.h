#ifndef IMPLAYER_SOURCE_FACTORY_H
#define IMPLAYER_SOURCE_FACTORY_H

#include <memory>

#include "source/i_source.h"
#include "utils/singleton.h"
#include "core/i_player.h"

namespace implayer
{
    enum class SourceType
    {
        UNKNOWN,
        WEBSOCKET_FMP4,
    };

    class SourceFactory : public Singleton<SourceFactory>
    {
    public:
        SourceFactory() = default;
        ~SourceFactory() = default;
        SourceFactory(SourceFactory &&other) = delete;
        SourceFactory &operator=(SourceFactory &&other) = delete;
        SourceFactory(const SourceFactory &other) = delete;
        SourceFactory &operator=(const SourceFactory &other) = delete;

    public:
        std::shared_ptr<ISource> Create(SourceType source_type, IMPlayerSharedPtr player);
        std::shared_ptr<ISource> CreateByPath(std::string path, IMPlayerSharedPtr player);
    };
}

#endif