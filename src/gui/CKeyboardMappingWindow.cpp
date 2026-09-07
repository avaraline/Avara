//
//  CKeyboardMappingWindow.cpp
//  Avara
//
//  Created by Andy Halstead on 8/24/26.
//

#include "CKeyboardMappingWindow.h"
#include "Preferences.h"
#include "CApplication.h"

CKeyboardMappingWindow::CKeyboardMappingWindow(CApplication *app, const std::string &actionDesc, const std::string &key, int imageOffset, int imageHandle) : CWindow(app, "Keyboard Mapping") {
    setTitle("Map Keys to Action");
    auto layout = new nanogui::GridLayout(nanogui::Orientation::Horizontal, 2, nanogui::Alignment::Maximum);
    setLayout(layout);
    //auto size = nanogui::Vector2i(700, 700);
    //setFixedSize(size);
    actionIcon = add<SpriteWidget>();
    actionIcon->setImage(app->nvgContext(), imageHandle);
    actionIcon->setOffset(0, imageOffset);
    actionIcon->setDisplaySize(48);

    actionLabel = add<nanogui::Label>("Action");
    actionLabel->setFixedWidth(300);
    actionLabel->setCaption(actionDesc);

    currentlyMapping = new std::string(key);

    auto addColor = nanogui::Color(22, 80, 22, 255);

    json allmap = app->Get(kKeyboardMappingTag);
    auto k = allmap.at(key);
    if (k.is_array()) {
        for (auto ik : k.items()) {
            std::string sdlkey = ik.value();
            add<nanogui::Label>(sdlkey);
            removeMappingButton(key, sdlkey);
        }
    }
    else {
        add<nanogui::Label>(k);
        removeMappingButton(key, k);
    }
    addbtn = add<nanogui::Button>("Add New");
    addbtn->setBackgroundColor(addColor);
    addbtn->setCallback([this] {
        actionLabel->setCaption("Press a key...");
        addbtn->setVisible(false);
        this->gathering = true;
        //this->gatherKey();
    });
    auto closeBtn = add<nanogui::Button>("Done");
    closeBtn->setCallback([this] {
        setModal(false);
        setVisible(false);
        this->dispose();
    });
    setNeedsLayout();
    setVisible(true);
    setModal(true);
    center();
    requestFocus();
}

void CKeyboardMappingWindow::removeMappingButton(const std::string &action, const std::string &key) {
    auto keybtn = add<nanogui::Button>();
    keybtn->setFont("icon");
    keybtn->setCaption("");
    keybtn->setIcon(ENTYPO_ICON_TRASH);
    keybtn->setBackgroundColor(nanogui::Color(80, 22, 22, 255));
    keybtn->setCallback([this, action, key] {
        //mApplication->DoCommand(kUnmapKeyCommand, ...);
        actionLabel->setCaption("Delete " + action + " " + key);
    });
}

void CKeyboardMappingWindow::gatherKey() {
    gathering = true;
}

bool CKeyboardMappingWindow::editing() {
    return true;
    if (gathering) {
        return true;
    }
    else {
        return false;
    }
}

bool CKeyboardMappingWindow::handleSDLEvent(SDL_Event &event) {
    if (gathering) {
        if (event.type == SDL_KEYDOWN) {
            auto sym = event.key.keysym;
            auto name = SDL_GetKeyName(sym.scancode);
            // todo finish
            actionLabel->setCaption(name);
        }
        gathering = false;
    }
    return false;
}

