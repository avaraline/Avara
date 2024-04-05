#pragma once

#include <string>

class AssetStorage {
public:
    /**
     * Get the root filesystem path for this storage destination.
     *
     * @return the filesystem path
     */
    virtual std::string GetRootPath() = 0;
};
