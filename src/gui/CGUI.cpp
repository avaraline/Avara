#include "CGUI.h"
#include "CApplication.h"
#include "AvaraGL.h"
#include "LevelLoader.h"
#include "CApplication.h"
#include "Preferences.h"

NVGcolor __white = nvgRGBA(255,255,255,255);
NVGcolor __red = nvgRGBA(255,0,0,0);

NVGCall flat_map(NVGCall a, NVGCall b) {
    return [a, b] (NVGcontext *c) {
        a(c);
        b(c);
    };
};

const Point pt(short x, short y) {
    Point p;
    p.h = x;
    p.v = y;
    return p;
}

const Point midpoint(Rect &r) {
    return pt((short)(r.left + ((r.right - r.left) / 2)), 
              (short)(r.top + ((r.bottom - r.top) / 2)));
};

const NVGCall text(const char *s, const char *font, NVGcolor color, Point pos) {
    return [s, font, color, pos](NVGcontext *ctx) {
        nvgFontFace(ctx, font);
        nvgFillColor(ctx, color);
        nvgTextAlign(ctx, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
        nvgFontSize(ctx, 25);
        nvgText(ctx, pos.h, pos.v, s, NULL);
    };
};

const NVGCall rect(Rect &r, NVGcolor &c) {
    return [r, c](NVGcontext *ctx) {
        nvgBeginPath(ctx);
        short w = std::abs(r.right - r.left);
        short h = std::abs(r.top - r.bottom);
        nvgRect(ctx, r.left , r.top, w, h);
        nvgStrokeColor(ctx, c);
        nvgStroke(ctx);
        nvgClosePath(ctx);
    };
};

const NVGCall quick_text(const char *s, Point pos) {
    return text(s, "mono", __white, pos);
};

const glm::vec3 CGUI::screenToWorld(Point *p) {
    float normalized_x = (((float)p->h / (float)itsApp->fb_size_x) * 2.0) - 1.0;
    float normalized_y = ((((float)p->v / (float)itsApp->fb_size_y) * 2.0) - 1.0) * -1.0;
    glm::vec4 v = glm::vec4(normalized_x, normalized_y, 1.0, -1);
    return AvaraGLScreenSpaceToWorldSpace(itsView, &v);
}


CGUI::CGUI(CApplication *app) {
    itsApp = app;
    itsWorld = new CBSPWorldImpl;
    itsWorld->IBSPWorld(50);
    itsView = new CViewParameters;
    itsView->IViewParameters();
    itsView->hitherBound = FIX3(1);
    itsView->horizonBound = FIX(35);
    itsView->yonBound = FIX(35);
    itsView->dirtyLook = true;
    itsView->ambientLight = FIX3(800);
    long wht = RGBAToLong(__white);
    itsView->ambientLightColor = wht;
    AvaraGLSetAmbient(ToFloat(itsView->ambientLight), itsView->ambientLightColor);
    itsView->LookFrom(0, 0, FIX(1));
    itsView->LookAt(0, 0, 0);
    itsView->PointCamera();
    AvaraGLViewport(app->fb_size_x, app->fb_size_y);
    itsView->SetViewRect(app->fb_size_x, app->fb_size_y, app->fb_size_x / 2, app->fb_size_y / 2);
    itsView->viewPixelRatio = FIX(4.0/3.0);
    itsView->CalculateViewPyramidCorners();

    itsCursor = new CScaledBSP;
    itsCursor->IScaledBSP(FIX3(100), kCursorBSP, 0, 0);
    itsCursor->UpdateOpenGLData();
    TranslatePart(itsCursor, 0, 0, 0);
    itsCursor->Reset();
    itsCursor->RotateX(FIX(270));
    itsCursor->RotateZ(FIX(30));
    itsCursor->MoveDone();
    cursorWorld = new CBSPWorldImpl;
    cursorWorld->IBSPWorld(1);
    cursorWorld->AddPart(itsCursor);
    
    started = SDL_GetTicks64();

    AvaraGLSetFOV(app->Number(kFOV));
    //AvaraGLLightDefaults();
    itsView->DoLighting();
    AvaraGLViewport(app->fb_size_x, app->fb_size_y);

    screen = new CGUIScreen(MainMenu, itsWorld, this);
}

void CGUI::Update() {
    state = state();
    last_t = t;
    t = SDL_GetTicks64();
    dt = t - last_t;
    if(screen) {
        screen->Update();
    }
    //itsCursor->RotateZ(FIX3(3 * dt));
}

void CGUI::mouse() {
    cursor_buttons = SDL_GetMouseState(&cursor_x, &cursor_y);
    Point p = pt(cursor_x, cursor_y);
    glm::vec3 worldpos = screenToWorld(&p);
    PlacePart(itsCursor, ToFixed(worldpos.x), ToFixed(worldpos.y), 0);
    TranslatePart(itsCursor, FIX3(15), FIX3(-25), 0);
    itsCursor->MoveDone();
}

bool CGUI::handleSDLEvent(SDL_Event &event) {
    switch(event.type) {
        case SDL_MOUSEMOTION:
        case SDL_MOUSEBUTTONDOWN:
            mouse();
            return true;
        case SDL_KEYDOWN:
            return true;
    }
    return false;
}

#define STAY std::move(state)
#define CHANGETO(t) std::bind(&CGUI::t, this)

StateFunction CGUI::_startup() {
    //if ()
    //auto *AvaraPart = new CBSPPart;
    //AvaraPart->IBSPPart(kAvaraLogo);
    return STAY;
}

StateFunction CGUI::_transitonScreen() {
    return STAY;
}


void CGUI::Render(NVGcontext *ctx) {
    nvgBeginFrame(ctx, itsApp->win_size_x, itsApp->win_size_y, itsApp->pixel_ratio);
    nvgBeginPath(ctx);

    auto wht = RGBAToLong(__white);
    //AvaraGLSetLight(0, .8, 20, 180, wht);
    //AvaraGLSetLight(1, .8, 20, 90, wht);
    //AvaraGLSetLight(2, .8, 20, 270, wht);


    itsWorld->Render(itsView);
    screen->Render(ctx);
    nvgEndFrame(ctx);
    cursorWorld->Render(itsView);
}

CGUIScreen::CGUIScreen(int screen, CBSPWorld *world, CGUI *gui) {
    _screen = screen;
    _world = world;
    _gui = gui;

    switch(screen){
        case MainMenu:
        {
            Rect r;
            r.top = 50;
            r.bottom = 100;
            r.left = 100;
            r.right = 300;
            CGUIButton *b = new CGUIButton(
                nvgRGB(25, 255, 255),
                "CLICK???",
                "mono",
                0,
                r
            );
            _children.push_back(b);
            b->attach(_gui);
            break;
        }
        default:
            break;
    }
}

void CGUI::Dispose() {
    delete this;
}



CGUIButton::CGUIButton(NVGcolor color, 
               char *str, 
               char *font,
               int action, 
               Rect r){
        _color = color;
        _str = str;
        _action = action;
        _rect = r;
        Point mid = midpoint(r);
        _nvgactions = text(_str, font, color, mid);
};

CGUIButton::~CGUIButton() {

}

void CGUIButton::attach(CGUI *gui) {
    Point s_topleft = pt(_rect.left, _rect.top);
    Point s_bottomright = pt(_rect.right, _rect.bottom);
    glm::vec3 ws_topleft = gui->screenToWorld(&s_topleft);
    glm::vec3 ws_bottomright = gui->screenToWorld(&s_bottomright);
    Vector dims;
    dims[0] = FIX(std::abs(ws_bottomright.x - ws_topleft.x) / 2.0);
    dims[1] = FIX(std::abs(ws_bottomright.y - ws_topleft.y) / 2.0);
    dims[2] = FIX3(1);
    _part = new CSmartBox;
    _part->ISmartBox(kBlockBSP,
                     dims, 
                     RGBAToLong(_color), 
                     RGBAToLong(_color), 
                     0, 0);

    _part->ReplaceColor(0x00fefefe, 0x00ffffff);
    Point mid = midpoint(_rect);
    glm::vec3 worldpos = gui->screenToWorld(&mid);
    _partLoc[0] = ToFixed(worldpos.x);
    _partLoc[1] = ToFixed(worldpos.y);
    _partLoc[2] = 0;
    _part->UpdateOpenGLData();
    gui->itsWorld->AddPart(_part);
}

void CGUIButton::update(CGUI *gui) {
        if(_rect.left < gui->cursor_x
        && gui->cursor_x < _rect.right
        && _rect.top < gui->cursor_y 
        && gui->cursor_y < _rect.bottom) {
            rot += ((float)gui->dt / 1000.0) / 2.0f;
            if (gui->cursor_buttons & SDL_BUTTON_LMASK) {
                _nvgactions = quick_text("CLICKED!", midpoint(_rect));
            }
        }
        else {
            rot = 0;
        }

        
        _part->Reset();
        InitialRotatePartY(_part, FIX(rot));
        TranslatePart(_part, _partLoc[0], _partLoc[1], _partLoc[2]);
        _part->MoveDone();
    }