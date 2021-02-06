#pragma once

#include "CDirectObject.h"

#include <json.hpp>
#include <string>
#include <vector>

nlohmann::json LoadLevelListFromJSON(std::string set);
nlohmann::json GetManifestJSON(std::string set);