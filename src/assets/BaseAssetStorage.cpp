#include "BaseAssetStorage.h"
#include "AssetManager.h"
#include "BasePath.h"

#include <sstream>

BaseAssetStorage::BaseAssetStorage()
{
    basePath = GetBasePath();
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
