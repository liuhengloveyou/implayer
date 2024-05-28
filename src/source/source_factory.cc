#include "source/source_factory.h"

namespace implayer
{
    std::shared_ptr<ISource> SourceFactory::Create(SourceType source_type)
    {
        std::shared_ptr<ISource> s{nullptr};

        switch (source_type)
        {
        case SourceType::WEBSOCKET_FMP4:
            s = std::make_shared<>();
            break;

        default:
            return nullptr;
        }

        return s;
    }

}