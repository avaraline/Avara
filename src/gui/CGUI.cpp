#include "CGUI.h"
#include "CApplication.h"
#include "CAvaraApp.h"
#include "CAvaraGame.h"
#include "CNetManager.h"
#include "AvaraGL.h"
#include "LevelLoader.h"
#include "CApplication.h"
#include "Preferences.h"
#include "CGUIScreen.h"
#include "CSoundHub.h"


const glm::vec3 CGUI::screenToWorld(Point *p) {
    float normalized_x = (((float)p->h / (float)gApplication->fb_size_x) * 2.0) - 1.0;
    float normalized_y = ((((float)p->v / (float)gApplication->fb_size_y) * 2.0) - 1.0) * -1.0;
    glm::vec4 v = glm::vec4(normalized_x, normalized_y, 1.0, -1);
    return AvaraGLScreenSpaceToWorldSpace(itsView, &v);
}


CGUI::CGUI(CAvaraGame *game) {
    itsGame = game;
    itsWorld = new CBSPWorldImpl;
    itsWorld->IBSPWorld(50);
    itsView = game->itsView;
    LookAtGUI();

    itsCursor = new CScaledBSP;
    itsCursor->IScaledBSP(FIX3(100), kCursorBSP, 0, 0);
    itsCursor->UpdateOpenGLData();
    cursorWorld = new CBSPWorldImpl;
    cursorWorld->IBSPWorld(1);
    cursorWorld->AddPart(itsCursor);
    
    started = SDL_GetTicks64();

    screen = new CGUIScreen(MainMenu, itsWorld, this);
}

void CGUI::LookAtGUI() {
    itsView->LookFrom(0, 0, FIX(1));
    itsView->LookAt(0, 0, 0);
    itsView->PointCamera();
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

void CGUI::PlaySound(short theSound) {
    CBasicSound *aSound;
    SoundLink *aLink;
    Vector v;
    v[0] = FIX(0);
    v[1] = FIX(0);
    v[2] = FIX(0);
    v[3] = FIX(1);

    aSound = gHub->GetSoundSampler(hubRate, theSound);
    aSound->SetVolume(FIX(12));
    aLink = gHub->GetSoundLink();
    PlaceSoundLink(aLink, v);
    aSound->SetSoundLink(aLink);
    aSound->Start();
    gHub->ReleaseLink(aLink);
}

void CGUI::mouse() {
    cursor_buttons = SDL_GetMouseState(&cursor_x, &cursor_y);

    Point p = pt(cursor_x, cursor_y);
    glm::vec3 worldpos = screenToWorld(&p);
    itsCursor->Reset();
    itsCursor->RotateZ(FIX(15 * dt));
    itsCursor->RotateX(FIX(270));
    itsCursor->RotateZ(FIX(30));
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
    PlaySound(411);
    return STAY; // CHANGETO(_transitonScreen);
}

StateFunction CGUI::_transitonScreen() {
    return STAY;
}


void CGUI::Render(NVGcontext *ctx) {
    nvgBeginFrame(ctx, gApplication->win_size_x, gApplication->win_size_y, gApplication->pixel_ratio);
    nvgBeginPath(ctx);

    LookAtGUI();
    itsWorld->Render(itsView);
    screen->Render(ctx);
    nvgEndFrame(ctx);
    cursorWorld->Render(itsView);
}


void CGUI::Dispose() {
    delete this;
}
