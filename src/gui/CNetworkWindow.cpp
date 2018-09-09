#include "CNetworkWindow.h"

#include "CAvaraApp.h"
#include "CNetManager.h"
#include "Preferences.h"

CNetworkWindow::CNetworkWindow(CApplication *app) : CWindow(app, "Network") {
    setLayout(new nanogui::BoxLayout(nanogui::Orientation::Vertical, nanogui::Alignment::Fill, 20, 10));

    nanogui::TextBox *addressBox = new nanogui::TextBox(this);
    addressBox->setValue("avara.jp");
    addressBox->setEditable(true);

    nanogui::Button *connectBtn = new nanogui::Button(this, "Connect");
    connectBtn->setCallback(
        [app, addressBox] { ((CAvaraApp *)app)->gameNet->ChangeNet(kClientNet, addressBox->value()); });

    nanogui::Button *startBtn = new nanogui::Button(this, "Start Hosting");
    startBtn->setCallback([app] { ((CAvaraApp *)app)->gameNet->ChangeNet(kServerNet, ""); });

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
        ((CAvaraApp *)app)->gameNet->ChangedServerOptions(options);
    });
    bool autoLatency = app->Number(kServerOptionsTag) & (1 << kUseAutoLatencyBit);
    autoLatencyBox->setChecked(autoLatency);
}

CNetworkWindow::~CNetworkWindow() {}

void CNetworkWindow::PrefChanged(std::string name) {
    latencyBox->setValue(std::to_string(mApplication->Number(kLatencyToleranceTag)));
}
