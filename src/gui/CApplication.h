#pragma once

#include "CWindow.h"

#include <SDL2/SDL.h>
#include <json.hpp>
#include <nanogui/nanogui.h>
#include <string>
#include <vector>

using json = nlohmann::json;

class CApplication : public nanogui::Screen {
public:
    CApplication(std::string title);
    virtual ~CApplication();

    void Register(CWindow *win) { windowList.push_back(win); }
    void Unregister(CWindow *win);

    // Broadcast a command to myself, and then any windows, stopping at the fist one to handle it.
    virtual void BroadcastCommand(int theCommand);

    // Called when the application is done and the process is about to exit.
    virtual void Done();

    // Handles a command broadcasted by BroadcastCommand. Returns true if it was actually handled.
    virtual bool DoCommand(int theCommand) { return false; }

    // Events that were not handled by nanogui.
    virtual bool HandleEvent(SDL_Event &event) { return false; }

    // Called when preference values change.
    virtual void PrefChanged(std::string name);

    virtual void WindowResized(int width, int height) {}

    // Screen overrides.
    virtual bool handleSDLEvent(SDL_Event &event);

    // templated version works for new types too (float, double, short, etc.)
    template <class T> T Get(const std::string name) {
        // throws json::out_of_range exception if 'name' not a known key
        try {
            return static_cast<T>(_prefs.at(name));
        }
        catch (json::type_error &ex) {
            // Type from json didn't match requested type so use hardcoded default instead
            SDL_Log("Type Error trying to read '%s'. Using default", name.c_str());
            return static_cast<T>(_defaultPrefs.at(name));
        }
    }
    // these getters are here for backwards compatibility and/or readability
    std::string String(const std::string name)       { return Get<std::string>(name); };
    long Number(const std::string name)              { return Get<long>(name); };
    long Number(const std::string name, const long defaultValue);
    bool Boolean(const std::string name)             { return Get<bool>(name); }
    json Get(const std::string name)                 { return Get<json>(name); }

    std::vector<const std::string> Matches(const std::string matchStr) {
        std::vector<const std::string> results;
        for (auto& el : _prefs.items()) {
            if (el.key().find(matchStr) != std::string::npos) {
//                std::cout << el.key() << " : " << el.value() << "\n";
                results.push_back(el.key());
            }
        }
        return results;
    }

    template <class T> void Set(const std::string name, const T value) {
        _prefs[name] = value;
        PrefChanged(name);
    }

    void Update(const std::string name, std::string &value) {
        // construct json from the inputs and update the internal JSON object
        if (_prefs.at(name).is_string()) {
            // wrap string values in quotes
            value = '"' + value + '"';
        }
        json updatePref = json::parse("{ \"" + name + "\": " + value + "}");
        _prefs.update(updatePref);
    }

protected:
    static json _prefs;
    static json _defaultPrefs;
    std::vector<CWindow *> windowList;
};

#ifdef APPLICATIONMAIN
CApplication *gApplication;
#else
extern CApplication *gApplication;
#endif
