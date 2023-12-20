#pragma once
#include "AssetStorage.h"
#include "AssetRepository.h"
#include "PackageManifest.h"

#include <json.hpp>

#include <map>
#include <memory>
#include <sstream>
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

// Path separator
#if defined(_WIN32)
#define PATHSEP "\\"
#else
#define PATHSEP "/"
#endif

// Package structure
#define LEVELDIR "levels"
#define MANIFESTFILE "set.json"
#define ALFDIR "alf"
#define BSPSDIR "bsps"
#define BSPSEXT ".json"
#define DEFAULTSCRIPT "default.avarascript"
#define OGGDIR "ogg"
#define WAVDIR "wav"

enum BasePackage { Avara, Aftershock };
typedef std::optional<std::string> MaybePackage;
#define NoPackage std::optional<std::string>{}

template<typename T>
class Asset {
public:
    T data;

    // If there is no package name, the data belongs to the base package.
    MaybePackage packageName = NoPackage;
};

template<typename T>
class AssetCache {
public:
    /**
     * Remove any items from the cache that belong to any of the packages in `packageList`.
     *
     * @param packageList The list of package names we are looking to remove from the cache.
     */
    void RemoveAll(std::vector<MaybePackage> packageList);
private:
    std::map<int16_t, Asset<T>> collection = {};
};

class AssetManager {
public:
    /**
     * Get an exhaustive list of all available packages (excluding base packages).
     *
     * @return the list of available packages
     */
    static std::vector<std::string> GetAvailablePackages();

    /**
     * Get the filesystem path for the specified package, if it's available.
     *
     * @param packageName The package we want the path for.
     * @return the path to the package, if available
     */
    static std::optional<std::string> GetPackagePath(std::string packageName);
    
    static void LoadLevel(std::string packageName, std::string levelTag);
private:
    AssetManager() {}

    static BasePackage basePackage;
    static std::vector<std::string> externalPackages;
    static std::shared_ptr<AssetStorage> baseStorage;
    static std::shared_ptr<AssetStorage> assetStorage;
    static std::vector<std::shared_ptr<AssetRepository>> repositoryStack;
    static std::map<std::string, PackageManifest> manifestCache;
    static std::map<std::string, std::string> avarascriptCache;
    static AssetCache<nlohmann::json> bspCache;
    static AssetCache<std::vector<uint8_t>> sndCache;

    /**
     * Get the filesystem path for the specified base package.
     *
     * @param basePackage The base package we want the path for.
     * @throws std::invalid_argument Thrown when there is no path defined for the base package.
     * @return the path to the base package
     */
    static std::string GetBasePackagePath(BasePackage basePackage) throw();

    /**
     * Change which base package is being used (e.g. Avara vs. Aftershock). If it differs from the
     * current base package, the cached assets from the current base are removed.
     *
     * @param newBase The base package we want to use.
     */
    static void SwitchBasePackage(BasePackage newBase);

    /**
     * Change which package is being used. If it differs from the current package, the cached
     * assets from the current package and any of its unneeded dependencies are removed.
     *
     * @param packageName The package we want to use.
     */
    static void SwitchContext(std::string packageName);
};
