#include "CServerWindow.h"

#include "CAvaraApp.h"
#include "CNetManager.h"
#include "Preferences.h"
#include "CommandList.h"
#include "CAvaraGame.h"  // gCurrentGame

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
    latencyBox->setValue(std::to_string(app->Get<float>(kLatencyToleranceTag)).substr(0, 5));
    latencyBox->setEditable(false);
    latencyBox->setEnabled(false);
    latencyBox->setCallback([this, app](std::string value) -> bool {
        double newLT = std::stod(value);

        // determining the min/max LT values from CAvaraGame::AdjustFrameTime()
        long maxLT = 4;  // anything above 4 is bad, don't let people set it higher
        if (newLT > maxLT)
            newLT = maxLT;

        if (newLT < 0)
            newLT = 0;

        newLT = app->SetLcd(kLatencyToleranceTag, newLT, gCurrentGame->fpsScale);
        // it might be modified on a bad input so get back the saved value
        latencyBox->setValue(std::to_string(newLT).substr(0, 5));
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

    std::vector<std::string> frameTimeOptions = {
        "64 ms (15.625 fps)", "32 ms (31.25 fps)", "16 ms (62.5 fps)", "8 ms (125 fps)"
    };
    std::vector<std::string> frameTimeOptionsShort = { "64 ms", "32 ms", "16 ms", "8 ms" };
    frameTimeBox = new nanogui::ComboBox(this, frameTimeOptions, frameTimeOptionsShort);
    frameTimeBox->setCallback([this, app](int selectedIdx) {
        gCurrentGame->SetFrameTime(pow(2, 6-selectedIdx));
    });
    frameTimeBox->setSelectedIndex(6-log(gCurrentGame->frameTime)/log(2));
    frameTimeBox->popup()->setSize(nanogui::Vector2i(200, 160));
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
                    frameTimeBox->setEnabled(true);
                    startBtn->setCaption("Start Hosting");
                    this->EnableLatencyOptions(false);
                    break;
                case kClientNet:
                    startBtn->setEnabled(false);
                    frameTimeBox->setEnabled(false);
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
    frameTimeBox->setSelectedIndex(6-log(gCurrentGame->frameTime)/log(2));
    latencyBox->setValue(std::to_string(mApplication->Get<float>(kLatencyToleranceTag)).substr(0, 5));
}

void CServerWindow::EnableLatencyOptions(bool enable) {
    latencyBox->setEditable(enable);
    latencyBox->setEnabled(enable);
    // force a callback which could change the LT depending on frame rate
    if (enable) latencyBox->callback()(latencyBox->value());
    autoLatencyBox->setEnabled(enable);
}
