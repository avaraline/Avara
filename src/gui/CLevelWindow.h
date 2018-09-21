#pragma once

#include "CWindow.h"

#include <string>
#include <vector>
#include <nanogui/desccombobox.h>

class CLevelWindow : public CWindow {
public:
    CLevelWindow(CApplication *app);

    virtual ~CLevelWindow();

    // Handles a command broadcasted by CApplication::BroadcastCommand. Returns true if it was actually handled.
    virtual bool DoCommand(int theCommand);

    void SelectSet(int selected);
    void SetIntro(int selected);
    void SendLoad();

    nanogui::ComboBox *setBox;
    nanogui::DescComboBox *levelBox;
    nanogui::Label *levelIntro;
    nanogui::Button *loadBtn;
    nanogui::Button *startBtn;

    std::vector<std::string> levelSets;
    std::vector<std::string> levelNames;
    std::vector<std::string> levelIntros;
    std::vector<OSType> levelTags;
};
