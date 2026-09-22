#pragma once

#include "CWindow.h"

#include <nanogui/desccombobox.h>
#include <string>
#include <vector>

#include "LevelInfo.h"

class CLevelWindow : public CWindow {
public:
    CLevelWindow(CApplication *app);

    virtual ~CLevelWindow();

    // Handles a command broadcasted by CApplication::BroadcastCommand. Returns true if it was actually handled.
    virtual bool DoCommand(int theCommand);

    void SelectSet(int selected);
    void SelectSet(std::string set);
    void SelectLevel(std::string set, std::string levelName);
    void FetchRecents();
    void UpdateRecents();
    void SetIntro(int selected);
    void SendLoad();

    nanogui::DescComboBox *recentsBox;
    nanogui::ComboBox *setBox;
    nanogui::DescComboBox *levelBox;
    nanogui::Label *levelIntro;
    nanogui::Button *loadBtn;
    nanogui::Button *startBtn;

    std::vector<std::string> recentSets;
    std::vector<std::string> recentLevels;
    std::vector<std::string> levelSets;
    std::vector<std::string> levelNames;
    std::vector<std::string> levelIntros;
    std::vector<std::string> levelTags;
    std::vector<std::string> levelTagStrings;
};
