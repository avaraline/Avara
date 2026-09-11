//
//  CMainMenu.hpp
//  Avara
//
//  Created by Andy Halstead on 8/21/26.
//

#ifndef CMainMenu_hpp
#define CMainMenu_hpp

#include "nanogui/nanogui.h"
#include "CApplication.h"
#include "NVGUtil.h"

class CAvaraAppImpl;

class CMainMenu : public nanogui::Widget {
public:
    CMainMenu(CAvaraAppImpl *app, NVGcontext *ctx);
    void draw(NVGcontext *ctx);
private:
    int titleImageDataHandle;
    int titleImageW, titleImageH, titleImageOffsetX, titleImageOffsetY;
    int titleW, titleH, paddingX, paddingY;
    std::vector<nanogui::Button*> homeScreenButtons;
    CAvaraAppImpl* app;
    nanogui::Widget* placeholder;
    void addHomeScreenButton(const std::string text, const std::function<void()> &callback);
    void addHomeScreenButton(const std::string text, GUIState target);
};

#endif /* CMainMenu_hpp */
