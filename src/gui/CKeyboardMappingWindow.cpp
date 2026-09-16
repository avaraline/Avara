//
//  CKeyboardMappingWindow.cpp
//  Avara
//
//  Created by Andy Halstead on 8/24/26.
//

#include "CKeyboardMappingWindow.h"
#include "Preferences.h"
#include "CApplication.h"
#include "CommandList.h"

CKeyboardMappingWindow::CKeyboardMappingWindow(CApplication *app, const std::string &actionDesc, const std::string &action, int imageOffset, int imageHandle) : CWindow(app, "Keyboard Mapping") {
    setTitle("Map Keys to Action");
    theAction = new std::string(action);
    auto layout = new nanogui::GridLayout(nanogui::Orientation::Horizontal, 2, nanogui::Alignment::Fill, 25, 25);
    layout->setRowAlignment(nanogui::Alignment::Middle);
    layout->setColAlignment(nanogui::Alignment::Middle);
    layout->setSpacing(0, 25);
    layout->setSpacing(1, 25);
    setLayout(layout);
    //auto size = nanogui::Vector2i(700, 700);
    //setFixedSize(size);
    actionIcon = add<SpriteWidget>();
    actionIcon->setImage(app->nvgContext(), imageHandle);
    actionIcon->setOffset(0, imageOffset);
    actionIcon->setDisplaySize(48);

    actionLabel = add<nanogui::Label>("Action");
    //actionLabel->setFixedWidth(300);
    actionLabel->setCaption(actionDesc);

    json allmap = app->Get(kKeyboardMappingTag);
    auto k = allmap.at(action);
    if (k.is_array()) {
        for (auto ik : k.items()) {
            std::string sdlkey = ik.value();
            add<nanogui::Label>(sdlkey);
            removeMappingButton(action, sdlkey);
        }
    }
    else {
        add<nanogui::Label>(k);
        removeMappingButton(action, k);
    }
    addbtn = add<nanogui::Button>("Add New");
    addbtn->setBackgroundColor(kGUIAccentPositive);
    addbtn->setCallback([this] {
        addbtn->setCaption("<waiting>");
        addbtn->setEnabled(false);
        gathering = true;
    });
    auto closeBtn = add<nanogui::Button>("Done");
    closeBtn->setCallback([this] {
        setModal(false);
        setVisible(false);
        gathering = false;
        if (mCallback) mCallback(0);
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
    keybtn->setBackgroundColor(kGUIAccentNegative);
    keybtn->setCallback([this, action, key] {
        SDL_Log("Delete %s bound to %s", key.c_str(), action.c_str());
        removeKeyBind(key);
        gathering = false;
        if (mCallback) mCallback(1);
    });
}

bool CKeyboardMappingWindow::editing() {
    return gathering;
}

bool CKeyboardMappingWindow::handleSDLEvent(SDL_Event &event) {
    if (gathering) {
        if (event.type == SDL_KEYDOWN) {
            auto sym = event.key.keysym.sym;
            auto name = SDL_GetKeyName(sym);
            SDL_Log("%d %s", sym, name);
            addKeyBind(name);
            gathering = false;
            if (mCallback) mCallback(1);
            return true;
        }
    }
    return false;
}


void CKeyboardMappingWindow::addKeyBind(const std::string &keyname) {
    json allmap = mApplication->Get(kKeyboardMappingTag);
    if (!(allmap.size()) || !theAction) return;
    auto theActionCStr = theAction->c_str();
    auto k = allmap.at(theActionCStr);
    if (k.is_array()) {
        bool found = false;
        for (auto ik : k.items()) {
            if (ik.value() == keyname) {
                found = true;
                SDL_Log("Not binding %s to %s because it is already bound", theActionCStr, keyname.c_str());
            }
        }
        if (!found) {
            k.push_back(json(keyname));
            allmap[theActionCStr] = k;
        }
    }
    else if (k != keyname) {
        json new_arr = nlohmann::json::array();
        new_arr = {k, json(keyname)};
        allmap[theActionCStr] = new_arr;
    }
    mApplication->Set(kKeyboardMappingTag, allmap);
}

void CKeyboardMappingWindow::removeKeyBind(const std::string &keyname) {
    json allmap = mApplication->Get(kKeyboardMappingTag);
    if (!(allmap.size()) || !theAction) return;
    auto theActionCStr = theAction->c_str();
    auto k = allmap.at(theActionCStr);
    auto remove_idxs = std::vector<int>();
    if (k.is_array()) {
        int ik_idx = 0;
        for (auto ik : k.items()) {
            if (ik.value() == keyname) {
                remove_idxs.push_back(ik_idx);
            }
            ik_idx++;
        }
        for (auto idx : remove_idxs) {
            k.erase(idx);
        }
        allmap[theActionCStr] = k;
    }
    else {
        allmap[theActionCStr] = json("");
    }
    mApplication->Set(kKeyboardMappingTag, allmap);
}
