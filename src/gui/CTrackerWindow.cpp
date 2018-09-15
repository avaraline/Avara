#include "CTrackerWindow.h"

#include "CApplication.h"
#include "Preferences.h"


class TrackerInfo : public nanogui::Widget {
public:
    bool mDrawLine;
    
    TrackerInfo(nanogui::Widget *parent, std::string line1, std::string line2, bool drawLine = true)
        : nanogui::Widget(parent), mDrawLine(drawLine) {
        setLayout(new nanogui::BoxLayout(nanogui::Orientation::Vertical, nanogui::Alignment::Fill, 10, 5));
        nanogui::Label *line1label = new nanogui::Label(this, line1, "sans-bold");
        nanogui::Label *line2label = new nanogui::Label(this, line2, "sans", 14);
        line2label->setColor(nanogui::Color(200, 200));
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

    nanogui::Button *refreshBtn = new nanogui::Button(top, "Refresh", ENTYPO_ICON_CW);
    refreshBtn->setCallback([] {
    });

    //new Divider(this);
    
    nanogui::VScrollPanel *results = new nanogui::VScrollPanel(this);
    results->setFixedHeight(200); //nanogui::Vector2i(250, 100));
    
    nanogui::Widget *wrapper = new nanogui::Widget(results);
    wrapper->setLayout(new nanogui::BoxLayout(nanogui::Orientation::Vertical, nanogui::Alignment::Fill, 0, 0));
    for (int i = 0; i < 10; i++) {
        new TrackerInfo(wrapper, "Server Name", "6 players - 127.0.0.1", i > 0);
    }
}

CTrackerWindow::~CTrackerWindow() {}

bool CTrackerWindow::DoCommand(int theCommand) {
    return false;
}
