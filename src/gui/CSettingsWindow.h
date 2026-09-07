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

    virtual bool editing() { return false; }
protected:
    nanogui::TabWidget *mPrefTabs;
    int keyboardIconsDataHandle;
};

#endif /* CSettingsWindow_hpp */
