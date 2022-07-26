#pragma once
#include "CDirectObject.h"
#include "CApplication.h"
#include "nanovg.h"
#include "CBSPWorld.h"
#include "CBSPPart.h"
#include "CSmartBox.h"
#include "CScaledBSP.h"
#include "CStateFunction.hpp"
#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include "CViewParameters.h"
#include "RGBAColor.h"
#include "CGUICommon.h"

#define kCursorBSP 801 
#define kAvaraLogo 100
#define kBlockBSP 400
#define kOutlineBSP 722

class CGUIScreen;
class CAvaraGame;

class CGUI : public CDirectObject {
public:
    CAvaraGame *itsGame;
    CGUI(CAvaraGame *game);

    virtual ~CGUI() {}
    void LookAtGUI();
    void PlaySound(short theSound);
    void Render(NVGcontext *ctx);
    void Update();

    bool handleSDLEvent(SDL_Event &event);

    const glm::vec3 screenToWorld(Point *p);
    Fixed *rectToDims(Rect *r);
    void Dispose();

    CBSPWorldImpl *itsWorld;
    CBSPWorldImpl *cursorWorld;
    CViewParameters *itsView;
    CScaledBSP *itsCursor;
    uint32_t cursor_buttons;
    int cursor_x;
    int cursor_y;
    uint64_t started = 0;
    uint64_t t = 0;
    uint64_t last_t = 0;
    uint64_t dt = 0;

protected:

    CGUIScreen *screen;

    void mouse();
    void kb();
    void joystick();

    short targetScreen;

    StateFunction _startup();
    StateFunction _transitonScreen();

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


typedef std::function<void(CGUI *c)> GUICall;





