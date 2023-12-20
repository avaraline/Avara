#include "AssetManager.h"

#include <SDL2/SDL.h>

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

std::shared_ptr<AssetRepository> localRepo = LocalAssetRepository::GetInstance();

// Initialize static variables.
BasePackage AssetManager::basePackage = BasePackage::Avara;
std::vector<std::string> AssetManager::externalPackages = {};
std::vector<std::shared_ptr<AssetRepository>> AssetManager::repositoryStack = {
    localRepo
};
AssetCache<nlohmann::json> AssetManager::bspCache = AssetCache<nlohmann::json>();
AssetCache<std::vector<uint8_t>> AssetManager::sndCache = AssetCache<std::vector<uint8_t>>();

void AssetManager::LoadLevel(std::string packageName, std::string levelTag)
{
    SwitchContext(packageName);
}

std::string AssetManager::GetBasePackagePath(BasePackage basePackage) throw()
{
    std::stringstream path;
    path << SDL_GetBasePath() << PATHSEP;
    switch (basePackage) {
        case Avara:
            path << "rsrc";
            break;
        case Aftershock:
            path << "rsrc" << PATHSEP << "aftershock";
            break;
        default:
            throw std::invalid_argument("No defined path for base package");
    }
    return path.str();
}

std::optional<std::string> AssetManager::GetPackagePath(std::string packageName)
{
    for (auto const &repo : AssetManager::repositoryStack) {
        std::optional<std::string> foundPath = repo->GetPackagePath(packageName);
        if (foundPath) {
            return foundPath;
        }
    }
    return std::optional<std::string>{};
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
