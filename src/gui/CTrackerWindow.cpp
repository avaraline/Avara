#include "CTrackerWindow.h"

#include "CApplication.h"
#include "Preferences.h"

CTrackerWindow::CTrackerWindow(CApplication *app) : CWindow(app, "Tracker") {
    setLayout(new nanogui::BoxLayout(nanogui::Orientation::Vertical, nanogui::Alignment::Fill, 20, 10));

    nanogui::Widget *top = new nanogui::Widget(this);
    top->setLayout(new nanogui::FlowLayout(nanogui::Orientation::Horizontal, true, 0, 10));

    nanogui::TextBox *addressBox = new nanogui::TextBox(top);
    addressBox->setValue(app->String(kTrackerAddress));
    addressBox->setEditable(true);

    nanogui::Button *refreshBtn = new nanogui::Button(top, "Refresh");
    refreshBtn->setCallback([] {
    });

    nanogui::VScrollPanel *results = new nanogui::VScrollPanel(this);
    results->setFixedSize(nanogui::Vector2i(200, 100));
}

CTrackerWindow::~CTrackerWindow() {}

bool CTrackerWindow::DoCommand(int theCommand) {
    return false;
}
