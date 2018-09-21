#include "CLevelWindow.h"

#include "CAvaraApp.h"
#include "CAvaraGame.h"
#include "CLevelDescriptor.h"
#include "CNetManager.h"
#include "Resource.h"

CLevelWindow::CLevelWindow(CApplication *app) : CWindow(app, "Levels") {
    // Hard-coded for now. Eventually use the level search API.
    levelSets = {
        "aa-normal", 
        "aa-abnormal", 
        "aa-deux-normal", 
        "aa-deux-abnormal", 
        "aa-tre",
        "a-bridge-too-far",
        "blockparty",  
        "classic-mix-up",
        "fosfori", 
        "hunting-grounds", 
        "net-99",
        "not-aa",
        "symbiosis",
        "the-codex",
        "the-lexicon",
        "wrestling"
    };

    setLayout(new nanogui::BoxLayout(nanogui::Orientation::Vertical, nanogui::Alignment::Fill, 10, 10));

    // TODO: check load permission: theNet->PermissionQuery(kAllowLoadBit, 0)

    setBox = new nanogui::ComboBox(this, levelSets);
    setBox->setCallback([this](int selectedIdx) { this->SelectSet(selectedIdx); });

    levelBox = new nanogui::DescComboBox(this, levelNames, levelIntros);
    levelBox->popup()->setSize(nanogui::Vector2i(500, 350));
    levelBox->setEnabled(false);

    loadBtn = new nanogui::Button(this, "Load Level");
    loadBtn->setCallback([this] { this->SendLoad(); });

    startBtn = new nanogui::Button(this, "Start Game");
    startBtn->setCallback([app] { ((CAvaraAppImpl *)app)->GetGame()->SendStartCommand(); });

    SelectSet(0);
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
