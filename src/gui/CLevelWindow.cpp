#include "CLevelWindow.h"

#include "CAvaraApp.h"
#include "CAvaraGame.h"
#include "CLevelDescriptor.h"
#include "CNetManager.h"
#include "Resource.h"
#include "Preferences.h"

CLevelWindow::CLevelWindow(CApplication *app) : CWindow(app, "Levels") {
    // Hard-coded for now. Eventually use the level search API.
    levelSets = {
        "aa-abnormal",
        "aa-deux-abnormal",
        "aa-deux-normal",
        "aa-normal",
        "aa-tre",
        "a-bridge-too-far",
        "blockparty",
        "butternut-squash",
        "cancer",
        "classic-mix-up",
        "cnemies-squares",
        "crescent",
        "disk-o-tech-light",
        "dodgeball",
        "fosfori",
        "gzr-balledness",
        "gzr-geriatric-ward",
        "gzr-grecian-formula",
        "gzr-liver-spots",
        "gzr-over-the-hill",
        "gzr-rip",
        "holy-hand-grenades",
        "hunting-grounds",
        "iya",
        "klaus-levels",
        "macabre",
        "net-99",
        "new-moon",
        "not-aa",
        "pastabravo",
        "scarlet-pimpernel-beta-0919",
        "single-player",
        "strawberry",
        "symbiosis",
        "the-codex",
        "the-lexicon",
        "wild-west-collection",
        "wrestling",
        "wut"
    };

    setLayout(new nanogui::BoxLayout(nanogui::Orientation::Vertical, nanogui::Alignment::Fill, 10, 10));

    // TODO: check load permission: theNet->PermissionQuery(kAllowLoadBit, 0)

    setBox = new nanogui::ComboBox(this, levelSets);
    setBox->setCallback([this, app](int selectedIdx) {
        this->SelectSet(selectedIdx);
        app->Set(kSelectedSet, levelSets[selectedIdx]);
        levelBox->setSelectedIndex(0);
        app->Set(kSelectedLevel, levelNames[0]);
    });

    levelBox = new nanogui::DescComboBox(this, levelNames, levelIntros);
    levelBox->setCallback([this, app](int selectedIdx) {
        app->Set(kSelectedLevel, levelNames[selectedIdx]);
    });
    levelBox->popup()->setSize(nanogui::Vector2i(500, 350));
    levelBox->setEnabled(false);

    loadBtn = new nanogui::Button(this, "Load Level");
    loadBtn->setCallback([this] { this->SendLoad(); });

    startBtn = new nanogui::Button(this, "Start Game");
    startBtn->setCallback([app] { ((CAvaraAppImpl *)app)->GetGame()->SendStartCommand(); });

    //select last used level set
    int setIndex = 0;
    auto it = std::find(levelSets.begin(), levelSets.end(), app->String(kSelectedSet));
    if (it != levelSets.end()) {
        setIndex = std::distance(levelSets.begin(), it);
        setBox->setSelectedIndex(setIndex);
    }

    SelectSet(setIndex);

    //select last used level
    int levelIndex = 0;
    auto levelIt = std::find(levelNames.begin(), levelNames.end(), app->String(kSelectedLevel));
    if (levelIt != levelNames.end()) {
        levelIndex = std::distance(levelNames.begin(), levelIt);
    }

    levelBox->setSelectedIndex(levelIndex);
}

CLevelWindow::~CLevelWindow() {}

bool CLevelWindow::DoCommand(int theCommand) {
    return false;
}

void CLevelWindow::SelectSet(int selected) {
    std::string rsrcPath = std::string("levels/") + levelSets[selected] + ".r";
    OSType setTag;
    UseResFile(rsrcPath);
    CLevelDescriptor *levels = LoadLevelListFromResource(&setTag);
    CLevelDescriptor *curLevel = levels;
    levelNames.clear();
    levelIntros.clear();
    levelTags.clear();
    while (curLevel) {
        std::string name((char *)curLevel->name + 1, curLevel->name[0]);
        std::string intro((char *)curLevel->intro + 1, curLevel->intro[0]);
        intro.erase(0, intro.find_first_not_of(" \r\n"));
        // std::string access((char *)curLevel->access + 1, curLevel->access[0]);
        levelNames.push_back(name);
        levelIntros.push_back(intro);
        levelTags.push_back(curLevel->tag);
        curLevel = curLevel->nextLevel;
    }
    levels->Dispose();
    levelBox->setItems(levelNames, levelIntros);
    levelBox->setEnabled(true);
    levelBox->setNeedsLayout();
}

void CLevelWindow::SendLoad() {
    std::string set = levelSets[setBox->selectedIndex()];
    OSType tag = levelTags[levelBox->selectedIndex()];
    ((CAvaraAppImpl *)gApplication)->GetNet()->SendLoadLevel(set, tag);
}
