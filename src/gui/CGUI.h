#pragma once
#include "CDirectObject.h"
#include "CApplication.h"
#include "nanovg.h"
#include "CBSPWorld.h"
#include "CBSPPart.h"
#include "CSmartBox.h"
#include "CWallActor.h"
#include "CScaledBSP.h"
#include "CStateFunction.hpp"
#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include "CViewParameters.h"
#include "ARGBColor.h"
#include "AvaraGL.h"
extern "C" {
#include "microui.h"
}

#define kCursorBSP 801 
#define kAvaraLogo 100
#define kBlockBSP 400
#define kOutlineBSP 722

class CAvaraAppImpl;

class CGUI : public CDirectObject {
public:
    CAvaraGame *itsGame;
    CAvaraAppImpl *itsApp;
    CGUI(CAvaraAppImpl *app);

    void LookAtGUI();
    void PlaySound(short theSound);
    void Render(NVGcontext *ctx);
    void Update();

    bool handleSDLEvent(SDL_Event &event);
    Fixed *rectToDims(Rect *r);
    void Dispose();

    CBSPWorldImpl *itsWorld;
    CBSPWorldImpl *cursorWorld;
    CViewParameters *itsView;
    CScaledBSP *itsCursor;
    uint32_t cursor_buttons;
    int cursor_x;
    int cursor_y;
    bool active = true;
    uint64_t started = 0;
    uint64_t t = 0;
    uint64_t last_t = 0;
    uint64_t dt = 0;
    uint16_t anim_timer = 0;

    const glm::vec3 screenToWorld(Point *p) {
        float normalized_x = (((float)p->h / (float)gApplication->fb_size_x) * 2.0) - 1.0;
        float normalized_y = ((((float)p->v / (float)gApplication->fb_size_y) * 2.0) - 1.0) * -1.0;
        glm::vec4 v = glm::vec4(normalized_x, normalized_y, 1.0, -1);
        return AvaraGLScreenSpaceToWorldSpace(itsView, &v);
    }

    static int text_width(mu_Font f, const char *text, int len) {
        return (int)round(nvgTextBounds(gApplication->nvg_context, 0, 0, text, nullptr, nullptr));
    }

    static int text_height(mu_Font f) {
        float lineh;
        nvgTextMetrics(gApplication->nvg_context, nullptr, nullptr, &lineh);
        return (int)round(lineh);
    }

    const Point pt(short x, short y) {
        Point p;
        p.h = x;
        p.v = y;
        return p;
    }

    static const long RGBAToLong(mu_Color c) {
        return (
            (static_cast<int>(c.a + 0.5) << 24) +
            (static_cast<int>(c.r) << 16) +
            (static_cast<int>(c.g) << 8) +
            static_cast<int>(c.b)
        );
    }

    static const NVGcolor toNVGcolor(mu_Color other) {
        NVGcolor c;
        c.r = other.r / 255.0;
        c.g = other.g / 255.0;
        c.b = other.b / 255.0;
        c.a = other.a / 255.0;
        return c;
    }

    void PlaceGUIPart(CBSPPart* _part, mu_Rect r);


    mu_Context *mui_ctx;
    //std::map<mu_Id, CSmartBox*> boxes;
    //std::map<mu_Id, CSmartBox*> outlines;
    std::map<mu_Id, CWallActor*> boxes;
    int BSPButton(std::string label);
    int BSPWidget(mu_Rect r, int res, mu_Id mu_id);
    int BSPTextInput(std::string s);

    void ClearParts() {
        for (const auto &x: boxes) {
            x.second->Dispose();
            delete x.second;
        }

        boxes.clear();
    }
    
    virtual ~CGUI() {
        ClearParts();
    }


protected:

    void mouse();
    void kb();
    void joystick();

    short targetScreen;

    StateFunction _startup();
    StateFunction _transitionScreen();

    StateFunction _inactive();
    StateFunction _active();
    StateFunction _textInput(); 
    StateFunction _modal();

    StateFunction state = std::bind(&CGUI::_startup, this);
};

enum GUIScreen {
    MainMenu,
    Solo,
    Tracker,
    HostGame,
    Server,
    Options,
    Keybind,
    About,
    Test
};


//typedef std::function<void(CGUI *c)> GUICall;





