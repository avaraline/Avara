#pragma once

#include "CWindow.h"

class CNetworkWindow : public CWindow {
public:
    CNetworkWindow(CApplication *app);

    virtual ~CNetworkWindow();

    virtual void PrefChanged(std::string name);

protected:
    nanogui::TextBox *latencyBox;
};
