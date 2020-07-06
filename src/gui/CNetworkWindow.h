#pragma once

#include "CWindow.h"

class CNetworkWindow : public CWindow {
public:
    CNetworkWindow(CApplication *app);

    virtual ~CNetworkWindow();

    virtual bool DoCommand(int theCommand) override;

    virtual void PrefChanged(std::string name) override;

    virtual bool editing() override;
    
protected:
	nanogui::TextBox *addressBox;
    nanogui::TextBox *latencyBox;
    nanogui::Button *connectBtn;
};
