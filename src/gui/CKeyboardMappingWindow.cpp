//
//  CKeyboardMappingWindow.cpp
//  Avara
//
//  Created by Andy Halstead on 8/24/26.
//

#include "CKeyboardMappingWindow.h"
#include "Preferences.h"
#include "CApplication.h"

CKeyboardMappingWindow::CKeyboardMappingWindow(CApplication *app) : CWindow(app, "Keyboard Mapping") {
    setTitle("Map Keys to Action");
    auto layout = new nanogui::GridLayout(nanogui::Orientation::Horizontal, 2);
    auto size = nanogui::Vector2i(300, 500);
    setLayout(layout);
    setFixedSize(size);
    //center();
    actionIcon = add<SpriteWidget>();
    actionLabel = add<nanogui::Label>("Action");
    currentLabel = add<nanogui::Label>("Currently mapped");
    currentlyMappedKeys = add<nanogui::Widget>();
}

void CKeyboardMappingWindow::setImage(int imageHandle) {
    actionIcon->setImage(mApplication->nvgContext(), imageHandle);
}

void CKeyboardMappingWindow::startMapping(const std::string &key, int imageOffset) {
    actionIcon->setOffset(0, imageOffset);
    actionLabel->setCaption(key);
    currentlyMapping = new std::string(key);
    //for (auto w : currentlyMappedKeys->children()) {
    //    currentlyMappedKeys->removeChild(w);
    //}
    json allmap = mApplication->Get(kKeyboardMappingTag);
    json singlemap = allmap.at(key);
    if (singlemap.is_array()) {
        for (auto ik : singlemap.items()) {
            std::string sdlkey = ik.value();
            auto keybtn = currentlyMappedKeys->add<nanogui::Button>();
            keybtn->setCaption(sdlkey);
        }
    }
    else {
        std::string sdlkey = singlemap;
        auto keybtn = currentlyMappedKeys->add<nanogui::Button>();
        keybtn->setCaption(sdlkey);
    }
    setVisible(true);
    setModal(true);
}

void CKeyboardMappingWindow::gatherKey() {
    gathering = true;
}

bool CKeyboardMappingWindow::handleSDLEvent(SDL_Event &event) {
    if (gathering) {
        if (event.type == SDL_KEYDOWN) {
            auto sym = event.key.keysym;
            auto name = SDL_GetKeyName(sym.scancode);
            // todo finish
        }
        gathering = false;
    }
    return false;
}

CKeyboardMappingWindow::~CKeyboardMappingWindow() {

}
