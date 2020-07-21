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
    //$ ls -w1 levels/*.r | sed -E 's/levels.(.+)\.r/"\1",/g'
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
        "clockwork-blue-b4",
        "cnemies-squares",
        "crescent",
        "dildensburg",
        "disk-o-tech-light",
        "dodgeball",
        "emotion",
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
        "medievos",
        "net-99",
        "net-levels",
        "new-moon",
        "not-aa",
        "oddities-v4",
        "pastabravo",
        "scarlet-pimpernel-beta-0919",
        "single-player",
        "someset",
        "strawberry",
        "symbiosis",
        "the-codex",
        "the-lexicon",
        "t-plus-5-part-a",
        "t-plus-5-part-b",
        "t-plus-5-part-c",
        "we-be-ground-pounders",
        "wide-open",
        "wild-west-collection",
        "wrestling",
        "wut"
    };

    json sets = app->Get(kRecentSets);
    for (json::iterator it = sets.begin(); it != sets.end(); ++it) {
        recentSets.push_back(it.value());
    }
    json levels = app->Get(kRecentLevels);
    for (json::iterator itLev = levels.begin(); itLev != levels.end(); ++itLev) {
        recentLevels.push_back(itLev.value());
    }
    
    setLayout(new nanogui::BoxLayout(nanogui::Orientation::Vertical, nanogui::Alignment::Fill, 10, 10));

    // TODO: check load permission: theNet->PermissionQuery(kAllowLoadBit, 0)

    recentsBox = new nanogui::DescComboBox(this, recentLevels, recentSets);
    recentsBox->setCaption("Recents");
    recentsBox->setCallback([this](int selectedIdx) {
        this->SelectLevel(recentSets[selectedIdx], recentLevels[selectedIdx]);
        recentsBox->setCaption("Recents");
    });

    setBox = new nanogui::ComboBox(this, levelSets);
    setBox->setCallback([this, app](int selectedIdx) {
        this->SelectSet(selectedIdx);
        levelBox->setSelectedIndex(0);
    });

    levelBox = new nanogui::DescComboBox(this, levelNames, levelIntros);
    levelBox->popup()->setSize(nanogui::Vector2i(500, 350));
    levelBox->setEnabled(false);

    loadBtn = new nanogui::Button(this, "Load Level");
    loadBtn->setCallback([this] { this->SendLoad(); });

    startBtn = new nanogui::Button(this, "Start Game");
    startBtn->setCallback([app] { ((CAvaraAppImpl *)app)->GetGame()->SendStartCommand(); });
    
    if(recentSets.size() > 0) {
        SelectLevel(recentSets[0], recentLevels[0]);
    }
    else {
        SelectSet(0);
        levelBox->setSelectedIndex(0);
    }
}

CLevelWindow::~CLevelWindow() {}

bool CLevelWindow::DoCommand(int theCommand) {
    return false;
}

void CLevelWindow::AddRecent(std::string set, std::string levelName) {
    if(json::accept("['" + set + "',  '" + levelName + "']")) {
        //remove level if it is already in recents
        for(unsigned i = 0; i < recentSets.size(); i++) {
            if(recentSets[i].compare(set) == 0 && recentLevels[i].compare(levelName) == 0) {
                recentSets.erase(recentSets.begin() + i);
                recentLevels.erase(recentLevels.begin() + i);
                break;
            }
        }
        
        recentSets.insert(recentSets.begin(), set);
        recentLevels.insert(recentLevels.begin(), levelName);
        
        if(recentSets.size() > 10) {
            recentSets.pop_back();
            recentLevels.pop_back();
        }
        
        recentsBox->setItems(recentLevels, recentSets);
        recentsBox->setCaption("Recents");
        recentsBox->setNeedsLayout();
        
        mApplication->Set(kRecentSets, recentSets);
        mApplication->Set(kRecentLevels, recentLevels);
    }
    else {
        SDL_Log("AddRecent ignoring bad set/level name.");
    }
}

void CLevelWindow::SelectLevel(std::string set, std::string levelName) {
    SelectSet(set);

    int levelIndex = 0;
    auto levelIt = std::find(levelNames.begin(), levelNames.end(), levelName);
    if (levelIt != levelNames.end()) {
        levelIndex = std::distance(levelNames.begin(), levelIt);
    }
    
    levelBox->setSelectedIndex(levelIndex);
}

void CLevelWindow::SelectSet(int selected) {
    SelectSet(levelSets[selected]);
}

void CLevelWindow::SelectSet(std::string set) {
    std::vector<std::string>::iterator itr = std::find(levelSets.begin(), levelSets.end(), set);
    if (itr != levelSets.end()) {
        setBox->setSelectedIndex(std::distance(levelSets.begin(), itr));
    }
    
    std::string rsrcPath = std::string("levels/") + set + ".r";
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
