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
    std::string titleImgPath = AssetManager::GetImagePath(NoPackage, "avara_wordmark.png");
    titleImageDataHandle = nvgCreateImage(ctx, titleImgPath.c_str(), 0);
    titleImageW = 0;
    titleImageH = 0;
    nvgImageSize(ctx, titleImageDataHandle, &titleImageW, &titleImageH);
    auto paddingterm = (phi * 80);
    auto screenX = screen()->width();
    auto screenY = screen()->height();
    paddingX = 25;
    paddingY = screenY / paddingterm;
    titleW = screenX / phi;
    titleH = ((screenX / phi) / titleImageW) * titleImageH;
    titleImageOffsetX = paddingX;
    titleImageOffsetY = paddingY * 4;

    homeScreenButtonWidth = titleW / phicube;

    setLayout(new nanogui::FlowLayout(nanogui::Orientation::Vertical, true, 0, paddingY));
    auto placeHolder = new nanogui::Widget(this);
    placeHolder->setWidth(titleW);
    placeHolder->setHeight(titleH + (titleImageOffsetY * 2));

    addHomeScreenButton("Play Online", GUIState::tracker);
    addHomeScreenButton("Host Server", GUIState::hostSettings);
    addHomeScreenButton("Single Player", GUIState::singlePlayer);
    addHomeScreenButton("Settings", GUIState::settings);
    addHomeScreenButton("Quit", [] { gApplication->Done(); });
}

void CMainMenu::addHomeScreenButton(const std::string text, const std::function<void()> &callback) {
    auto button = new nanogui::Button(this, text);
    button->setCallback(callback);
    //button->setFixedWidth(homeScreenButtonWidth);
    button->setTextPosition(nanogui::Button::TextPosition::Left);
    homeScreenButtons.push_back(button);
}

void CMainMenu::addHomeScreenButton(const std::string text, GUIState target) {
    addHomeScreenButton(text, [this, target] { app->SetGUIState(target); });
}


void CMainMenu::draw(NVGcontext *ctx) {
    Widget::draw(ctx);
    titleW = screen()->width() / phi;
    titleH = ((screen()->width() / phi) / titleImageW) * titleImageH;
    titleImageOffsetX = paddingX;
    titleImageOffsetY = paddingY * 4;
    titleImage = nvgImagePattern(ctx, titleImageOffsetX, titleImageOffsetY, titleW, titleH, 0, titleImageDataHandle, 1);
    nvgBeginPath(ctx);
    nvgFillPaint(ctx, titleImage);
    nvgRect(ctx, titleImageOffsetX, titleImageOffsetY, titleW, titleH);
    nvgFill(ctx);
}
