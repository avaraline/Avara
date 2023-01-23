#pragma once

#include "CWindow.h"

class CServerWindow : public CWindow {
public:
    CServerWindow(CApplication *app);

    virtual bool DoCommand(int theCommand) override;

    virtual void PrefChanged(std::string name) override;

    void EnableLatencyOptions(bool enable);

    virtual ~CServerWindow();

    virtual bool editing() override;

protected:

    nanogui::TextBox *trackerBox;
    nanogui::TextBox *descriptionBox;
    nanogui::TextBox *passwordBox;
    nanogui::Button *startBtn;
    nanogui::TextBox *latencyBox;
    nanogui::CheckBox *autoLatencyBox;
    nanogui::ComboBox *frameTimeBox;

};
