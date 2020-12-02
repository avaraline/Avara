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
        if (avara->GetNet()->netStatus == kServerNet)
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

    latencyBox = new nanogui::TextBox(this);
    latencyBox->setValue(std::to_string(app->Number(kLatencyToleranceTag)));
    latencyBox->setEditable(false);
    latencyBox->setEnabled(false);
    latencyBox->setCallback([app](std::string value) -> bool {
        char * pointer;
        long newLT = strtol(value.c_str(), &pointer, 10);

        // determining the min/max LT values from CAvaraGame::AdjustFrameTime()
        long maxLT = 8;

        // make sure the provided value is an integer
        if (*pointer != 0)
            return false;

        if (newLT > maxLT)
            newLT = maxLT;

        if (newLT < 0)
            newLT = 0;

        app->Set(kLatencyToleranceTag, newLT);
        return true;
    });

    autoLatencyBox = new nanogui::CheckBox(this, "Auto Latency", [app](bool checked) {
        long options = app->Number(kServerOptionsTag);
        if (checked)
            options |= 1 << kUseAutoLatencyBit;
        else
            options &= ~(long)(1 << kUseAutoLatencyBit);
        app->Set(kServerOptionsTag, options);
        ((CAvaraAppImpl *)app)->GetNet()->ChangedServerOptions(options);
    });
    bool autoLatency = app->Number(kServerOptionsTag) & (1 << kUseAutoLatencyBit);
    autoLatencyBox->setChecked(autoLatency);
    autoLatencyBox->setEnabled(false);
}

CServerWindow::~CServerWindow() {}

bool CServerWindow::editing() {
    return descriptionBox->focused() || passwordBox->focused() || trackerBox->focused() || latencyBox->focused();
}

bool CServerWindow::DoCommand(int theCommand) {
    switch(theCommand) {
        case kNetChangedCmd:
            CAvaraAppImpl *app = (CAvaraAppImpl *)gApplication;
            switch(app->GetNet()->netStatus) {
                case kNullNet:
                    startBtn->setEnabled(true);
                    startBtn->setCaption("Start Hosting");
                    this->EnableLatencyOptions(false);
                    break;
                case kClientNet:
                    startBtn->setEnabled(false);
                    this->EnableLatencyOptions(false);
                    break;
                case kServerNet:
                    startBtn->setCaption("Stop Hosting");
                    this->EnableLatencyOptions(true);
                    break;
            }
            break;
    }
    return false;
}

void CServerWindow::PrefChanged(std::string name) {
    latencyBox->setValue(std::to_string(mApplication->Number(kLatencyToleranceTag)));
}

void CServerWindow::EnableLatencyOptions(bool enable) {
    this->latencyBox->setEditable(enable);
    this->latencyBox->setEnabled(enable);
    this->autoLatencyBox->setEnabled(enable);
}

