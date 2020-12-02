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
}

CNetworkWindow::~CNetworkWindow() {}

bool CNetworkWindow::editing() {
    return addressBox->focused();
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
