#pragma once

#include "AssetRepository.h"
#include "AssetStorage.h"

class LocalAssetRepository final: public AssetRepository, public AssetStorage {
public:
    static std::shared_ptr<LocalAssetRepository> GetInstance();

    /**
     * Get the list of available packages in this repository.
     *
     * @return the list of available packages
     */
    std::shared_ptr<std::vector<std::string>> GetPackageList();

    /**
     * Get the root filesystem path for this storage destination.
     *
     * @return the filesystem path
     */
    std::string GetRootPath();

    /**
     * Refresh the list of available packages.
     */
    void Refresh();
private:
    bool populatedList = false;
    std::shared_ptr<std::vector<std::string>> packageList = std::make_shared<std::vector<std::string>>();

    LocalAssetRepository() {};
    void BuildPackageList();
};
