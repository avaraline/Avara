#include "CPlayerWindow.h"

#include "CApplication.h"
#include "Preferences.h"

CPlayerWindow::CPlayerWindow(CApplication *app) : CWindow(app, "Player") {
    setLayout(new nanogui::BoxLayout(nanogui::Orientation::Vertical, nanogui::Alignment::Fill, 20, 10));

    std::string name = app->String(kPlayerNameTag);
    nanogui::TextBox *nameBox = new nanogui::TextBox(this);
    nameBox->setValue(name);
    nameBox->setEditable(true);
    nameBox->setCallback([app](std::string value) -> bool {
        app->Set(kPlayerNameTag, value);
        return true;
    });

    std::vector<std::string> hullTypes = {"Light", "Medium", "Heavy"};

    nanogui::ComboBox *hullBox = new nanogui::ComboBox(this, hullTypes);
    hullBox->setCallback([app](int selectedIdx) { app->Set(kHullTypeTag, selectedIdx); });
    hullBox->popup()->setSize(nanogui::Vector2i(180, 140));
    hullBox->setSelectedIndex(app->Number(kHullTypeTag));
}

CPlayerWindow::~CPlayerWindow() {}

bool CPlayerWindow::DoCommand(int theCommand) {
    return false;
}
