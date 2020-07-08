#include "CNetworkWindow.h"

#include "CAvaraApp.h"
#include "CNetManager.h"
#include "Preferences.h"
#include "CommandList.h"

CNetworkWindow::CNetworkWindow(CApplication *app) : CWindow(app, "Network") {
    setLayout(new nanogui::BoxLayout(nanogui::Orientation::Vertical, nanogui::Alignment::Fill, 10, 10));

    addressBox = new nanogui::TextBox(this);
    addressBox->setPlaceholder("Address");
    addressBox->setValue(app->String(kLastAddress));
    addressBox->setEditable(true);
    addressBox->setCallback([app](std::string value) -> bool {
        app->Set(kLastAddress, value);
        return true;
    });

    connectBtn = new nanogui::Button(this, "Connect");
    connectBtn->setCallback([this, app] {
        CAvaraAppImpl *avara = (CAvaraAppImpl *)app;
        if(avara->GetNet()->netStatus == kClientNet)
            avara->GetNet()->ChangeNet(kNullNet, "");
        else
            avara->GetNet()->ChangeNet(kClientNet, this->addressBox->value());
    });

    latencyBox = new nanogui::TextBox(this);
    latencyBox->setValue(std::to_string(app->Number(kLatencyToleranceTag)));
    latencyBox->setEditable(true);
    latencyBox->setCallback([app](std::string value) -> bool {
        app->Set(kLatencyToleranceTag, std::stoi(value));
        return true;
    });

    nanogui::CheckBox *autoLatencyBox = new nanogui::CheckBox(this, "Auto Latency", [app](bool checked) {
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
}

CNetworkWindow::~CNetworkWindow() {}

bool CNetworkWindow::editing() {
    return addressBox->focused() || latencyBox->focused();
}

bool CNetworkWindow::DoCommand(int theCommand) {
    switch(theCommand) {
        case kNetChangedCmd:
            CAvaraAppImpl *app = (CAvaraAppImpl *)gApplication;
            switch(app->GetNet()->netStatus) {
                case kNullNet:
                    addressBox->setEnabled(true);
                    connectBtn->setEnabled(true);
                    connectBtn->setCaption("Connect");
                    break;
                case kClientNet:
                    addressBox->setEnabled(false);
                    connectBtn->setCaption("Disconnect");
                    break;
                case kServerNet:
                    addressBox->setEnabled(false);
                    connectBtn->setEnabled(false);
                    break;
            }
            break;
    }
    return false;
}

void CNetworkWindow::PrefChanged(std::string name) {
    latencyBox->setValue(std::to_string(mApplication->Number(kLatencyToleranceTag)));
}
