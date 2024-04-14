#include "CTrackerWindow.h"

#include "CApplication.h"
#include "Preferences.h"
#include "CAvaraApp.h"

#include <json.hpp>
#include "httplib.h"

using json = nlohmann::json;

class PasswordConnect : public CWindow {
    
public:
    
    nanogui::TextBox *passwordBox;
    
    bool editing() {
        return passwordBox->focused();
    }
    
    PasswordConnect(CApplication *avara, std::string address) : CWindow(avara, "Password") {
        setLayout(new nanogui::BoxLayout(nanogui::Orientation::Vertical,
                                                         nanogui::Alignment::Middle, 10, 10));
        setModal(true);
        
        Widget *panel1 = new Widget(this);
        panel1->setLayout(new nanogui::BoxLayout(nanogui::Orientation::Horizontal,
                                                 nanogui::Alignment::Middle, 10, 15));
        
        nanogui::Label *messageLabel = new nanogui::Label(panel1, "Enter the server password");
        messageLabel->setFixedWidth(200);
        passwordBox = new nanogui::TextBox(panel1);
        passwordBox->setFixedWidth(150);
        passwordBox->setValue(avara->String(kClientPassword));
        passwordBox->setEditable(true);
        
        Widget *panel2 = new Widget(this);
        panel2->setLayout(new nanogui::BoxLayout(nanogui::Orientation::Horizontal,
                                                 nanogui::Alignment::Middle, 0, 15));
        
        nanogui::Button *cancelButton = new nanogui::Button(panel2, "Cancel");
        cancelButton->setCallback([this] {
            this->dispose();
        });
        nanogui::Button *connectButton = new nanogui::Button(panel2, "Connect");
        connectButton->setCallback([avara, this, address] {
            avara->Set(kClientPassword, passwordBox->value());
            ((CAvaraAppImpl *)avara)->GetNet()->ChangeNet(kClientNet, address, passwordBox->value());
            this->dispose();
        });
        
        center();
        passwordBox->requestFocus();
    }
};

class TrackerInfo : public nanogui::Widget {
public:
    bool mDrawLine;

    TrackerInfo(nanogui::Widget *parent, std::string line1, std::string line2,
                std::string description, bool password = false, bool drawLine = true)
        : nanogui::Widget(parent), mDrawLine(drawLine) {
        nanogui::AdvancedGridLayout *layout = new nanogui::AdvancedGridLayout();
        layout->appendRow(1, 1);
        layout->appendRow(1, 1);
        layout->appendRow(1, 1);
        layout->appendRow(1, 1);
        layout->appendCol(1, 0.60);
        layout->appendCol(1, 0.40);
            
        nanogui::Label *addressLabel = new nanogui::Label(this, line1, "sans", 14);
        nanogui::Label *playersLabel = new nanogui::Label(this, line2, "sans-bold");
        playersLabel->setFixedWidth(300);
        playersLabel->setColor(nanogui::Color(200, 200));
        nanogui::Label *descriptionLabel = new nanogui::Label(this, description, "sans-bold");
        descriptionLabel->setFixedWidth(200);
            
        std::string btnStr(password ? "Connect..." : "Connect");
        nanogui::Button *btn = new nanogui::Button(this, btnStr);
        //btn->setFixedWidth(100);
        btn->setCallback([line1, password] {
            CAvaraAppImpl *avara = (CAvaraAppImpl *)gApplication;
            
            //connect if not hosting
            if(avara->GetNet()->netStatus != kServerNet) {
                if(password == true)
                    new PasswordConnect(avara, line1);
                else
                    avara->GetNet()->ChangeNet(kClientNet, line1);
            }
        });

        layout->setMargin(10);
        layout->setAnchor(descriptionLabel, nanogui::AdvancedGridLayout::Anchor(0, 0, 1, 2, nanogui::Alignment::Fill, nanogui::Alignment::Middle));
        layout->setAnchor(btn, nanogui::AdvancedGridLayout::Anchor(1, 0, 1, 1, nanogui::Alignment::Fill, nanogui::Alignment::Minimum));
        layout->setAnchor(addressLabel, nanogui::AdvancedGridLayout::Anchor(0, 2, 1, 1));
        layout->setAnchor(playersLabel, nanogui::AdvancedGridLayout::Anchor(0, 3, 2, 1));

        setLayout(layout);
    }

    void draw(NVGcontext* ctx) {
        nanogui::Widget::draw(ctx);

        if (mDrawLine) {
            nvgBeginPath(ctx);
            nvgMoveTo(ctx, (float)mPos.x, (float)mPos.y);
            nvgLineTo(ctx, (float)(mPos.x + mSize.x), (float)mPos.y);
            nvgStrokeColor(ctx, nvgRGBA(100, 100, 100, 150));
            nvgStroke(ctx);
        }
    }
};

class Divider : public nanogui::Widget {
public:
    Divider(nanogui::Widget *parent) : nanogui::Widget(parent) {
        setFixedHeight(2);
    }

    void draw(NVGcontext* ctx) {
        nvgBeginPath(ctx);
        nvgMoveTo(ctx, (float)mPos.x, (float)mPos.y);
        nvgLineTo(ctx, (float)(mPos.x + mSize.x), (float)mPos.y);
        nvgStrokeColor(ctx, nvgRGBA(100, 100, 100, 150));
        nvgStroke(ctx);
    }
};

CTrackerWindow::CTrackerWindow(CApplication *app) : CWindow(app, "Tracker") {
    setLayout(new nanogui::BoxLayout(nanogui::Orientation::Vertical, nanogui::Alignment::Fill, 0, 5));

    nanogui::Widget *top = new nanogui::Widget(this);
    top->setLayout(new nanogui::FlowLayout(nanogui::Orientation::Horizontal, false, 10, 10));

    addressBox = new nanogui::TextBox(top);
    addressBox->setValue(app->String(kTrackerAddress));
    addressBox->setEditable(true);
    addressBox->setCallback([app](std::string value) -> bool {
        app->Set(kTrackerAddress, value);
        return true;
    });

    nanogui::Button *refreshBtn = new nanogui::Button(top, "Refresh", ENTYPO_ICON_CW);
    refreshBtn->setCallback([this] {
        resultsLabel->setCaption("");
        this->Query();
    });
    resultsLabel = new nanogui::Label(top, "");

    //new Divider(this);

    nanogui::VScrollPanel *results = new nanogui::VScrollPanel(this);
    results->setFixedHeight(200); //nanogui::Vector2i(250, 100));

    resultsBox = new nanogui::Widget(results);
    resultsBox->setLayout(new nanogui::BoxLayout(nanogui::Orientation::Vertical, nanogui::Alignment::Fill, 0, 0));
    /*
    for (int i = 0; i < 10; i++) {
        new TrackerInfo(resultsBox, "Server Name", "6 players - 127.0.0.1", i > 0);
    }
    */
}

CTrackerWindow::~CTrackerWindow() {}

bool CTrackerWindow::DoCommand(int theCommand) {
    return false;
}

bool CTrackerWindow::editing() {
    return addressBox->focused();
}

void CTrackerWindow::Query() {
    std::string address = gApplication->String(kTrackerAddress);
    httplib::Client client(address.c_str(), 80);
    auto resp = client.Get("/api/v1/games/");
    if (resp && resp->status == 200) {
        json apiData = json::parse(resp->body);

        while (resultsBox->childCount() > 0) {
            resultsBox->removeChild(0);
        }
        auto serverCount = apiData["games"].size();
        for (int i = 0; i < serverCount; i++) {
            auto game = apiData["games"][i];
            std::string players;
            bool commas = false;
            for (const auto &p : game["players"]) {
                if (commas) {
                    players += ", ";
                }
                players += p.get<std::string>();
                commas = true;
            }

            bool password = false;
            if(! game["password"].is_null())
                 password = game["password"].get<bool>();
            
            new TrackerInfo(resultsBox, game["address"].get<std::string>(), players, game["description"].get<std::string>(), password, i > 0);
        }

        std::string serverStr = std::to_string(serverCount) + " server";
        if(serverCount != 1)
            serverStr.append("s");
        resultsLabel->setCaption(serverStr);

        setNeedsLayout();
    }
}
