#include "BaseAssetStorage.h"
#include "AssetManager.h"

#include <SDL2/SDL.h>
#include <sstream>

BaseAssetStorage::BaseAssetStorage()
{
    char *sdlPathTmp = SDL_GetBasePath();
    basePath = std::string(sdlPathTmp);
    SDL_free(sdlPathTmp);
}

std::shared_ptr<BaseAssetStorage> BaseAssetStorage::GetInstance() {
    static auto instance = std::make_shared<BaseAssetStorage>(BaseAssetStorage());
    return instance;
};

std::string BaseAssetStorage::GetRootPath()
{
    std::stringstream path;
    path << basePath << "rsrc";
    return path.str();
}
