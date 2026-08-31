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
    CKeyboardMappingWindow(CApplication *app);

    virtual ~CKeyboardMappingWindow();

    virtual bool editing() { return false; };
    bool handleSDLEvent(SDL_Event &event);

    void startMapping(const std::string &key, int imageOffset);
    void gatherKey();
    void setImage(int keyImageHandle);

protected:
    SpriteWidget *actionIcon;
    nanogui::Label *actionLabel;
    nanogui::Label *currentLabel;
    nanogui::Widget *currentlyMappedKeys;
    std::string *currentlyMapping;
    std::vector<std::string> keys;
    bool gathering = false;

};


#endif /* CKeyboardMappingWindow_h */
