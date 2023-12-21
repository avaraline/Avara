#pragma once
#include "AssetStorage.h"
#include "AssetRepository.h"
#include "PackageManifest.h"
#include "PlayerConfig.h"
#include "Types.h"

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
#define MANIFESTFILE "set.json"
#define ALFDIR "alf"
#define BSPSDIR "bsps"
#define BSPSEXT ".json"
#define OGGDIR "ogg"
#define WAVDIR "wav"

enum struct BasePackage { Avara, Aftershock };

// If there is no package name, the data belongs to the base package.
typedef std::optional<std::string> MaybePackage;
#define NoPackage std::optional<std::string>{}

template <typename T>
using SimpleAssetCache = std::map<MaybePackage, std::shared_ptr<T>>;

template <typename T>
class Asset {
public:
    std::shared_ptr<T> data;

    MaybePackage packageName = NoPackage;
};

template <typename T>
class AssetCache {
public:
    /**
     * Remove any items from the cache that belong to any of the packages in `packageList`.
     *
     * @param packageList The list of package names we are looking to remove from the cache.
     */
    void RemoveAll(std::vector<MaybePackage> &packageList);
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
    
    static OSErr LoadLevel(std::string packageName, std::string levelTag);
private:
    AssetManager() {}

    static BasePackage basePackage;
    static std::vector<std::string> externalPackages;
    static std::shared_ptr<AssetStorage> baseStorage;
    static std::shared_ptr<AssetStorage> assetStorage;
    static std::vector<std::shared_ptr<AssetRepository>> repositoryStack;
    static SimpleAssetCache<PackageManifest> manifestCache;
    static SimpleAssetCache<std::string> avarascriptCache;
    static AssetCache<HullConfigRecord> hullCache;
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
     * Get the filesystem path for the specified relative path for the specified package.
     *
     * @param package The package we want the path for.
     * @param relativePath The relative path within that package.
     * @return the full path
     */
    static std::string GetFullPath(MaybePackage package, std::string relativePath);

    /**
     * Get the filesystem path for the specified package's manifest file.
     *
     * @param package The package we want the path for.
     * @return the path to the manifest file
     */
    static std::string GetManifestPath(MaybePackage package);

    /**
     * Get the filesystem path for the specified package's default script file.
     *
     * @param package The package we want the path for.
     * @return the path to the default script file
     */
    static std::string GetScriptPath(MaybePackage package);

    /**
     * Load the specified package's manifest file.
     *
     * @param package The package whose manifest we want to load.
     */
    static void LoadManifest(MaybePackage package);

    /**
     * Load the specified package's default script file.
     *
     * @param package The package whose default script we want to load.
     */
    static void LoadScript(MaybePackage package);

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

    /**
     * Recursively build a list of dependencies for the specified package. The dependencies are
     * appended to the provided list in the order in which they are encountered. As a side effect,
     * any packages encountered will have their manifest and avarascript files loaded and cached if
     * they are not already.
     *
     * @param currentPackage The package we are currently evaluating.
     * @param list The list we are in the process of modifying.
     */
    static void BuildDependencyList(std::string currentPackage, std::vector<std::string> &list);
};
