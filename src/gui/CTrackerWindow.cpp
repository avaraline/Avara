#include "CTrackerWindow.h"

#include "CApplication.h"
#include "Preferences.h"
#include "CAvaraApp.h"

#include <json.hpp>
#include "httplib.h"

using json = nlohmann::json;


class TrackerInfo : public nanogui::Widget {
public:
    bool mDrawLine;

    TrackerInfo(nanogui::Widget *parent, std::string line1, std::string line2, bool drawLine = true)
        : nanogui::Widget(parent), mDrawLine(drawLine) {
        nanogui::AdvancedGridLayout *layout = new nanogui::AdvancedGridLayout();
        layout->appendRow(1, 1);
        layout->appendRow(1, 1);
        layout->appendCol(1, 0.75);
        layout->appendCol(1, 0.25);

        nanogui::Label *line1label = new nanogui::Label(this, line1, "sans-bold");
        nanogui::Label *line2label = new nanogui::Label(this, line2, "sans", 14);
        line2label->setColor(nanogui::Color(200, 200));
        nanogui::Button *btn = new nanogui::Button(this, "Connect");
        btn->setCallback([this, line1, btn] {
            CAvaraAppImpl *avara = (CAvaraAppImpl *)gApplication;
            
            //connect if not hosting
            if(avara->GetNet()->netStatus != kServerNet) {
                avara->GetNet()->ChangeNet(kClientNet, line1);
            }
        });

        layout->setMargin(10);
        layout->setAnchor(line1label, nanogui::AdvancedGridLayout::Anchor(0, 0, 1, 1));
        layout->setAnchor(line2label, nanogui::AdvancedGridLayout::Anchor(0, 1, 1, 1));
        layout->setAnchor(btn, nanogui::AdvancedGridLayout::Anchor(1, 0, 1, 2, nanogui::Alignment::Maximum,
            nanogui::Alignment::Minimum));

        setLayout(layout);
    }

    void draw(NVGcontext* ctx) {
        nanogui::Widget::draw(ctx);

        if (mDrawLine) {
            nvgBeginPath(ctx);
            nvgMoveTo(ctx, mPos.x, mPos.y);
            nvgLineTo(ctx, mPos.x + mSize.x, mPos.y);
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
        nvgMoveTo(ctx, mPos.x, mPos.y);
        nvgLineTo(ctx, mPos.x + mSize.x, mPos.y);
        nvgStrokeColor(ctx, nvgRGBA(100, 100, 100, 150));
        nvgStroke(ctx);
    }
};

CTrackerWindow::CTrackerWindow(CApplication *app) : CWindow(app, "Tracker") {
    setLayout(new nanogui::BoxLayout(nanogui::Orientation::Vertical, nanogui::Alignment::Fill, 0, 5));

    nanogui::Widget *top = new nanogui::Widget(this);
    top->setLayout(new nanogui::FlowLayout(nanogui::Orientation::Horizontal, false, 10, 10));

    nanogui::TextBox *addressBox = new nanogui::TextBox(top);
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

void CTrackerWindow::Query() {
    std::string address = gApplication->String(kTrackerAddress);
    httplib::Client client(address.c_str(), 80);
    auto resp = client.Get("/api/v1/games/");
    if (resp && resp->status == 200) {
        json apiData = json::parse(resp->body);

        while (resultsBox->childCount() > 0) {
            resultsBox->removeChild(0);
        }

        int serverCount = apiData["games"].size();
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
            new TrackerInfo(resultsBox, game["address"].get<std::string>(), players, i > 0);
        }
        
        std::string serverStr = std::to_string(serverCount) + " server";
        if(serverCount != 1)
            serverStr.append("s");
        resultsLabel->setCaption(serverStr);

        setNeedsLayout();
    }
}
