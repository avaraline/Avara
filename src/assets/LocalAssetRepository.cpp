#include "LocalAssetRepository.h"
#include "AssetManager.h"

#define CUTE_FILES_IMPLEMENTATION
#include <cute_files.h>

#include <fstream>
#include <sstream>

std::shared_ptr<LocalAssetRepository> LocalAssetRepository::GetInstance() {
    static std::shared_ptr<LocalAssetRepository> instance
        = std::make_shared<LocalAssetRepository>(LocalAssetRepository());
    return instance;
};

std::shared_ptr<std::vector<std::string>> LocalAssetRepository::GetPackageList()
{
    if (!populatedList) {
        BuildPackageList();
    }
    return packageList;
}

std::optional<std::string> LocalAssetRepository::GetPackagePath(std::string packageName)
{
    std::stringstream path;
    path << SDL_GetBasePath() << PATHSEP << LEVELDIR << packageName;

    std::stringstream manifestPath;
    manifestPath << path.str() << PATHSEP << MANIFESTFILE;

    std::ifstream testFile(manifestPath.str());
    return testFile.fail()
        ? std::optional<std::string>{}
        : path.str();
}

void LocalAssetRepository::Refresh()
{
    packageList->clear();
    populatedList = false;
}

void LocalAssetRepository::BuildPackageList()
{
    std::stringstream path;
    path << SDL_GetBasePath() << PATHSEP << LEVELDIR;

    cf_dir_t dir;
    cf_dir_open(&dir, path.str().c_str());

    while (dir.has_next) {
        cf_file_t file;
        cf_read_file(&dir, &file);
        std::string filename = std::string(file.name);
        if (file.is_dir > 0 &&
            filename.size() >= 2 &&
            filename.compare(0, 1, ".") != 0 &&
            filename.compare(0, 2, "..") != 0) {
            // This is a directory, check to see if there's a manifest inside.
            path.str("");
            path << SDL_GetBasePath() << PATHSEP << LEVELDIR << PATHSEP;
            path << filename << PATHSEP << MANIFESTFILE;
            if (cf_file_exists(path.str().c_str())) {
                packageList->push_back(filename);
            }
        }
        cf_dir_next(&dir);
    }

    cf_dir_close(&dir);

    // Sort alphabetically.
    std::sort(packageList->begin(), packageList->end(),
        [](std::string &a, std::string &b) -> bool {
            return a < b;
        }
    );

    populatedList = true;
};
