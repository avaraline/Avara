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
    setFixedSize(nanogui::Vector2i(screenX, screenY));
    paddingX = 25;
    paddingY = screenY / paddingterm;
    titleW = screenX / phi;
    titleH = ((screenX / phi) / titleImageW) * titleImageH;
    titleImageOffsetX = paddingX;
    titleImageOffsetY = (paddingY * 4) + 25;

    setLayout(new nanogui::FlowLayout(nanogui::Orientation::Vertical, true, 0, paddingY));
    placeholder = new nanogui::Widget(this);
    placeholder->setWidth(titleW);
    placeholder->setHeight(titleH + (titleImageOffsetY * 2) + 25);

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
    titleW = screen()->width() / phi;
    titleH = ((screen()->width() / phi) / titleImageW) * titleImageH;
    placeholder->setFixedWidth(screen()->width());
    placeholder->setFixedHeight(titleH + 50);
    setLayout(new nanogui::FlowLayout(nanogui::Orientation::Vertical, true, 0, paddingY * 2));
    setNeedsLayout();
    titleImageOffsetX = 25;
    paddingY = screen()->height() / (phi * 80);
    titleImageOffsetY = (paddingY * 4) + 25;
    //titleImage = nvgImagePattern(ctx, titleImageOffsetX, titleImageOffsetY, titleW, titleH, 0, titleImageDataHandle, 1);
    DrawImage(ctx, titleImageDataHandle, .67, 0, 0, titleImageW, titleImageH, titleImageOffsetX, titleImageOffsetY, titleW, titleH);
    //nvgFillPaint(ctx, titleImage);
    //nvgRect(ctx, titleImageOffsetX, titleImageOffsetY, titleW, titleH);
    //nvgFill(ctx);

    Widget::draw(ctx);
}
