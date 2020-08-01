#include "CPlayerWindow.h"

#include "CApplication.h"
#include "Preferences.h"
#include "CAvaraApp.h"
#include "GitVersion.h"

CPlayerWindow::CPlayerWindow(CApplication *app) : CWindow(app, "Player") {
    setLayout(new nanogui::BoxLayout(nanogui::Orientation::Vertical, nanogui::Alignment::Fill, 10, 10));

    auto versionLabel = new nanogui::Text(this);
    versionLabel->setFont("mono");
    versionLabel->setFontSize(15);
    versionLabel->setValue(GIT_VERSION);

    std::string name = app->String(kPlayerNameTag);
    nameBox = new nanogui::TextBox(this);
    nameBox->setValue(name);
    nameBox->setEditable(true);
    nameBox->setCallback([app](std::string value) -> bool {
        app->Set(kPlayerNameTag, value);

        Str255 the_name;
        the_name[0] = value.length();
        BlockMoveData(value.c_str(), the_name + 1, value.length());

        ((CAvaraAppImpl *)gApplication)->GetNet()->NameChange(the_name);
        return true;
    });

    std::vector<std::string> hullTypes = {"Light", "Medium", "Heavy"};

    hullBox = new nanogui::ComboBox(this, hullTypes);
    hullBox->setCallback([app](int selectedIdx) { app->Set(kHullTypeTag, selectedIdx); });
    hullBox->popup()->setSize(nanogui::Vector2i(180, 140));
    hullBox->setSelectedIndex(app->Number(kHullTypeTag));
}

CPlayerWindow::~CPlayerWindow() {}

bool CPlayerWindow::editing() {
    return nameBox->focused();
}

bool CPlayerWindow::DoCommand(int theCommand) {
    return false;
}

void CPlayerWindow::PrefChanged(std::string name) {
    nameBox->setValue(mApplication->String(kPlayerNameTag));
    hullBox->setSelectedIndex(mApplication->Number(kHullTypeTag));
}
