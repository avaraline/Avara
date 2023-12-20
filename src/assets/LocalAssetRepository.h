#pragma once

#include "AssetRepository.h"

class LocalAssetRepository final: public AssetRepository {
public:
    static std::shared_ptr<LocalAssetRepository> GetInstance();

    /**
     * Get the list of available packages in this repository.
     *
     * @return the list of available packages
     */
    std::shared_ptr<std::vector<std::string>> GetPackageList();

    /**
     * Get the filesystem path for the specified package, if it's available.
     *
     * @param packageName The package we want the path for.
     * @return the path to the package, if available
     */
    std::optional<std::string> GetPackagePath(std::string packageName);

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
