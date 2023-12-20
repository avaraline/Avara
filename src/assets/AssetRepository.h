#pragma once

#include <memory>
#include <string>
#include <vector>

class AssetRepository {
public:
    /**
     * Get the list of available packages in this repository.
     *
     * @return the list of available packages
     */
    virtual std::shared_ptr<std::vector<std::string>> GetPackageList() = 0;
};
