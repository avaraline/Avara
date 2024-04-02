#pragma once
#include "AssetStorage.h"
#include "AssetRepository.h"
#include "PackageManifest.h"
#include "OggFile.h"
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

enum struct BasePackage { Avara, Aftershock };

// If there is no package name, the data belongs to the base package.
typedef std::optional<std::string> MaybePackage;
#define NoPackage std::optional<std::string>{}

template <typename T>
class SimpleAssetCache : public std::map<MaybePackage, std::shared_ptr<T>> {
public:
    /**
     * Remove any items from the cache that belong to any of the packages in `packageList`.
     *
     * @param packageList The list of package names we are looking to remove from the cache.
     */
    void RemoveAll(std::vector<MaybePackage> &packageList);
};

template <typename T>
class Asset {
public:
    std::shared_ptr<T> data;

    MaybePackage packageName = NoPackage;
};

template <typename T>
class AssetCache : public std::map<int16_t, Asset<T>> {
public:
    /**
     * Remove any items from the cache that belong to any of the packages in `packageList`.
     *
     * @param packageList The list of package names we are looking to remove from the cache.
     */
    void RemoveAll(std::vector<MaybePackage> &packageList);
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
     * Get a JSON object containing descriptions of every Avarascript object that can be
     * instantiated, listed in their properly enumerated order. The order is critical for
     * `LinkLoose`, which maps Avarascript object names with their actual C++ types.
     *
     * @return the enumerated object descriptions
     */
    static std::shared_ptr<nlohmann::json> GetEnumeratedObjectTypes();

    /**
     * Get the full filesystem path for an ALF file, if it is available.
     *
     * @param relativePath The relative path to look for.
     * @return the full path to the ALF file
     */
    static std::optional<std::string> GetResolvedAlfPath(std::string relativePath);

    /**
     * Get the full filesystem path for a shader file, if it is available.
     *
     * @param relativePath The relative path to look for.
     * @return the full path to the shader file
     */
    static std::optional<std::string> GetShaderPath(std::string relativePath);

    /**
     * Get the manifest for the specified package.
     *
     * @param package The package we want the manifest for.
     */
    static std::optional<std::shared_ptr<PackageManifest>> GetManifest(MaybePackage package);

    /**
     * Get the default scripts for all loaded packages in the order in which they should run.
     *
     * @return the default scripts
     */
    static std::vector<std::shared_ptr<std::string>> GetAllScripts();

    /**
     * Get the BSP with the provided resource ID, if available.
     *
     * @param id The resource ID.
     * @return the BSP json
     */
    static std::optional<std::shared_ptr<nlohmann::json>> GetBsp(int16_t id);

    /**
     * Get the OGG with the provided resource ID, if available.
     *
     * @param id The resource ID.
     * @return the OGG data
     */
    static std::optional<std::shared_ptr<OggFile>> GetOgg(int16_t id);

    /**
     * Get the hull with the provided resource ID, if available.
     *
     * @param id The resource ID.
     * @return the hull configuration
     */
    static std::optional<std::shared_ptr<HullConfigRecord>> GetHull(int16_t id);

    /**
     * Run important operations at application start.
     */
    static void Init();

    /**
     * Attempt to load the level with the provided relative path from the provided package name.
     *
     * @param packageName The package we want to load from.
     * @param relativePath The relative path to an ALF file.
     * @param[out] levelName The name of the level as defined by the package's manifest.
     * @return a status code indicating either no error or that the file was not found
     */
    static OSErr LoadLevel(std::string packageName, std::string relativePath, std::string &levelName);

    /**
     * Checks to see if a package with the given name is stored locally.
     *
     * @param packageName The package we want to search for.
     * @return whether the package is stored locally
     */
    static bool PackageInStorage(std::string packageName);
private:
    AssetManager() {}

    static BasePackage basePackage;
    static std::vector<std::string> externalPackages;
    static std::shared_ptr<AssetStorage> baseStorage;
    static std::shared_ptr<AssetStorage> assetStorage;
    static std::vector<std::shared_ptr<AssetRepository>> repositoryStack;
    static std::shared_ptr<nlohmann::json> objectTypes;
    static SimpleAssetCache<PackageManifest> manifestCache;
    static SimpleAssetCache<std::string> avarascriptCache;
    static AssetCache<nlohmann::json> bspCache;
    static AssetCache<OggFile> sndCache;
    static AssetCache<HullConfigRecord> hullCache;

    /**
     * Get the filesystem path for the specified base package.
     *
     * @param basePackage The base package we want the path for.
     * @throws std::invalid_argument Thrown when there is no path defined for the base package.
     * @return the path to the base package
     */
    static std::string GetBasePackagePath(BasePackage basePackage);

    /**
     * Get the filesystem path for the specified package, if it's available.
     *
     * @param packageName The package we want the path for.
     * @return the path to the package, if available
     */
    static std::optional<std::string> GetPackagePath(std::string packageName);

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
     * Get the filesystem path for an ALF file within the specified package.
     *
     * @param package The package we want the path for.
     * @return the path to the ALF file
     */
    static std::string GetAlfPath(MaybePackage package, std::string relativePath);

    /**
     * Get the filesystem path for a BSP file with the specified id and package.
     *
     * @param package The package we want the path for.
     * @param id The BSP's resource ID.
     * @return the path to the BSP file
     */
    static std::string GetBspPath(MaybePackage package, int16_t id);

    /**
     * Get the filesystem path for an OGG file with the specified id and package.
     *
     * @param package The package we want the path for.
     * @param id The OGG's resource ID.
     * @return the path to the OGG file
     */
    static std::string GetOggPath(MaybePackage package, int16_t id);

    /**
     * Load the JSON file containing descriptions of every Avarascript object that can be
     * instantiated, listed in their properly enumerated order. The order is critical for
     * `LinkLoose`, which maps Avarascript object names with their actual C++ types.
     *
     * @throws std::runtime_error Thrown when the JSON file cannot be read.
     */
    static void LoadEnumeratedObjectTypes();

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
     * Load the specified BSP json file.
     *
     * @param id The resource ID of the BSP.
     */
    static void LoadBsp(int16_t id);

    /**
     * Load the specified OGG file.
     *
     * @param id The resource ID of the OGG.
     */
    static void LoadOgg(int16_t id);

    /**
     * Load the specified hull configuration. (Despite its name, nothing is actually loaded
     * here--merely put into the hull cache to speed future lookups.)
     *
     * @param id The resource ID of the hull configuration.
     */
    static void LoadHull(int16_t id);

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

    /**
     * Compare items currently in the provided cache with the list of external packages, and remove
     * items from the cache that should be overridden by higher priority packages.
     *
     * @param cache The cache to review.
     * @tparam T The type of the assets in the cache.
     */
    template <typename T>
    static void ReviewPriorities(AssetCache<T> &cache);

    /** @copydoc AssetManager::ReviewPriorities */
    static void ReviewPriorities(AssetCache<nlohmann::json> &cache);

    /** @copydoc AssetManager::ReviewPriorities */
    static void ReviewPriorities(AssetCache<OggFile> &cache);

    /** @copydoc AssetManager::ReviewPriorities */
    static void ReviewPriorities(AssetCache<HullConfigRecord> &cache);
};
