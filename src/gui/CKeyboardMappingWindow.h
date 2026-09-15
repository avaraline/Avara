//
//  CKeyboardMappingWindow.hpp
//  Avara
//
//  Created by Andy Halstead on 8/24/26.
//

#ifndef CKeyboardMappingWindow_h
#define CKeyboardMappingWindow_h

#include "CWindow.h"
#include "NVGUtil.h"

class CKeyboardMappingWindow : public CWindow {
public:
    CKeyboardMappingWindow(CApplication *app, const std::string &actionDesc, const std::string &key, int imageOffset, int imageHandle);
    virtual bool editing();
    bool handleSDLEvent(SDL_Event &event);
    void gatherKey();
    bool gathering = false;
    std::function<void(int)> callback() const { return mCallback; }
    void setCallback(const std::function<void(int)> &callback) { mCallback = callback; }
protected:
    std::function<void(int)> mCallback;
    void removeMappingButton(const std::string &action, const std::string &key);
    void addKeyBind(const std::string &keyname);
    void removeKeyBind(const std::string &keyname);
    std::string *theKey;
    SpriteWidget *actionIcon;
    nanogui::Label *actionLabel;
    nanogui::Label *currentLabel;
    nanogui::Widget *currentlyMappedKeys;
    nanogui::Button *addbtn;
    std::string *currentlyMapping;
    std::vector<std::string> keys;

};


#endif /* CKeyboardMappingWindow_h */
