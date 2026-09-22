//
//  CMainMenu.cpp
//  Avara
//
//  Created by Andy Halstead on 8/21/26.
//

#include "CMainMenu.h"
#include "AssetManager.h"
#include "CAvaraApp.h"
#include "NVGUtil.h"

CMainMenu::CMainMenu(CAvaraAppImpl *app, NVGcontext *ctx) : nanogui::Widget((nanogui::Widget *)app) {
    this->app = app;
    std::string titleImgPath = AssetManager::GetImagePath(NoPackage, "avara-logo-only-transparent.png");
    titleImageDataHandle = nvgCreateImage(ctx, titleImgPath.c_str(), 0);
    titleImageW = 0;
    titleImageH = 0;
    nvgImageSize(ctx, titleImageDataHandle, &titleImageW, &titleImageH);

    setLayout(new nanogui::FlowLayout(nanogui::Orientation::Vertical, true, 0, 25));
    placeholder = new nanogui::Widget(this);

    updateAspectRatio();
    addHomeScreenButton("Play Online", GUIState::tracker);
    addHomeScreenButton("Host Server", GUIState::hostSettings);
    addHomeScreenButton("Single Player", GUIState::singlePlayer);
    addHomeScreenButton("Settings", GUIState::settings);
    //addHomeScreenButton("About", GUIState::about);
    addHomeScreenButton("Quit", [] { gApplication->Done(); });
}

void CMainMenu::updateAspectRatio() {
    auto screenX = screen()->width();
    auto screenY = screen()->height();
    setFixedSize(nanogui::Vector2i(screenX, screenY));
    paddingX = 25;
    paddingY = 25;
    titleW = screenX / (phi * 1.6);
    titleH = ((screenX / (phi * 1.6)) / titleImageW) * titleImageH;

    placeholder->setFixedWidth(screen()->width());
    placeholder->setFixedHeight(titleH);
    setNeedsLayout();
}

void CMainMenu::addHomeScreenButton(const std::string text, const std::function<void()> &callback) {
    auto button = new nanogui::Button(this, text);
    button->setCallback(callback);
    //button->setFixedWidth(homeScreenButtonWidth);
    button->setTextPosition(nanogui::Button::TextPosition::Left);
    homeScreenButtons.push_back(button);
}

void CMainMenu::addHomeScreenButton(const std::string text, GUIState target) {
    addHomeScreenButton(text, [this, target] { app->UpdateGUIState(target); });
}

void CMainMenu::draw(NVGcontext *ctx) {
    DrawImage(ctx, titleImageDataHandle, 1, 0, 0, titleImageW, titleImageH, paddingX, paddingY, titleW, titleH);
    Widget::draw(ctx);
}
