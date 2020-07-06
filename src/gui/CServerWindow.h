#pragma once

#include "CWindow.h"

class CServerWindow : public CWindow {
public:
    CServerWindow(CApplication *app);

    virtual bool DoCommand(int theCommand) override;

    virtual ~CServerWindow();
    
    virtual bool editing() override;
    
protected:

    nanogui::TextBox *trackerBox;
    nanogui::TextBox *descriptionBox;
    nanogui::TextBox *passwordBox;
    nanogui::Button *startBtn;

};
