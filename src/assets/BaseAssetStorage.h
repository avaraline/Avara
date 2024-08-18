#pragma once

#include "AssetStorage.h"

#include <memory>
#include <string>

class BaseAssetStorage final: public AssetStorage {
public:
    static std::shared_ptr<BaseAssetStorage> GetInstance();
    
    /**
     * Get the root filesystem path for this storage destination.
     *
     * @return the filesystem path
     */
    std::string GetRootPath();
private:
    std::string basePath;
    
    BaseAssetStorage();
};
