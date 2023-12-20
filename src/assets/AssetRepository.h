#pragma once

#include <memory>
#include <string>
#include <vector>

#ifdef __has_include
#  if __has_include(<optional>)                // Check for a standard library
#    include <optional>
#  elif __has_include(<experimental/optional>) // Check for an experimental version
#    include <experimental/optional>           // Check if __has_include is present
#  else                                        // Not found at all
#     error "Missing <optional>"
#  endif
#endif

class AssetRepository {
public:
    /**
     * Get the list of available packages in this repository.
     *
     * @return the list of available packages
     */
    virtual std::shared_ptr<std::vector<std::string>> GetPackageList() = 0;

    /**
     * Get the filesystem path for the specified package, if it's available.
     *
     * @param packageName The package we want the path for.
     * @return the path to the package, if available
     */
    virtual std::optional<std::string> GetPackagePath(std::string packageName) = 0;
};
