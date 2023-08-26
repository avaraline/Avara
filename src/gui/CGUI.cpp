#include "CGUI.h"
#include "CApplication.h"
#include "CAvaraApp.h"
#include "CAvaraGame.h"
#include "CDepot.h"
#include "CNetManager.h"
#include "LevelLoader.h"
#include "CApplication.h"
#include "Preferences.h"
#include "CSoundHub.h"
#include "CSwitchActor.h"
#include <cstring>

std::string teststring = "hella";
#define TEXT_INPUT_TEMP_BUFFER_SIZE 1024

CGUI::CGUI(CAvaraAppImpl *app) {
    itsApp = app;
    itsGame = app->GetGame();
    itsWorld = new CBSPWorldImpl;
    itsWorld->IBSPWorld(50);
    itsView = itsGame->itsView;
    LookAtGUI();

    mui_ctx = (mu_Context*)malloc(sizeof(mu_Context));
    mu_init(mui_ctx);
    mui_ctx->text_width = text_width;
    mui_ctx->text_height = text_height;

    itsCursor = new CScaledBSP;
    itsCursor->IScaledBSP(FIX3(100), kCursorBSP, 0, 0);
    itsCursor->privateAmbient = FIX3(800);
    AvaraGLUpdateData(itsCursor);
    cursorWorld = new CBSPWorldImpl;
    cursorWorld->IBSPWorld(1);
    cursorWorld->AddPart(itsCursor);

    started = SDL_GetTicks();
    anim_timer = 0;

    SDL_StartTextInput();
}

void CGUI::LookAtGUI() {
    itsView->LookFrom(0, 0, FIX(1));
    itsView->LookAt(0, 0, 0);
    itsView->PointCamera();
}

void CGUI::Update() {
    last_t = t;
    t = SDL_GetTicks();
    dt = t - last_t;
    anim_timer += dt;
    int fb_size_x = gApplication->fb_size_x;
    int fb_size_y = gApplication->fb_size_y;
    mu_Rect screen = mu_rect(0, 0, fb_size_x, fb_size_x);
    mui_ctx->style->padding = 20;
    mu_begin(mui_ctx);
    int muiflags =0;
    const int row_widths[] = { (int)std::floor(fb_size_x / 5.0) };
    const int row_height = std::floor(fb_size_y / 9.0);
    if(mu_begin_window_ex(mui_ctx, "Main", screen, muiflags)) {
        mu_layout_row(mui_ctx, 1, row_widths, row_height);
        state = state();
        mu_end_window(mui_ctx);
    }
    mu_end(mui_ctx);

    if (anim_timer > 16) {
        itsGame->itsDepot->RunSliverActions();
        anim_timer = 0;
    }

}

int CGUI::BSPWidget(mu_Rect r, int res, mu_Id mu_id) {
    Point mid = pt(r.x + (r.w / 2), r.y + (r.h / 2));
    glm::vec3 worldpos = screenToWorld(&mid);

    if (actors.count(mu_id) > std::size_t(0)) {
        //CSmartBox* _part = boxes.at(mu_id);
        CAbstractActor* _wall = actors.at(mu_id);
        CSmartPart* _part = _wall->partList[0];
        _part->Reset();
        TranslatePart(_part, ToFixed(worldpos.x), ToFixed(worldpos.y), ToFixed(0));
        //_part->MoveDone();
    }
    else {
        Point s_topleft = pt(r.x, r.y);
        Point s_bottomright = pt(r.x + r.w, r.y + r.h);
        glm::vec3 ws_topleft = screenToWorld(&s_topleft);
        glm::vec3 ws_bottomright = screenToWorld(&s_bottomright);

        Vector dims;
        dims[0] = ToFixed((ws_bottomright.x - ws_topleft.x) / 2.0);
        dims[1] = ToFixed((ws_bottomright.y - ws_topleft.y) / 2.0);
        dims[2] = FIX3(1);

        //CSmartBox* _part = new CSmartBox;

        CWallActor *theWall = new CWallActor;
        theWall->IAbstractActor();
        theWall->MakeWallFromDims(dims, ToFixed(worldpos.x), ToFixed(worldpos.y), 0);
        long color = RGBAToLong(mui_ctx->style->colors[MU_COLOR_BASE]);

        theWall->partList[0]->ReplaceColor(0x00fefefe, color);
        //Vector _partLoc;
        //AvaraGLUpdateData(_part);
        //itsWorld->AddPart(_part);
        //_part->Reset();
        //TranslatePart(_part, ToFixed(worldpos.x), ToFixed(worldpos.y), 0);
        //_part->MoveDone();
        actors.emplace(mu_id, theWall);
    }
    return res;
}

int CGUI::BSPButton(std::string s) {
    int res = 0;
    mu_Id mu_id = mu_get_id(mui_ctx, s.c_str(), s.length());
    mu_Rect r = mu_layout_next(mui_ctx);
    mu_update_control(mui_ctx, mu_id, r, 0);
    BSPWidget(r, res, mu_id);
    CAbstractActor* _wall = actors.at(mu_id);
    CSmartPart* _part = _wall->partList[0];
    /* hover */
    if (mui_ctx->hover == mu_id) {
        long color = RGBAToLong(mui_ctx->style->colors[MU_COLOR_BUTTONHOVER]);
        _part->ReplaceColor(0xfefefe, color);
    }
    else {
        long color = RGBAToLong(mui_ctx->style->colors[MU_COLOR_BUTTON]);
        _part->ReplaceColor(0xfefefe, color);
    }
    /* handle click */
    if (mui_ctx->mouse_pressed == MU_MOUSE_LEFT && mui_ctx->focus == mu_id) {
        res |= MU_RES_SUBMIT;
        _wall->Blast();
    }
    /* draw */
    if (s.length() > 0) { mu_draw_control_text(mui_ctx, s.c_str(), r, MU_COLOR_TEXT, 0); }
    return res;
}

int CGUI::BSPTextInput(const char *id, std::string &s) {
    mu_Id mu_id = mu_get_id(mui_ctx, id, sizeof(id));
    mu_Rect r = mu_layout_next(mui_ctx);
    int res = 0;
    BSPWidget(r, res, mu_id);
    CAbstractActor* _wall = actors.at(mu_id);
    CSmartPart* _part = _wall->partList[0];

    // TODO: something different than this
    // can the mu_textbox use a std::string buffa somehow?
    const size_t len = TEXT_INPUT_TEMP_BUFFER_SIZE;
    char temp[len];
    std::strncpy(temp, s.c_str(), len);
    if (temp[len - 1] != '\0') {
        // overflow, truncating
        temp[len - 1] = '\0';
    }

    res |= mu_textbox_raw(mui_ctx, temp, len, mu_id, r, 0);
    // temp now contains updated string
    s.assign(temp);
    if (mui_ctx->focus == mu_id) {

        _part->ReplaceColor(0xfefefe, ColorManager::getEnergyGaugeColor());
    }
    else {
        long color = RGBAToLong(mui_ctx->style->colors[MU_COLOR_BASE]);
        _part->ReplaceColor(0x00fefefe, color);
    }
    return res;
}

int CGUI::BSPCheckbox(const char *label, int *state) {
    mu_Id mu_id = mu_get_id(mui_ctx, &state, sizeof(state));
    mu_Rect r = mu_layout_next(mui_ctx);
    mu_Rect box = mu_rect(r.x, r.y, r.h, r.h);
    mu_update_control(mui_ctx, mu_id, r, 0);

    Point s_topleft = pt(r.x, r.y);
    Point s_bottomright = pt(r.x + r.w, r.y + r.h);
    Point mid = pt(r.x + (r.w / 2), r.y + (r.h / 2));
    glm::vec3 worldpos = screenToWorld(&mid);
    glm::vec3 ws_topleft = screenToWorld(&s_topleft);
    glm::vec3 ws_bottomright = screenToWorld(&s_bottomright);
    int res = 0;
    if (mui_ctx->mouse_pressed == MU_MOUSE_LEFT && mui_ctx->focus == mu_id) {
        res |= MU_RES_CHANGE;
        *state = !*state;
    }

    if (actors.count(mu_id) > std::size_t(0)) {
        CAbstractActor* _wall = actors.at(mu_id);
        CSmartPart* _part = _wall->partList[0];
        _part->Reset();

        TranslatePart(_part, ToFixed(worldpos.x), ToFixed(worldpos.y), ToFixed(0));
    }
    else {
        Vector dims;
        dims[0] = ToFixed((ws_bottomright.x - ws_topleft.x) / 2.0);
        dims[1] = ToFixed((ws_bottomright.y - ws_topleft.y) / 2.0);
        dims[2] = FIX3(1);
        CSwitchActor *sw = new CSwitchActor;
        //sw->enabled = true;
        actors.emplace(mu_id, sw);
    }

    r = mu_rect(r.x + box.w, r.y, r.w - box.w, r.h);
    mu_draw_control_text(mui_ctx, label, r, MU_COLOR_TEXT, 0);
    return res;
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

static const char key_map[256] = {
  [ SDLK_LSHIFT       & 0xff ] = MU_KEY_SHIFT,
  [ SDLK_RSHIFT       & 0xff ] = MU_KEY_SHIFT,
  [ SDLK_LCTRL        & 0xff ] = MU_KEY_CTRL,
  [ SDLK_RCTRL        & 0xff ] = MU_KEY_CTRL,
  [ SDLK_LALT         & 0xff ] = MU_KEY_ALT,
  [ SDLK_RALT         & 0xff ] = MU_KEY_ALT,
  [ SDLK_RETURN       & 0xff ] = MU_KEY_RETURN,
  [ SDLK_BACKSPACE    & 0xff ] = MU_KEY_BACKSPACE,
};

bool CGUI::handleSDLEvent(SDL_Event &event) {
    if (!active) return false;
    switch(event.type) {
        case SDL_MOUSEMOTION: {
            mouse();
            mu_input_mousemove(mui_ctx, event.motion.x, event.motion.y);
            return true;
        }
        case SDL_MOUSEWHEEL: {
            mu_input_scroll(mui_ctx, 0, event.wheel.y * -30);
            return true;
        }
        case SDL_TEXTINPUT: {
            mu_input_text(mui_ctx, event.text.text);
            return true;
        }
        case SDL_KEYDOWN:
        case SDL_KEYUP: {
            int c = key_map[event.key.keysym.sym & 0xff];
            if (c && event.type == SDL_KEYDOWN) {
                mu_input_keydown(mui_ctx, c);
            }
            if (c && event.type == SDL_KEYUP) {
                mu_input_keyup(mui_ctx, c);
            }
            return true;
        }

        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP: {
            mouse();
            int b = event.button.button;
            if (b && event.type == SDL_MOUSEBUTTONDOWN) {
                mu_input_mousedown(mui_ctx, event.button.x, event.button.y, b);
            }
            if (b && event.type == SDL_MOUSEBUTTONUP) {
                mu_input_mouseup(mui_ctx, event.button.x, event.button.y, b);
            }
            return true;
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
    return CHANGETO(_transitionScreen);
}

StateFunction CGUI::_transitionScreen() {
    //itsGame->RunFrameActions();
    return STAY;
}

StateFunction CGUI::_test() {
    std::stringstream fps;
    fps << "frame in: " << dt << "ms";
    mu_label(mui_ctx, fps.str().c_str());
    if (BSPButton("PLAY")) {
        itsGame->SendStartCommand();
        active = false;
    }
    if (BSPButton("ABOUT")) {
        PlaySound(411);
    }
    if (BSPButton("QUIT")) {
        SDL_Event sdlevent;
        sdlevent.type = SDL_QUIT;
        SDL_PushEvent(&sdlevent);
    }
    const char* label = "A text input:";
    int w = text_width(0, label, strlen(label));
    mu_layout_row(mui_ctx, 2, (int[]) { w + 50, 400 }, 0);
    mu_label(mui_ctx, label);
    BSPTextInput("myinputid", teststring);

    //BSPCheckbox("A Checkbox", 0);
    return STAY;
}

void CGUI::Render(NVGcontext *ctx) {
    if (!active) return;
    //nvgSave(ctx);
    nvgBeginFrame(ctx, gApplication->win_size_x, gApplication->win_size_y, gApplication->pixel_ratio);
    nvgBeginPath(ctx);
    //LookAtGUI();
    itsWorld->Render(itsView);
    //screen->Render(ctx);

    mu_Command *cmd = NULL;
    while (mu_next_command(mui_ctx, &cmd)) {
        /*
        if (cmd->type == MU_COMMAND_RECT) {
            mu_Rect rect = cmd->rect.rect;
            NVGcolor c = toNVGcolor(cmd->rect.color);
            nvgBeginPath(ctx);
            nvgRect(ctx, cmd->rect.rect.x, cmd->rect.rect.y, cmd->rect.rect.w, cmd->rect.rect.h);
            nvgStrokeColor(ctx, c);
            nvgFillColor(ctx, c);
            nvgStroke(ctx);
            nvgFill(ctx);
            nvgClosePath(ctx);
        }
        */
        if (cmd->type == MU_COMMAND_TEXT) {
            nvgFontFace(ctx, "archivo");
            nvgTextAlign(ctx, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
            nvgFillColor(ctx, toNVGcolor(cmd->text.color));
            nvgFontSize(ctx, 55);
            nvgText(ctx, cmd->text.pos.x, cmd->text.pos.y, cmd->text.str, NULL);
        }
    }
    nvgEndFrame(ctx);
    cursorWorld->Render(itsView);
}

void CGUI::Dispose() {
    delete this;
}
