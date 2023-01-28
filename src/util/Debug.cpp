#include "Debug.h"

std::map<std::string, int> Debug::debugMap;

// toggle a flag on/off
bool Debug::Toggle(const std::string& key) {
    bool hasKey = IsEnabled(key);
    if (hasKey) {
        debugMap.erase(key);
    } else {
        debugMap[key] = -1;  // value doesn't matter for on/off debug
    }
    return !hasKey;
}

bool Debug::IsEnabled(const std::string& key) {
    return (debugMap.find(key) != debugMap.end());
}

int Debug::SetValue(const std::string& key, int value) {
    return debugMap[key] = value;
}

int Debug::GetValue(const std::string& key) {
    if (debugMap.find(key) != debugMap.end()) {
        return debugMap[key];
    }
    return -1;
}
