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

protected:
    void removeMappingButton(const std::string &action, const std::string &key);
    SpriteWidget *actionIcon;
    nanogui::Label *actionLabel;
    nanogui::Label *currentLabel;
    nanogui::Widget *currentlyMappedKeys;
    nanogui::Button *addbtn;
    std::string *currentlyMapping;
    std::vector<std::string> keys;
    bool gathering = false;

};


#endif /* CKeyboardMappingWindow_h */
