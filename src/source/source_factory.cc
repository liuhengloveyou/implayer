#include "source/source_factory.h"
#include "source/websocket_fmp4.h"
#include "simple_source.h"

namespace implayer
{
    std::shared_ptr<ISource> SourceFactory::Create(SourceType source_type, IMPlayerSharedPtr player)
    {
        std::shared_ptr<ISource> s;

        switch (source_type)
        {
        case SourceType::WEBSOCKET_FMP4:
            printf(" SourceFactory::Create: WEBSOCKET_FMP4\n");
            s = std::make_shared<WebsocketFmp4Source>(player);
            break;

        default:
            printf(" SourceFactory::Create: SimpleSource\n");
            s = std::make_shared<SimpleSource>(player);
            break;
        }

        return s;
    }

    std::shared_ptr<ISource> SourceFactory::CreateByPath(std::string path, IMPlayerSharedPtr player)
    {

        if (path.find_first_of("ws") == 0)
        {
            return Create(SourceType::WEBSOCKET_FMP4, player);
        }
    }

}