#include "CPlayerWindow.h"

#include "CApplication.h"
#include "Preferences.h"
#include "CAvaraApp.h"
#include "GitVersion.h"
#include "LevelLoader.h"
#include "Parser.h"

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

    std::vector<std::string> hullTypes = {10,""};
    hullValues = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

    hullBox = new nanogui::ComboBox(this, hullTypes);
    hullBox->setCallback([app, this](int selectedIdx) {
        app->Set(kHullTypeTag, hullValues[selectedIdx]);
    });
    hullBox->popup()->setSize(nanogui::Vector2i(180, 140));
    hullBox->setSelectedIndex(app->Get<int>(kHullTypeTag));
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
    hullBox->setSelectedIndex(mApplication->Get<int>(kHullTypeTag));
}

void CPlayerWindow::RepopulateHullOptions() {
    std::vector<std::string> names {};
    auto current = hullBox->selectedIndex();
    bool currentIsValid = false;

    hullValues.clear();
    for (auto i = 0; i < 10; i++) {
        std::string name = ReadStringVar(iHullName01 + i);
        long resId = ReadLongVar(iHull01 + i);
        if (resId != 0) {
            names.push_back(name);
            hullValues.push_back(i);
            if (current == i) {
                currentIsValid = true;
            }
        }
    }
    hullBox->setItems(names);
    hullBox->setSelectedIndex(currentIsValid ? current : 0);
    hullBox->callback()(currentIsValid ? current : 0);
    hullBox->setNeedsLayout();
}
