#include "AssetManager.h"
#include "BaseAssetStorage.h"
#include "LocalAssetRepository.h"

#include "LevelLoader.h"
#include "Resource.h" // TODO: REMOVE THIS ONCE SOUND LOADING WORKS

#include <algorithm>
#include <fstream>
#include <regex>
#include <set>

template <typename T>
void SimpleAssetCache<T>::RemoveAll(std::vector<MaybePackage> &packageList)
{
    for (auto const &pkg : packageList) {
        this->erase(pkg);
    }
};

template <typename T>
void AssetCache<T>::RemoveAll(std::vector<MaybePackage> &packageList)
{
    std::vector<int16_t> cacheHits = {};
    for (auto const &[id, asset] : *this) {
        for (auto &pkg : packageList) {
            if (pkg == asset.packageName) {
                cacheHits.push_back(id);
            }
        }
    }
    for (auto &id : cacheHits) {
        this->erase(id);
    }
};

std::shared_ptr<AssetStorage> _baseStorage = BaseAssetStorage::GetInstance();
std::shared_ptr<AssetStorage> _localStorage = LocalAssetRepository::GetInstance();
std::shared_ptr<AssetRepository> _localRepo = LocalAssetRepository::GetInstance();

// Initialize static variables.
BasePackage AssetManager::basePackage = BasePackage::Avara;
std::vector<std::string> AssetManager::externalPackages = {};
std::shared_ptr<AssetStorage> AssetManager::baseStorage = _baseStorage;
std::shared_ptr<AssetStorage> AssetManager::assetStorage = _localStorage;
std::vector<std::shared_ptr<AssetRepository>> AssetManager::repositoryStack = {
    _localRepo
};
SimpleAssetCache<PackageManifest> AssetManager::manifestCache = {};
SimpleAssetCache<std::string> AssetManager::avarascriptCache = {};
AssetCache<HullConfigRecord> AssetManager::hullCache = {};
AssetCache<nlohmann::json> AssetManager::bspCache = {};
AssetCache<std::vector<uint8_t>> AssetManager::sndCache = {};

std::vector<std::string> AssetManager::GetAvailablePackages()
{
    std::set<std::string> uniquePkgs = {};
    for (auto const &repo : repositoryStack) {
        for (auto const &pkg : *repo->GetPackageList()) {
            uniquePkgs.insert(pkg);
        }
    }

    std::vector<std::string> pkgs(uniquePkgs.begin(), uniquePkgs.end());
    return pkgs;
}

std::optional<std::string> AssetManager::GetResolvedAlfPath(std::string relativePath)
{
    // Attempt to load from packages in priority order.
    for (auto const &pkg : externalPackages) {
        auto path = GetAlfPath(pkg, relativePath);
        std::ifstream file(path);
        if (file.good()) {
            return path;
        }
    }

    // Attempt to fall back to the base package.
    auto path = GetAlfPath(NoPackage, relativePath);
    std::ifstream file(path);
    if (file.good()) {
        return path;
    }

    return std::optional<std::string>{};
}

std::vector<std::shared_ptr<std::string>> AssetManager::GetAllScripts()
{
    std::vector<std::shared_ptr<std::string>> scripts = {};

    // Quick note, the scripts should already be loaded thanks to `Init`, `SwitchBasePackage`, and
    // `BuildDependencyList`. We don't need to try and load them here!

    // Add the base script first.
    if (avarascriptCache.count(NoPackage) > 0) {
        scripts.push_back(avarascriptCache.at(NoPackage));
    }

    // Add scripts from the other packages--in reverse order!
    for (auto pkg = externalPackages.rbegin(); pkg != externalPackages.rend(); ++pkg) {
        if (avarascriptCache.count(*pkg) > 0) {
            scripts.push_back(avarascriptCache.at(*pkg));
        }
    }

    return scripts;
}

std::optional<std::shared_ptr<nlohmann::json>> AssetManager::GetBsp(int16_t id)
{
    // Already cached?
    if (bspCache.count(id) > 0) {
        return bspCache.at(id).data;
    }

    // Attempt to load from packages in priority order.
    for (auto const &pkg : externalPackages) {
        LoadBsp(pkg, id);
        if (bspCache.count(id) > 0) {
            return bspCache.at(id).data;
        }
    }

    // Attempt to fall back to the base package.
    LoadBsp(NoPackage, id);
    if (bspCache.count(id) > 0) {
        return bspCache.at(id).data;
    }

    return std::optional<std::shared_ptr<nlohmann::json>>{};
}

std::optional<std::shared_ptr<HullConfigRecord>> AssetManager::GetHull(int16_t id)
{
    // Already cached?
    if (hullCache.count(id) > 0) {
        return hullCache.at(id).data;
    }

    // Attempt to retrieve from packages in priority order.
    for (auto const &pkg : externalPackages) {
        LoadHull(pkg, id);
        if (hullCache.count(id) > 0) {
            return hullCache.at(id).data;
        }
    }

    // Attempt to fall back to the base package.
    LoadHull(NoPackage, id);
    if (hullCache.count(id) > 0) {
        return hullCache.at(id).data;
    }

    return std::optional<std::shared_ptr<HullConfigRecord>>{};
}

void AssetManager::Init()
{
    LoadManifest(NoPackage);
    LoadScript(NoPackage);
}

OSErr AssetManager::LoadLevel(std::string packageName, std::string relativePath, std::string &levelName)
{
    if (externalPackages.size() == 0 || externalPackages[0] != packageName) {
        SwitchContext(packageName);
    }

    if (manifestCache.count(packageName) == 0) {
        return fnfErr;
    }

    auto manifest = manifestCache.at(packageName);
    std::optional<LevelDirectoryEntry> ledi;
    for (auto const &level : manifest->levelDirectory) {
        if (level.alfPath == relativePath) {
            ledi = level;
        }
    }
    if (ledi) {
        BasePackage newBase = ledi->useAftershock
            ? BasePackage::Aftershock
            : BasePackage::Avara;
        if (basePackage != newBase) {
            SwitchBasePackage(newBase);
        }
        levelName = ledi->levelName;

        // TODO: REMOVE THIS ONCE SOUND LOADING WORKS
        UseLevelFolder(packageName);
        switch (basePackage) {
            case BasePackage::Avara:
                UseBaseFolder("rsrc");
                break;
            case BasePackage::Aftershock:
                UseBaseFolder("rsrc/aftershock");
                break;
        }
        LoadLevelOggFiles(packageName);
        // END OF CODE SLATED FOR DEMOLITION
    } else {
        return fnfErr;
    }

    std::optional<std::string> alfPath = GetResolvedAlfPath(ledi->alfPath);
    if (!alfPath) {
        return fnfErr;
    }

    bool wasSuccessful = LoadALF(*alfPath);
    if (!wasSuccessful) {
        return fnfErr;
    }

    return noErr;
}

bool AssetManager::PackageInStorage(std::string packageName)
{
    return (bool)GetPackagePath(packageName);
}

std::string AssetManager::GetBasePackagePath(BasePackage basePackage) throw()
{
    std::stringstream path;
    path << baseStorage->GetRootPath();
    switch (basePackage) {
        case BasePackage::Avara:
            // No-op.
            break;
        case BasePackage::Aftershock:
            path << PATHSEP << "aftershock";
            break;
        default:
            throw std::invalid_argument("No defined path for base package");
    }
    return path.str();
}

std::optional<std::string> AssetManager::GetPackagePath(std::string packageName)
{
    std::stringstream path;
    path << assetStorage->GetRootPath() << PATHSEP << packageName;

    std::stringstream manifestPath;
    manifestPath << path.str() << PATHSEP << MANIFESTFILE;

    std::ifstream testFile(manifestPath.str());
    return testFile.good()
        ? path.str()
        : std::optional<std::string>{};
}

std::string AssetManager::GetFullPath(MaybePackage package, std::string relativePath)
{
#ifdef _WIN32
    // Ensure path separators are appropriate for Windows.
    std::regex pattern("/");
    relativePath = std::regex_replace(relativePath, pattern, PATHSEP);
#endif
    std::stringstream path;
    if (package) {
        path << assetStorage->GetRootPath() << PATHSEP;
        path << *package << PATHSEP << relativePath;
    } else {
        path << GetBasePackagePath(basePackage) << PATHSEP << relativePath;
    }
    return path.str();
}

std::string AssetManager::GetManifestPath(MaybePackage package)
{
    return GetFullPath(package, MANIFESTFILE);
}

std::string AssetManager::GetScriptPath(MaybePackage package)
{
    return GetFullPath(package, "default.avarascript");
}

std::string AssetManager::GetAlfPath(MaybePackage package, std::string relativePath)
{
    std::stringstream path;
    path << "alf" << PATHSEP << relativePath;
    return GetFullPath(package, path.str());
}

std::string AssetManager::GetBspPath(MaybePackage package, int16_t id)
{
    std::stringstream relativePath;
    relativePath << "bsps" << PATHSEP << id << ".json";
    return GetFullPath(package, relativePath.str());
}

std::string AssetManager::GetOggPath(MaybePackage package, int16_t id)
{
    std::stringstream relativePath;
    relativePath << "ogg" << PATHSEP << id << ".ogg";
    return GetFullPath(package, relativePath.str());
}

void AssetManager::LoadManifest(MaybePackage package)
{
    std::string path = GetManifestPath(package);
    std::ifstream file(path);
    if (file.good()) {
        auto manifest = std::make_shared<PackageManifest>(
            PackageManifest(nlohmann::json::parse(file))
        );
        manifestCache.insert_or_assign(package, manifest);
    }
}

void AssetManager::LoadScript(MaybePackage package)
{
    std::string path = GetScriptPath(package);
    std::ifstream file(path);
    if (file.good()) {
        std::stringstream script;
        script << file.rdbuf();

        avarascriptCache.insert_or_assign(package, std::make_shared<std::string>(script.str()));
    }
}

void AssetManager::LoadBsp(MaybePackage package, int16_t id)
{
    std::string path = GetBspPath(package, id);
    std::ifstream file(path);
    if (file.good()) {
        auto bsp = std::make_shared<nlohmann::json>(nlohmann::json::parse(file));
        Asset<nlohmann::json> asset;
        asset.data = bsp;
        asset.packageName = package;
        bspCache.insert_or_assign(id, asset);
    }
}

void AssetManager::LoadHull(MaybePackage package, int16_t id)
{
    // Note that all manifests should already be loaded due to `Init`, `SwitchBasePackage`, and
    // `BuildDependencyList`.
    auto manifest = manifestCache.at(package);
    if (manifest->hullResources.count(id) > 0) {
        auto hull = std::make_shared<HullConfigRecord>(manifest->hullResources.at(id));
        Asset<HullConfigRecord> asset;
        asset.data = hull;
        asset.packageName = package;
        hullCache.insert_or_assign(id, asset);
    }
}

void AssetManager::SwitchBasePackage(BasePackage newBase)
{
    std::vector<MaybePackage> packageList = {NoPackage};
    manifestCache.RemoveAll(packageList);
    avarascriptCache.RemoveAll(packageList);
    hullCache.RemoveAll(packageList);
    bspCache.RemoveAll(packageList);
    sndCache.RemoveAll(packageList);

    basePackage = newBase;

    LoadManifest(NoPackage);
    LoadScript(NoPackage);
}

void AssetManager::SwitchContext(std::string packageName)
{
    std::vector<std::string> newContext = {};

    BuildDependencyList(packageName, newContext);

    std::vector<MaybePackage> needsCacheClear = {};
    for (auto const &pkg : externalPackages) {
        if (std::find(newContext.begin(), newContext.end(), pkg) == newContext.end()) {
            needsCacheClear.push_back(pkg);
        }
    }

    if (needsCacheClear.size() > 0) {
        manifestCache.RemoveAll(needsCacheClear);
        avarascriptCache.RemoveAll(needsCacheClear);
        hullCache.RemoveAll(needsCacheClear);
        bspCache.RemoveAll(needsCacheClear);
        sndCache.RemoveAll(needsCacheClear);
    }

    externalPackages = newContext;

    ReviewPriorities(bspCache);
    ReviewPriorities(hullCache);
}

void AssetManager::BuildDependencyList(std::string currentPackage, std::vector<std::string> &list)
{
    // Try and load the manifest if it isn't already in the cache.
    if (manifestCache.count(currentPackage) == 0) {
        LoadManifest(currentPackage);
    }

    // Ditto for the avarascript.
    if (avarascriptCache.count(currentPackage) == 0) {
        LoadScript(currentPackage);
    }

    // Technically, LoadManifest might have failed, so we need to check the cache again before we
    // try to read from it. If there's no manifest, technically there's nothing more to do and we
    // can't recurse further down this branch.
    if (manifestCache.count(currentPackage) > 0) {
        // Add this package to the list as long as it isn't in the list already.
        if (std::find(list.begin(), list.end(), currentPackage) == list.end()) {
            list.push_back(currentPackage);
        }

        auto manifest = manifestCache.at(currentPackage);
        for (auto const &req : manifest->requiredPackages) {
            BuildDependencyList(req.packageName, list);
        }
    }
}

template <>
void AssetManager::ReviewPriorities(AssetCache<nlohmann::json> &cache)
{
    std::vector<int16_t> needsRemoval = {};
    for (auto const &[id, asset] : cache) {
        MaybePackage assetPkg = asset.packageName;
        for (auto const &pkg : externalPackages) {
            if (assetPkg == pkg) {
                // We've reached the point in the package list where the cached asset is of equal
                // or higher priority than the remaining packages, so we can stop looking for this
                // particular asset ID.
                break;
            }

            std::string path = GetBspPath(pkg, id);
            std::ifstream testFile(path);
            if (testFile.good()) {
                needsRemoval.push_back(id);

                // We've found a higher priority asset than the one in the cache, so we can stop
                // looking for this particular asset ID.
                break;
            }
        }
    }
    for (auto &id : needsRemoval) {
        cache.erase(id);
    }
};

template <>
void AssetManager::ReviewPriorities(AssetCache<HullConfigRecord> &cache)
{
    std::vector<int16_t> needsRemoval = {};
    for (auto const &[id, asset] : cache) {
        MaybePackage assetPkg = asset.packageName;
        for (auto const &pkg : externalPackages) {
            if (assetPkg == pkg) {
                // We've reached the point in the package list where the cached asset is of equal
                // or higher priority than the remaining packages, so we can stop looking for this
                // particular asset ID.
                break;
            }

            auto manifest = manifestCache.at(pkg);
            if (manifest->hullResources.count(id) > 0) {
                needsRemoval.push_back(id);

                // We've found a higher priority asset than the one in the cache, so we can stop
                // looking for this particular asset ID.
                break;
            }
        }
    }
    for (auto &id : needsRemoval) {
        cache.erase(id);
    }
};
