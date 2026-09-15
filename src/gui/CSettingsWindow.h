//
//  CSettingsWindow.h
//  Avara
//
//  Created by Andy Halstead on 8/18/26.
//

#ifndef CSettingsWindow_h
#define CSettingsWindow_h

#include "CWindow.h"
#include "CKeyboardMappingWindow.h"

class CSettingsWindow : public CWindow {
public:
    CSettingsWindow(CApplication *app);

    virtual ~CSettingsWindow();

    virtual bool editing();
    bool handleSDLEvent(SDL_Event &event);
    bool currentlyMapping();
    CKeyboardMappingWindow* getKeyMapWindow();
    void refreshKeyboardMappingWindow(int status);
protected:
    nanogui::TabWidget *mPrefTabs;
    int keyboardIconsDataHandle;
    bool currentlyMappingKey;
    std::string currentlyMappingAction;
    std::string currentlyMappingActionDesc;
    int currentlyMappingKeyboardIconOffset;
    nanogui::Button* currentlyMappingButton;
    CKeyboardMappingWindow* keyMapWindow;
};

#endif /* CSettingsWindow_hpp */
