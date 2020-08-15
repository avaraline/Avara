#pragma once

#include "CDirectObject.h"
#include <string>
#include <vector>
#include <json.hpp>

nlohmann::json LoadLevelListFromJSON(std::string set);
nlohmann::json GetManifestJSON(std::string set);