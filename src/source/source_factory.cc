#include "source/source_factory.h"
#include "source/websocket_fmp4_source.h"
#include "source/simple_source.h"

namespace implayer
{
    std::shared_ptr<ISource> SourceFactory::Create(SourceType source_type, IMPlayerSharedPtr player)
    {
        std::shared_ptr<ISource> s;

        switch (source_type)
        {
        case SourceType::WEBSOCKET_FMP4_SOURCE:
            printf(" SourceFactory::Create: WEBSOCKET_FMP4\n");
            s = std::make_shared<WebsocketFmp4Source>(player);
            break;
        case SourceType::FILE_SOURCE:
            printf(" SourceFactory::Create: FILE_SOURCE\n");
            s = std::make_shared<SimpleSource>(player);
            break;
        default:
            printf(" SourceFactory::Create: SimpleSource\n");
            s = std::make_shared<SimpleSource>(player);
            break;
        }

        return s;
    }

}