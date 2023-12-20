#include "AssetManager.h"
#include "BaseAssetStorage.h"
#include "LocalAssetRepository.h"

#include <fstream>
#include <set>

template <typename T>
void AssetCache<T>::RemoveAll(std::vector<std::optional<std::string>> packageList)
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
    path << _localStorage->GetRootPath() << PATHSEP << packageName;

    std::stringstream manifestPath;
    manifestPath << path.str() << PATHSEP << MANIFESTFILE;

    std::ifstream testFile(manifestPath.str());
    return testFile.good()
        ? path.str()
        : std::optional<std::string>{};
}

void AssetManager::LoadLevel(std::string packageName, std::string levelTag)
{
    SwitchContext(packageName);
}

std::string AssetManager::GetBasePackagePath(BasePackage basePackage) throw()
{
    std::stringstream path;
    path << _baseStorage->GetRootPath();
    switch (basePackage) {
        case Avara:
            // No-op.
            break;
        case Aftershock:
            path << PATHSEP << "aftershock";
            break;
        default:
            throw std::invalid_argument("No defined path for base package");
    }
    return path.str();
}

void AssetManager::SwitchBasePackage(BasePackage newBase)
{
    if (basePackage != newBase) {
        std::vector<MaybePackage> packageList = {NoPackage};
        bspCache.RemoveAll(packageList);
        sndCache.RemoveAll(packageList);
        basePackage = newBase;
    }
}

void AssetManager::SwitchContext(std::string packageName)
{
    if (externalPackages.size() == 0 || externalPackages[0] != packageName) {
        std::vector<std::string> oldContext = externalPackages;
        externalPackages.clear();
        // TODO: repopulate externalPackages and store manifests
    }
}
