#include "CDirectObject.h"
#include "CApplication.h"
#include "nanovg.h"
#include "CBSPWorld.h"
#include "CBSPPart.h"
#include "CScaledBSP.h"
#include "CSmartBox.h"
#include "CStateFunction.hpp"
#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include "CViewParameters.h"
#include "RGBAColor.h"

#define kCursorBSP 801 
#define kAvaraLogo 100
#define kBlockBSP 400

class CGUIScreen;

class CGUI : public CDirectObject {
public:
    CApplication *itsApp;
    CGUI(CApplication *app);

    virtual ~CGUI() {}

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

typedef std::function<void(NVGcontext *c)> NVGCall;

typedef std::function<void(CGUI *c)> GUICall;

class CGUIWidget {
public:

    Rect _rect;
    std::vector<CGUIWidget> children;
    CSmartBox *_part;
    Vector _partLoc;
    NVGCall _nvgactions;

    CGUIWidget() {
        _partLoc[0] = 0;
        _partLoc[1] = 0;
        _partLoc[2] = 0;
    }
    virtual void attach(CGUI *gui) {
        gui->itsWorld->AddPart(_part);
    }
    virtual void update(CGUI *gui) = 0;
    virtual void activated() = 0;
    void render(NVGcontext *ctx) {
        _nvgactions(ctx);
    };
    virtual ~CGUIWidget() {
        //_gui->itsWorld->RemovePart(_part);
        //_part->Dispose();
    }
};


class CGUIScreen {
public: 
    CGUIScreen(int screen, CBSPWorld *world, CGUI *gui);
    virtual ~CGUIScreen(){};
    virtual bool handleSDLEvent(SDL_Event &event){ return false; };
    virtual void Render(NVGcontext *ctx) {
        for(auto i = _children.begin(); i != _children.end(); i++) {
            (*i)->render(ctx);
        }
    };
    virtual void Update() {
        for(auto i = _children.begin(); i != _children.end(); i++) {
            (*i)->update(_gui);
        }
    }
protected:
    int _screen;
    CBSPWorld *_world;
    CGUI *_gui;
    std::vector<CGUIWidget*> _children;
};

class CGUIButton : public CGUIWidget {
public:
    CGUIButton(NVGcolor color, 
               char *str, 
               char *font,
               int action, 
               Rect rect);
    void attach(CGUI *gui);
    void update(CGUI *gui);
    void activated() {}
    //void Render(NVGcontext *ctx);
    ~CGUIButton();

protected:
    NVGcolor _color;
    char *_str;
    int _action;
    float rot = 0;
    CSmartBox *_part;
};



