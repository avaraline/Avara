#include "CLevelWindow.h"

#include "AssetManager.h"
#include "CAvaraApp.h"
#include "CAvaraGame.h"
#include "CNetManager.h"
#include "Preferences.h"
#include "CRUD.h"

static int MAX_RECENTS = 50;

CLevelWindow::CLevelWindow(CApplication *app) : CWindow(app, "Levels") {
    // Searches "levels/" directory alongside application.
    // will eventually use level search API

    levelSets = AssetManager::GetAvailablePackages();

    FetchRecents();

    setLayout(new nanogui::BoxLayout(nanogui::Orientation::Vertical, nanogui::Alignment::Fill, 10, 10));

    // TODO: check load permission: theNet->PermissionQuery(kAllowLoadBit, 0)

    recentsBox = new nanogui::DescComboBox(this, recentLevels, recentSets);
    recentsBox->setCaption("Recents");
    recentsBox->setCallback([this](int selectedIdx) {
        this->SelectLevel(recentSets[selectedIdx], recentLevels[selectedIdx]);
        recentsBox->setCaption("Recents");
    });

    setBox = new nanogui::ComboBox(this, levelSets);
    setBox->setCallback([this](int selectedIdx) {
        this->SelectSet(selectedIdx);
        levelBox->setSelectedIndex(0);
    });
    setBox->popup()->setSize(nanogui::Vector2i(300, 600));

    levelBox = new nanogui::DescComboBox(this, levelNames, levelIntros);
    levelBox->popup()->setSize(nanogui::Vector2i(500, 350));
    levelBox->setEnabled(false);

    loadBtn = new nanogui::Button(this, "Load Level");
    loadBtn->setCallback([this] { this->SendLoad(); });

    startBtn = new nanogui::Button(this, "Start/Ready");
    startBtn->setCallback([app] {
        if (SDL_GetModState() & KMOD_ALT) {
            // if ALT key pressed, start right away
            ((CAvaraAppImpl *)app)->GetGame()->SendStartCommand();
        } else {
            // send the "ready" checkmark √
            ((CAvaraAppImpl *)app)->GetNet()->SendRosterMessage(checkMark_utf8);
        }
    });
    SelectSet(0);
    levelBox->setSelectedIndex(0);
}

CLevelWindow::~CLevelWindow() {}

bool CLevelWindow::DoCommand(int theCommand) {
    return false;
}

void CLevelWindow::FetchRecents() {
    recentSets.clear();
    recentLevels.clear();
    CRUD::RecentLevelsList recents = static_cast<CAvaraAppImpl *>(mApplication)->itsAPI->GetRecentLevels(MAX_RECENTS);
    for (auto recent : recents) {
        recentSets.push_back(recent.setTag);
        recentLevels.push_back(recent.levelName);
    }
}

void CLevelWindow::UpdateRecents() {
    FetchRecents();
    recentsBox->setItems(recentLevels, recentSets);
    recentsBox->setCaption("Recents");
    recentsBox->setNeedsLayout();
}

void CLevelWindow::SelectLevel(std::string set, std::string levelName) {
    SelectSet(set);

    int levelIndex = 0;
    auto levelIt = std::find(levelNames.begin(), levelNames.end(), levelName);
    if (levelIt != levelNames.end()) {
        levelIndex = static_cast<int>(std::distance(levelNames.begin(), levelIt));
    }

    levelBox->setSelectedIndex(levelIndex);
}

void CLevelWindow::SelectSet(int selected) {
    SelectSet(levelSets[selected]);
}

void CLevelWindow::SelectSet(std::string set) {
    if (set.length() < 1)
        return;
    std::vector<std::string>::iterator itr = std::find(levelSets.begin(), levelSets.end(), set);
    int level_idx = 0;
    if (itr != levelSets.end()) {
        level_idx = static_cast<int>(std::distance(levelSets.begin(), itr));
        setBox->setSelectedIndex(level_idx);
    }
    levelNames.clear();
    levelIntros.clear();
    levelTags.clear();

    auto manifest = *AssetManager::GetManifest(set);
    for (auto const &ledi : manifest->levelDirectory) {
        levelNames.push_back(ledi.levelName);
        levelIntros.push_back(ledi.levelInfo);
        levelTags.push_back(ledi.alfPath);
    }
    levelBox->setItems(levelNames, levelIntros);
    levelBox->setEnabled(true);
    levelBox->setNeedsLayout();
}

void CLevelWindow::SendLoad() {
    std::string set = levelSets[setBox->selectedIndex()];
    std::string title = levelNames[levelBox->selectedIndex()];
    std::string tag = levelTags[levelBox->selectedIndex()];
    ((CAvaraAppImpl *)gApplication)->GetNet()->SendLoadLevel(set, tag);
}
