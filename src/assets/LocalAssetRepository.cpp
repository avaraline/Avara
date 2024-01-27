#include "LocalAssetRepository.h"
#include "AssetManager.h"

#define CUTE_FILES_IMPLEMENTATION
#include <cute_files.h>

#include <SDL2/SDL.h>

#include <fstream>
#include <sstream>

LocalAssetRepository::LocalAssetRepository()
{
    char *sdlPathTmp = SDL_GetBasePath();
    basePath = std::string(sdlPathTmp);
    SDL_free(sdlPathTmp);
}

std::shared_ptr<LocalAssetRepository> LocalAssetRepository::GetInstance()
{
    static auto instance = std::make_shared<LocalAssetRepository>(LocalAssetRepository());
    return instance;
};

std::shared_ptr<std::vector<std::string>> LocalAssetRepository::GetPackageList()
{
    if (!populatedList) {
        BuildPackageList();
    }
    return packageList;
}

std::string LocalAssetRepository::GetRootPath()
{
    std::stringstream path;
    path << basePath << "levels";
    return path.str();
}

void LocalAssetRepository::Refresh()
{
    packageList->clear();
    populatedList = false;
}

void LocalAssetRepository::BuildPackageList()
{
    std::string rootPath(GetRootPath());

    cf_dir_t dir;
    cf_dir_open(&dir, rootPath.c_str());

    while (dir.has_next) {
        cf_file_t file;
        cf_read_file(&dir, &file);
        std::string filename = std::string(file.name);
        if (file.is_dir > 0 &&
            filename.size() >= 2 &&
            filename.compare(0, 1, ".") != 0 &&
            filename.compare(0, 2, "..") != 0) {
            // This is a directory, check to see if there's a manifest inside.
            std::stringstream manifestPath;
            manifestPath << rootPath << PATHSEP << filename << PATHSEP << MANIFESTFILE;

            std::ifstream testFile(manifestPath.str());
            if (testFile.good()) {
                packageList->push_back(filename);
            }
        }
        cf_dir_next(&dir);
    }

    cf_dir_close(&dir);

    populatedList = true;
};
