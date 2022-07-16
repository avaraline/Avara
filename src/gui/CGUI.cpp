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
extern "C" {
#include "microui.h"
}
mu_Context *mui_ctx;

const glm::vec3 CGUI::screenToWorld(Point *p) {
    float normalized_x = (((float)p->h / (float)gApplication->fb_size_x) * 2.0) - 1.0;
    float normalized_y = ((((float)p->v / (float)gApplication->fb_size_y) * 2.0) - 1.0) * -1.0;
    glm::vec4 v = glm::vec4(normalized_x, normalized_y, 1.0, -1);
    return AvaraGLScreenSpaceToWorldSpace(itsView, &v);
}
/*
static auto text_width(mu_Font f, const char *text, int len) {
    if (len == -1) { len = strlen(text); }
    return [f, text, len](NVGcontext *ctx) { return nvgTextBounds(ctx, 0, 0, text, nullptr, nullptr); };
}

static auto text_width(NVGcontext *ctx) {
    return [ctx](mu_Font f, const char *text, int len) {
        return (int)nvgTextBounds(ctx, 0, 0, text, nullptr, nullptr);
    };
}

static auto text_height(NVGcontext* ctx) {
    return [ctx](mu_Font f) {
        float lineh;
        nvgTextMetrics(ctx, nullptr, nullptr, &lineh);
        return (int)lineh;
    };
}

static int text_height(NVGcontext* ctx, mu_Font f) {
    float asc, desc, lineh;
    nvgTextMetrics(ctx, &asc, &desc, &lineh);
    return lineh;
}
*/

static int text_width(mu_Font f, const char *text, int len) {
    return (int)round(nvgTextBounds(gApplication->nvg_context, 0, 0, text, nullptr, nullptr));
}
static int text_height(mu_Font f) {
    float lineh;
    nvgTextMetrics(gApplication->nvg_context, nullptr, nullptr, &lineh);
    return (int)round(lineh);
}

CGUI::CGUI(CAvaraGame *game) {
    itsGame = game;
    itsGame->InitMixer(false);
    itsWorld = new CBSPWorldImpl;
    itsWorld->IBSPWorld(50);
    itsView = game->itsView;
    LookAtGUI();
    
    mui_ctx = (mu_Context*)malloc(sizeof(mu_Context));
    mu_init(mui_ctx);
    mui_ctx->text_width = text_width;
    mui_ctx->text_height = text_height;

    itsGame->itsApp->LoadLevel("aa-normal", "bwadi.alf", NULL);

    itsCursor = new CScaledBSP;
    itsCursor->IScaledBSP(FIX3(100), kCursorBSP, 0, 0);
    itsCursor->privateAmbient = FIX3(800);
    itsCursor->UpdateOpenGLData();
    cursorWorld = new CBSPWorldImpl;
    cursorWorld->IBSPWorld(1);
    cursorWorld->AddPart(itsCursor);
    
    started = SDL_GetTicks64();

    //screen = new CGUIScreen(MainMenu, itsWorld, this);
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
    itsGame->GameTick();
    //screen->Update();
    mui_ctx->style->padding = 10;
    mu_begin(mui_ctx);
    int muiflags = MU_OPT_NOFRAME | MU_OPT_NOTITLE | MU_OPT_NOCLOSE | MU_OPT_NOINTERACT | MU_OPT_NORESIZE | MU_OPT_AUTOSIZE;
    if(mu_begin_window_ex(mui_ctx, 
                          "Main", 
                          mu_rect(0, 0, gApplication->fb_size_x, gApplication->fb_size_y), 
                          muiflags)) {

        mu_layout_row(mui_ctx, 1, (int[]) { gApplication->fb_size_x / 5 }, gApplication->fb_size_y / 9);

        if (mu_button(mui_ctx, "PLAY")) {
            PlaySound(411);
        }
        if (mu_button(mui_ctx, "ABOUT")) {
            PlaySound(411);
        }
        if (mu_button(mui_ctx, "OPTIONS")) {
            PlaySound(411);
        }
        if (mu_button(mui_ctx, "QUIT")) {
            SDL_Event sdlevent;
            sdlevent.type = SDL_QUIT;
            SDL_PushEvent(&sdlevent);
        }

        mu_end_window(mui_ctx);
    }
    mu_end(mui_ctx);
}

void CGUI::PlaySound(short theSound) {
    CBasicSound *aSound;
    SoundLink *aLink;
    Vector v;
    v[0] = FIX(0);
    v[1] = FIX(0);
    v[2] = FIX(0);
    v[3] = FIX(1);
    gHub->LoadSample(theSound);
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
            mouse();
            mu_input_mousemove(mui_ctx, event.motion.x, event.motion.y);
            break;
        case SDL_MOUSEWHEEL:
            mu_input_scroll(mui_ctx, 0, event.wheel.y * -30);
            break;
        case SDL_TEXTINPUT:
            mu_input_text(mui_ctx, event.text.text);
            break;

        case SDL_KEYDOWN:

            break;

        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
        {
            mouse();
            int b = event.button.button;
            if (b && event.type == SDL_MOUSEBUTTONDOWN)
            {
                mu_input_mousedown(mui_ctx, event.button.x, event.button.y, b);
            }
            if (b && event.type == SDL_MOUSEBUTTONUP)
            {
                mu_input_mouseup(mui_ctx, event.button.x, event.button.y, b);
            }
            break;
        }
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
    return CHANGETO(_transitonScreen);
}

StateFunction CGUI::_transitonScreen() {
    return STAY;
}

const NVGcolor toNVGcolor(mu_Color other) {
    NVGcolor c;
    c.r = other.r / 255.0;
    c.g = other.g / 255.0;
    c.b = other.b / 255.0;
    c.a = other.a / 255.0;
    return c;
}

void CGUI::Render(NVGcontext *ctx) {

    nvgSave(ctx);
    nvgBeginFrame(ctx, gApplication->win_size_x, gApplication->win_size_y, gApplication->pixel_ratio);
    nvgBeginPath(ctx);


    LookAtGUI();
    itsWorld->Render(itsView);
    //screen->Render(ctx);

    mu_Command *cmd = NULL;
    while (mu_next_command(mui_ctx, &cmd)) {
        if (cmd->type == MU_COMMAND_RECT) {
            NVGcolor c = toNVGcolor(cmd->rect.color);
            nvgBeginPath(ctx);
            nvgRect(ctx, cmd->rect.rect.x, cmd->rect.rect.y, cmd->rect.rect.w, cmd->rect.rect.h);
            nvgStrokeColor(ctx, c);
            nvgFillColor(ctx, c);
            nvgStroke(ctx);
            nvgFill(ctx);
            nvgClosePath(ctx);
        }
        if (cmd->type == MU_COMMAND_TEXT) {
            //nvgScale(ctx, .7, 1.0);
            nvgFontFace(ctx, "archivo");
            nvgTextAlign(ctx, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
            nvgFillColor(ctx, toNVGcolor(cmd->text.color));
            nvgFontSize(ctx, 65);
            nvgText(ctx, cmd->text.pos.x, cmd->text.pos.y, cmd->text.str, NULL);
            //nvgRestore(ctx);
        }
    }
    nvgEndFrame(ctx);
    nvgRestore(ctx);
    cursorWorld->Render(itsView);
}


void CGUI::Dispose() {
    delete this;
}
