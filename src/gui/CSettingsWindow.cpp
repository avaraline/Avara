//
//  CSettingsWindow.cpp
//  Avara
//
//  Created by Andy Halstead on 8/18/26.
//

#include "CSettingsWindow.h"
#include "CApplication.h"
#include "Preferences.h"
#include "ColorManager.h"
#include "AssetManager.h"
#include "ARGBColor.h"
#include "NVGUtil.h"

CSettingsWindow::CSettingsWindow(CApplication *app) : CWindow(app, "Avara Settings") {
    setLayout(new nanogui::BoxLayout(nanogui::Orientation::Vertical, nanogui::Alignment::Fill, 10, 10));
    mPrefTabs = this->add<nanogui::TabWidget>();

    NVGcontext *ctx = app->nvgContext();

    std::string kbIconsPath = AssetManager::GetImagePath(NoPackage, "control48px.png");
    keyboardIconsDataHandle = nvgCreateImage(ctx, kbIconsPath.c_str(), 0);

    this->setFixedWidth(700);
    this->setFixedHeight(600);
    for (auto &pane : optionsScreens.items()) {
        std::string title(pane.key());
        Widget* layer = mPrefTabs->createTab(title);
        layer->setLayout(new nanogui::GroupLayout());
        nanogui::VScrollPanel *panel = new nanogui::VScrollPanel(layer);
        panel->setFixedHeight(450);
        panel->setLayout(new nanogui::GroupLayout());
        nanogui::Widget *panelContent = new nanogui::Widget(panel);
        panelContent->setLayout(new nanogui::GroupLayout());
        for (auto &opt : pane.value().items()) {
            std::string optLabelStr(opt.value()[0]);
            std::string optKey(opt.value()[1]);
            optionTypes optType = opt.value()[2];
            panelContent->add<nanogui::Label>(optLabelStr);
            switch(optType) {
                case kOptionTypeBool: {
                    bool value = mApplication->Get(optKey);
                    auto cb = panelContent->add<nanogui::CheckBox>();
                    cb->setChecked(value);
                    cb->setCaption(value ? "Enabled" : "Disabled");
                    cb->setCallback([this, optKey, cb] (bool checked) {
                        mApplication->Set(optKey, checked);
                        cb->setCaption(checked ? "Enabled" : "Disabled");
                    });
                    break;
                }
                case kOptionTypeString: {
                    auto tb = panelContent->add<nanogui::TextBox>();
                    tb->setValue(mApplication->String(optKey));
                    tb->setCallback([this, optKey](const std::string &input) -> bool {
                        mApplication->Set(optKey, input);
                        return true;
                    });
                    break;
                }
                case kOptionTypeFloat: {
                    auto tb = panelContent->add<nanogui::TextBox>();
                    std::string valStr = std::to_string(mApplication->Get<float>(optKey));
                    tb->setValue(valStr);
                    tb->setAlignment(nanogui::TextBox::Alignment::Left);
                    tb->setFormat("[-]?[0-9]*\\.?[0-9]+");
                    tb->setCallback([this, optKey](const std::string &input) -> bool {
                        mApplication->Set(optKey, std::stof(input));
                        return true;
                    });
                    break;
                }
                case kOptionTypeInteger: {
                    auto tb = panelContent->add<nanogui::TextBox>();
                    std::string valStr = std::to_string(mApplication->Get<long>(optKey));
                    tb->setValue(valStr);
                    tb->setAlignment(nanogui::TextBox::Alignment::Left);
                    tb->setFormat("[1-9][0-9]*");
                    tb->setCallback([this, optKey](const std::string &input) -> bool {
                        mApplication->Set(optKey, std::stoi(input));
                        return true;
                    });
                    break;
                }
                case kOptionTypeChoice: {
                    std::vector<std::string> labels = std::vector<std::string>();
                    for (json &choice : opt.value()[3]) {
                        labels.push_back(choice[1]);
                    }
                    auto cb = panelContent->add<nanogui::ComboBox>();
                    cb->setItems(labels);
                    cb->setSelectedIndex((int)mApplication->Get<long>(optKey));
                    cb->setTextPosition(nanogui::ComboBox::TextPosition::Left);
                    cb->setCallback([this, optKey] (int input) {
                        mApplication->Set<long>(optKey, input);
                    });
                    break;
                }
                case kOptionTypeKeyboard: {
                    int keyboardConfigIndex = 0;
                    int keyboardIconSize = 25;
                    json theKeys = app->Get(kKeyboardMappingTag);
                    auto container = panelContent->add<nanogui::Widget>();
                    auto layout = new nanogui::GridLayout(nanogui::Orientation::Horizontal, 3);
                    layout->setSpacing(1, 15);
                    std::vector<nanogui::Alignment> aligns;
                    aligns.push_back(nanogui::Alignment::Minimum);
                    aligns.push_back(nanogui::Alignment::Minimum);
                    aligns.push_back(nanogui::Alignment::Fill);
                    layout->setColAlignment(aligns);
                    container->setLayout(layout);
                    for (auto &action : keyboardConfig) {
                        int keyboardIconOffset = 48 * keyboardConfigIndex;
                        std::string actionDesc = action.first;
                        std::string actionKey = action.second;
                        if (!actionKey.length()) continue;
                        if (keyboardConfigIndex < keyboardConfigIconCount) {
                            auto kbIcon = container->add<SpriteWidget>();
                            kbIcon->setImage(ctx, keyboardIconsDataHandle);
                            kbIcon->setSpriteSize(48);
                            kbIcon->setDisplaySize(keyboardIconSize);
                            kbIcon->setOffset(0, keyboardIconOffset);
                        }
                        else {
                            container->add<Widget>();
                        }
                        container->add<nanogui::Label>(actionDesc);
                        auto k = theKeys.at(actionKey);
                        auto current_keys_str = std::stringstream();
                        if (k.is_array()) {
                            auto separator = "";
                            for (auto ik : k.items()) {
                                std::string sdlkey = ik.value();
                                current_keys_str << separator << sdlkey;
                                separator = " · ";
                            }
                        }
                        else {
                            std::string sdlkey = k;
                            current_keys_str << sdlkey;
                        }
                        auto longbutton = container->add<nanogui::PopupButton>();
                        longbutton->setCaption(current_keys_str.str());
                        auto p = longbutton->popup();
                        auto layout = new nanogui::GridLayout(nanogui::Orientation::Horizontal, 2);
                        auto size = nanogui::Vector2i(300, 500);
                        p->setLayout(layout);
                        p->setFixedSize(size);
                        //center();
                        auto actionIcon = p->add<SpriteWidget>();
                        actionIcon->setImage(ctx, keyboardIconSize);
                        actionIcon->setSpriteSize(48);
                        actionIcon->setDisplaySize(keyboardIconSize);
                        actionIcon->setOffset(0, keyboardIconOffset);
                        auto actionLabel = p->add<nanogui::Label>("Action");
                        auto currentLabel = p->add<nanogui::Label>("Currently mapped");
                        auto currentlyMappedKeys = p->add<nanogui::Widget>();
                        // todo figure this crap out
                        actionIcon->setOffset(0, keyboardIconOffset);
                        actionLabel->setCaption(actionKey);
                        json allmap = mApplication->Get(kKeyboardMappingTag);
                        json singlemap = allmap.at(actionKey);
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
                        longbutton->setTextPosition(nanogui::Button::TextPosition::Left);
                        if (keyboardConfigIndex % 2 == 0) {
                            longbutton->setBackgroundColor(nanogui::Color(255, 255, 255, 35));
                        }
                        longbutton->setCallback([this, actionKey, keyboardIconOffset] {
                            keyMapWindow->startMapping(actionKey, keyboardIconOffset);
                        });
                        keyboardConfigIndex++;
                    }
                    break;
                }
                case kOptionTypeColor: {
                    auto color = panelContent->add<nanogui::Button>();
                    color->setCaption("");
                    color->setFixedSize(nanogui::Vector2i(35, 35));
                    auto tb = panelContent->add<nanogui::TextBox>();
                    std::string valStr = mApplication->Get<std::string>(optKey);
                    tb->setValue(valStr);
                    tb->setAlignment(nanogui::TextBox::Alignment::Left);
                    tb->setFormat("#([a-f]|[A-F]|[0-9]){3}(([a-f]|[A-F]|[0-9]){3})?");
                    tb->setCallback([this, color, optKey] (const std::string newVal) {
                        mApplication->Set<std::string>(optKey, newVal);
                        color->setBackgroundColor(ToNanoguiColor(ARGBColor().Parse(newVal).value_or(ColorManager::getLookForwardColor())));
                        return true;
                    });
                    color->setBackgroundColor(ToNanoguiColor(ARGBColor().Parse(valStr).value_or(ColorManager::getLookForwardColor())));
                    break;
                }
            }
        }
        panelContent->setNeedsLayout();
    }
    /*
    auto exitsettings = this->add<nanogui::Button>("Done");
    exitsettings->setCallback([this] () {
        this->setVisible(false);
    });
    */
}

void CSettingsWindow::setKeyMapWindow(CKeyboardMappingWindow *win) {
    keyMapWindow = win;
    keyMapWindow->setImage(keyboardIconsDataHandle);
}

CSettingsWindow::~CSettingsWindow() {

}
