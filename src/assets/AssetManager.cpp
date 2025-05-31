#include "AssetManager.h"
#include "BaseAssetStorage.h"
#include "LocalAssetRepository.h"
#include "BasePath.h"
#include "LevelLoader.h"

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
std::shared_ptr<nlohmann::json> AssetManager::objectTypes = nullptr;
SimpleAssetCache<PackageManifest> AssetManager::manifestCache = {};
SimpleAssetCache<std::string> AssetManager::avarascriptCache = {};
AssetCache<nlohmann::json> AssetManager::bspCache = {};
AssetCache<OggFile> AssetManager::sndCache = {};
AssetCache<HullConfigRecord> AssetManager::hullCache = {};
AssetCache<BSPSRecord> AssetManager::bspsCache = {};

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

std::shared_ptr<nlohmann::json> AssetManager::GetEnumeratedObjectTypes()
{
    if (objectTypes == nullptr) {
        LoadEnumeratedObjectTypes();
    }

    return objectTypes;
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

std::optional<std::string> AssetManager::GetShaderPath(std::string relativePath)
{
    std::stringstream path;
    path << baseStorage->GetRootPath() << PATHSEP << "shaders" << PATHSEP << relativePath;

    std::ifstream file(path.str());
    if (file.good()) {
        return path.str();
    }

    return std::optional<std::string>{};
}

std::optional<std::shared_ptr<PackageManifest>> AssetManager::GetManifest(MaybePackage package)
{
    if (manifestCache.count(package) > 0) {
        return manifestCache.at(package);
    }

    // If it isn't loaded, *don't* call `LoadManifest` to prevent it from being cached. It's
    // most likely being requested for UI purposes.
    std::string path = GetManifestPath(package);
    std::ifstream file(path);
    if (file.good()) {
        return std::make_shared<PackageManifest>(nlohmann::json::parse(file));
    }

    return std::optional<std::shared_ptr<PackageManifest>>{};
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
    if (bspCache.count(id) > 0) {
        return bspCache.at(id).data;
    }

    LoadBsp(id);
    if (bspCache.count(id) > 0) {
        return bspCache.at(id).data;
    }

    return std::optional<std::shared_ptr<nlohmann::json>>{};
}

std::optional<std::shared_ptr<BSPSRecord>> AssetManager::GetBspScale(int16_t id)
{
    if (bspsCache.count(id) > 0) {
        return bspsCache.at(id).data;
    }

    LoadBspScale(id);
    if (bspsCache.count(id) > 0) {
        return bspsCache.at(id).data;
    }

    return std::optional<std::shared_ptr<BSPSRecord>>{};
}

std::optional<std::shared_ptr<OggFile>> AssetManager::GetOgg(int16_t id)
{
    if (sndCache.count(id) > 0) {
        return sndCache.at(id).data;
    }

    LoadOgg(id);
    if (sndCache.count(id) > 0) {
        return sndCache.at(id).data;
    }

    return std::optional<std::shared_ptr<OggFile>>{};
}

std::optional<std::shared_ptr<HullConfigRecord>> AssetManager::GetHull(int16_t id)
{
    if (hullCache.count(id) > 0) {
        return hullCache.at(id).data;
    }

    LoadHull(id);
    if (hullCache.count(id) > 0) {
        return hullCache.at(id).data;
    }

    return std::optional<std::shared_ptr<HullConfigRecord>>{};
}

void AssetManager::Init()
{
    LoadManifest(NoPackage);
    LoadScript(NoPackage);
    LoadEnumeratedObjectTypes();
    InitParser();
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

std::string AssetManager::GetBasePackagePath(BasePackage basePackage)
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

std::string AssetManager::GetImagePath(MaybePackage package, std::string fileName) {
    std::stringstream relativePath;
    relativePath << "img" << PATHSEP << fileName;
    return GetFullPath(package, relativePath.str());
}

void AssetManager::LoadEnumeratedObjectTypes()
{
    std::stringstream path;
    path << baseStorage->GetRootPath() << PATHSEP << "objects.json";
    std::ifstream file(path.str());
    if (file.fail()) {
        throw std::runtime_error("Failed to load objects.json");
    } else {
        objectTypes = std::make_shared<nlohmann::json>(nlohmann::json::parse(file));
    }
}

void AssetManager::LoadManifest(MaybePackage package)
{
    std::string path = GetManifestPath(package);
    std::ifstream file(path);
    if (file.good()) {
        auto manifest = std::make_shared<PackageManifest>(nlohmann::json::parse(file));
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

void AssetManager::LoadBsp(int16_t id)
{
    // Attempt to retrieve from packages in priority order.
    for (auto const &pkg : externalPackages) {
        std::string path = GetBspPath(pkg, id);
        std::ifstream file(path);
        if (file.good()) {
            Asset<nlohmann::json> asset;
            asset.data = std::make_shared<nlohmann::json>(nlohmann::json::parse(file));;
            asset.packageName = pkg;
            bspCache.insert_or_assign(id, asset);
            return;
        }
    }

    // Fallback to base package.
    std::string path = GetBspPath(NoPackage, id);
    std::ifstream file(path);
    if (file.good()) {
        Asset<nlohmann::json> asset;
        asset.data = std::make_shared<nlohmann::json>(nlohmann::json::parse(file));;
        asset.packageName = NoPackage;
        bspCache.insert_or_assign(id, asset);
    }
}

void AssetManager::LoadBspScale(int16_t id)
{
    // Note that all manifests should already be loaded due to `Init`, `SwitchBasePackage`, and
    // `BuildDependencyList`.

    // Attempt to retrieve from packages in priority order.
    for (auto const &pkg : externalPackages) {
        auto manifest = manifestCache.at(pkg);
        if (manifest->bspsResources.count(id) > 0) {
            Asset<BSPSRecord> asset;
            asset.data = std::make_shared<BSPSRecord>(manifest->bspsResources.at(id));
            asset.packageName = pkg;
            bspsCache.insert_or_assign(id, asset);
            return;
        }
    }

    // Fallback to base package.
    auto manifest = manifestCache.at(NoPackage);
    if (manifest->bspsResources.count(id) > 0) {
        Asset<BSPSRecord> asset;
        asset.data = std::make_shared<BSPSRecord>(manifest->bspsResources.at(id));
        asset.packageName = NoPackage;
        bspsCache.insert_or_assign(id, asset);
    }
}

void AssetManager::LoadOgg(int16_t id)
{
    // Note that all manifests should already be loaded due to `Init`, `SwitchBasePackage`, and
    // `BuildDependencyList`.

    std::optional<std::string> path = {};
    std::optional<HSNDRecord> hsnd = {};

    // Track which package has the highest priority for this particular asset. This is tricky for
    // oggs since the asset is split into two parts, the actual ogg file and the HSND metadata.
    // We need to know what the highest priority is so the cache can be cleared appropriately when
    // needed later on.
    size_t highestPriorityPkgIdx = externalPackages.size();

    // Attempt to retrieve from packages in priority order.
    size_t idx = 0;
    for (auto const &pkg : externalPackages) {
        std::string testPath = GetOggPath(pkg, id);
        std::ifstream testFile(testPath);
        if (testFile.good()) {
            path = testPath;
            highestPriorityPkgIdx = std::min(idx, highestPriorityPkgIdx);
        }

        auto manifest = manifestCache.at(pkg);
        if (manifest->hsndResources.count(id) > 0) {
            hsnd = manifest->hsndResources.at(id);
            highestPriorityPkgIdx = std::min(idx, highestPriorityPkgIdx);
        }

        if (path && hsnd) {
            break;
        }

        idx++;
    }

    // Fallback to base package.
    if (!path) {
        std::string testPath = GetOggPath(NoPackage, id);
        std::ifstream testFile(testPath);
        if (testFile.good()) {
            path = testPath;
        }
    }

    if (!hsnd) {
        auto manifest = manifestCache.at(NoPackage);
        if (manifest->hsndResources.count(id) > 0) {
            hsnd = manifest->hsndResources.at(id);
        } else {
            // If we *still* don't have an HSND, default to 129 in the base package.
            hsnd = manifest->hsndResources.at(129);
        }
    }

    if (path && hsnd) {
        Asset<OggFile> asset;
        asset.data = std::make_shared<OggFile>(*path, *hsnd);
        asset.packageName = (highestPriorityPkgIdx < externalPackages.size())
            ? externalPackages[highestPriorityPkgIdx]
            : NoPackage;
        sndCache.insert_or_assign(id, asset);
    }
}

void AssetManager::LoadHull(int16_t id)
{
    // Note that all manifests should already be loaded due to `Init`, `SwitchBasePackage`, and
    // `BuildDependencyList`.

    // Attempt to retrieve from packages in priority order.
    for (auto const &pkg : externalPackages) {
        auto manifest = manifestCache.at(pkg);
        if (manifest->hullResources.count(id) > 0) {
            Asset<HullConfigRecord> asset;
            asset.data = std::make_shared<HullConfigRecord>(manifest->hullResources.at(id));
            asset.packageName = pkg;
            hullCache.insert_or_assign(id, asset);
            return;
        }
    }

    // Fallback to base package.
    auto manifest = manifestCache.at(NoPackage);
    if (manifest->hullResources.count(id) > 0) {
        Asset<HullConfigRecord> asset;
        asset.data = std::make_shared<HullConfigRecord>(manifest->hullResources.at(id));
        asset.packageName = NoPackage;
        hullCache.insert_or_assign(id, asset);
    }
}

void AssetManager::SwitchBasePackage(BasePackage newBase)
{
    std::vector<MaybePackage> packageList = {NoPackage};
    manifestCache.RemoveAll(packageList);
    avarascriptCache.RemoveAll(packageList);
    bspCache.RemoveAll(packageList);
    sndCache.RemoveAll(packageList);
    hullCache.RemoveAll(packageList);

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
        bspCache.RemoveAll(needsCacheClear);
        sndCache.RemoveAll(needsCacheClear);
        hullCache.RemoveAll(needsCacheClear);
        bspsCache.RemoveAll(needsCacheClear);
    }

    externalPackages = newContext;

    ReviewPriorities(bspCache);
    ReviewPriorities(sndCache);
    ReviewPriorities(hullCache);
    ReviewPriorities(bspsCache);
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

void AssetManager::ReviewPriorities(AssetCache<OggFile> &cache)
{
    std::vector<int16_t> needsRemoval = {};
    for (auto const &[id, asset] : cache) {
        MaybePackage assetPkg = asset.packageName;
        bool foundHsnd = false;
        bool foundOgg = false;
        for (auto const &pkg : externalPackages) {
            if (assetPkg == pkg) {
                // We've reached the point in the package list where the cached asset is of equal
                // or higher priority than the remaining packages, so we can stop looking for this
                // particular asset ID.
                break;
            }

            auto manifest = manifestCache.at(pkg);
            if (manifest->hsndResources.count(id) > 0) {
                foundHsnd = true;
            }

            std::string path = GetOggPath(pkg, id);
            std::ifstream testFile(path);
            if (testFile.good()) {
                foundOgg = true;
            }

            if (foundHsnd || foundOgg) {
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
}

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
}

void AssetManager::ReviewPriorities(AssetCache<BSPSRecord> &cache)
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
            if (manifest->bspsResources.count(id) > 0) {
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
