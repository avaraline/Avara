#include "AssetManager.h"
#include "BaseAssetStorage.h"
#include "LocalAssetRepository.h"

#include <algorithm>
#include <fstream>
#include <regex>
#include <set>

template <typename T>
void SimpleCacheRemoveAll(SimpleAssetCache<T> &simpleCache,
                          std::vector<MaybePackage> &packageList)
{
    for (auto const &pkg : packageList) {
        simpleCache.erase(pkg);
    }
};

template <typename T>
void AssetCache<T>::RemoveAll(std::vector<MaybePackage> &packageList)
{
    std::vector<int16_t> cacheHits = {};
    for (auto const &[id, asset] : collection) {
        for (auto &pkg : packageList) {
            if (pkg == asset.packageName) {
                cacheHits.push_back(id);
            }
        }
    }
    for (auto &id : cacheHits) {
        collection.erase(id);
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
AssetCache<HullConfigRecord> AssetManager::hullCache = AssetCache<HullConfigRecord>();
AssetCache<nlohmann::json> AssetManager::bspCache = AssetCache<nlohmann::json>();
AssetCache<std::vector<uint8_t>> AssetManager::sndCache = AssetCache<std::vector<uint8_t>>();

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

OSErr AssetManager::LoadLevel(std::string packageName, std::string levelTag)
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
        if (level.alfPath == levelTag) {
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
    } else {
        return fnfErr;
    }

    // TODO: actually load the level

    return noErr;
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
        std::string contents;
        file.seekg(0, std::ios::end);
        contents.reserve(file.tellg());
        file.seekg(0, std::ios::beg);

        contents.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

        auto script = std::make_shared<std::string>(contents);
        avarascriptCache.insert_or_assign(package, script);
    }
}

void AssetManager::SwitchBasePackage(BasePackage newBase)
{
    std::vector<MaybePackage> packageList = {NoPackage};
    SimpleCacheRemoveAll(manifestCache, packageList);
    SimpleCacheRemoveAll(avarascriptCache, packageList);
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
        SimpleCacheRemoveAll(manifestCache, needsCacheClear);
        SimpleCacheRemoveAll(avarascriptCache, needsCacheClear);
        hullCache.RemoveAll(needsCacheClear);
        bspCache.RemoveAll(needsCacheClear);
        sndCache.RemoveAll(needsCacheClear);
    }

    externalPackages = newContext;
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
