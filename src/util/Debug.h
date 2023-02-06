#pragma once

#include <iostream>
#include <string>
#include <map>

#define DBG_Log(key, ...)  if (Debug::IsEnabled(key)) { SDL_Log( key "> " __VA_ARGS__); }

class Debug {
private:
    static std::map<std::string, int> debugMap;

public:
    // debug flags that can be toggled on/off by user
    static bool Toggle(const std::string& key);
    static bool IsEnabled(const std::string& key);
    // set/get a debug flag value
    static int SetValue(const std::string& key, int value);
    static int GetValue(const std::string& key);
};
