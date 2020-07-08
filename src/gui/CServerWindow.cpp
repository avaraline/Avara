#include "CServerWindow.h"

#include "CAvaraApp.h"
#include "CNetManager.h"
#include "Preferences.h"
#include "CommandList.h"

CServerWindow::CServerWindow(CApplication *app) : CWindow(app, "Server") {
    setLayout(new nanogui::BoxLayout(nanogui::Orientation::Vertical, nanogui::Alignment::Fill, 10, 10));
   
    std::string description = app->String(kServerDescription);
    descriptionBox = new nanogui::TextBox(this);
    descriptionBox->setPlaceholder("Server Message");
    descriptionBox->setValue(description);
    descriptionBox->setEditable(true);
    descriptionBox->setCallback([app](std::string value) -> bool {
        app->Set(kServerDescription, value);
        return true;
    });

    std::string password = app->String(kServerPassword);
    passwordBox = new nanogui::TextBox(this);
    passwordBox->setPlaceholder("Password");
    passwordBox->setValue(password);
    passwordBox->setEditable(true);
    passwordBox->setCallback([app](std::string value) -> bool {
        app->Set(kServerPassword, value);
        return true;
    });
    
    startBtn = new nanogui::Button(this, "Start Hosting");
    startBtn->setCallback([app] {
        CAvaraAppImpl *avara = (CAvaraAppImpl *)app;
        if(avara->GetNet()->netStatus == kServerNet)
            avara->GetNet()->ChangeNet(kNullNet, "");
        else
            avara->GetNet()->ChangeNet(kServerNet, "");
    });
    
    nanogui::CheckBox *registerBox = new nanogui::CheckBox(this, "Register With Tracker:", [this, app](bool checked) {
        this->trackerBox->setEditable(checked);
        this->trackerBox->setEnabled(checked);
        app->Set(kTrackerRegister, checked);
    });
    bool shouldRegister = app->Number(kTrackerRegister) != 0;
    registerBox->setChecked(shouldRegister);
    
    trackerBox = new nanogui::TextBox(this);
    trackerBox->setValue(app->String(kTrackerRegisterAddress));
    trackerBox->setEditable(true);
    trackerBox->setCallback([app](std::string value) -> bool {
        app->Set(kTrackerRegisterAddress, value);
        return true;
    });
}

CServerWindow::~CServerWindow() {}

bool CServerWindow::editing() {
    return descriptionBox->focused() || passwordBox->focused() || trackerBox->focused();
}

bool CServerWindow::DoCommand(int theCommand) {
    switch(theCommand) {
        case kNetChangedCmd:
            CAvaraAppImpl *app = (CAvaraAppImpl *)gApplication;
            switch(app->GetNet()->netStatus) {
                case kNullNet:
                    startBtn->setEnabled(true);
                    startBtn->setCaption("Start Hosting");
                    break;
                case kClientNet:
                    startBtn->setEnabled(false);
                    break;
                case kServerNet:
                    startBtn->setCaption("Stop Hosting");
                    break;
            }
            break;
    }
    return false;
}

