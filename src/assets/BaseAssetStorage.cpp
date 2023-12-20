#include "BaseAssetStorage.h"
#include "AssetManager.h"

#include <SDL2/SDL.h>
#include <sstream>

std::shared_ptr<BaseAssetStorage> BaseAssetStorage::GetInstance() {
    static std::shared_ptr<BaseAssetStorage> instance
        = std::make_shared<BaseAssetStorage>(BaseAssetStorage());
    return instance;
};

std::string BaseAssetStorage::GetRootPath()
{
    std::stringstream path;
    path << SDL_GetBasePath() << "rsrc";
    return path.str();
}
