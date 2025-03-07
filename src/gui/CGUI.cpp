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
bool testbool = false;
#define TEXT_INPUT_TEMP_BUFFER_SIZE 1024

void pushQuit()
{
    SDL_Event ev;
    ev.type = SDL_QUIT;
    SDL_PushEvent(&ev);
}

CGUI::CGUI(CAvaraAppImpl *app) {
    itsApp = app;
    itsGame = app->GetGame();
    itsTui = app->GetTui();
    itsView = gRenderer->viewParams;
    itsLocalPlayer = CPlayerManagerImpl::LocalPlayer();
    LookAtGUI();

    mui_ctx = (mu_Context*)malloc(sizeof(mu_Context));
    mu_init(mui_ctx);
    mui_ctx->text_width = text_width;
    mui_ctx->text_height = text_height;
    itsCursor = std::make_unique<CScaledBSP>((Fixed)FIX3(100), (short)kCursorBSP, (CAbstractActor*)NULL, (short)0);
    itsCursor->privateAmbient = FIX3(800);
    //itsCursor->
    //AvaraGLUpdateData(itsCursor);
    gRenderer->AddPart(itsCursor.get());

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
    if (!active) return;
    last_t = t;
    t = SDL_GetTicks();
    dt = t - last_t;
    anim_timer += dt;
    int fb_size_x = gApplication->win_size_x;
    int fb_size_y = gApplication->win_size_y;
    mu_Rect screen = mu_rect(0, 0, fb_size_x, fb_size_x);
    mui_ctx->style->padding = 20;
    mu_begin(mui_ctx);
    int muiflags = 0;
    const int row_widths[] = { (int)std::floor(fb_size_x / 5.0) };
    const int row_height = std::floor(fb_size_y / 9.0);
    if(mu_begin_window_ex(mui_ctx, "Main", screen, muiflags)) {
        mu_layout_row(mui_ctx, 1, row_widths, row_height);
        
        // state function
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
    if (!active) return res;
    Point mid = pt(r.x + (r.w / 2), r.y + (r.h / 2));
    glm::vec3 worldpos = screenToWorld(&mid);

    if (actors.count(mu_id) > std::size_t(0)) {
        //CSmartBox* _part = boxes.at(mu_id);
        std::shared_ptr<CAbstractActor> _wall = actors.at(mu_id);
        //CBSPPart* _part = parts.at(mu_id);
        auto _part = _wall->partList[0];
        _part->Reset();
        TranslatePart(_part, ToFixed(worldpos.x), ToFixed(worldpos.y), 0);
        _part->MoveDone();
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
        
        
        ARGBColor color = RGBAToLong(mui_ctx->style->colors[MU_COLOR_BASE]);
        std::shared_ptr<CWallActor> theWall = std::make_shared<CWallActor>();
        theWall->MakeWallFromDims(dims, ToFixed(worldpos.x), ToFixed(worldpos.y), ToFixed(0.1f));
        auto theBox = theWall->partList[0];
        theBox->ReplaceColor(0x00fefefe, color);
        actors.emplace(mu_id, theWall);
        //boxes.emplace(mu_id, (CSmartBox*)theBox);
    }
    return res;
}

int CGUI::BSPButton(std::string s) {
    int res = 0;
    if (!active) return res;
    mu_Id mu_id = mu_get_id(mui_ctx, s.c_str(), s.length());
    mu_Rect r = mu_layout_next(mui_ctx);
    mu_update_control(mui_ctx, mu_id, r, 0);
    BSPWidget(r, res, mu_id);
    auto _wall = actors.at(mu_id);
    /* hover */
    if (mui_ctx->hover == mu_id) {
        ARGBColor color = RGBAToLong(mui_ctx->style->colors[MU_COLOR_BUTTONHOVER]);
        //_wall->partList[0]->ReplaceColor(ColorManager::getMarkerColor(0).value(), color);
    }
    else {
        ARGBColor color = RGBAToLong(mui_ctx->style->colors[MU_COLOR_BUTTON]);
        //_wall->partList[0]->ReplaceColor(ColorManager::getMarkerColor(0).value(), color);
        //gRenderer->r
    }
    /* handle click */
    if (mui_ctx->mouse_pressed == MU_MOUSE_LEFT && mui_ctx->focus == mu_id) {
        res |= MU_RES_SUBMIT;
        actors.at(mu_id)->Blast();
        return res;
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
    auto _wall = actors.at(mu_id);
    auto _part = _wall->partList[0];

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
    if (!_part) return res;
    if (mui_ctx->focus == mu_id) {
        //_part->ReplaceColor(ColorManager::getMarkerColor(0).value(), ColorManager::getEnergyGaugeColor());
    }
    else {
        ARGBColor color = RGBAToLong(mui_ctx->style->colors[MU_COLOR_BASE]);
        //_part->ReplaceColor(0x00fefefe, color);
    }
    return res;
}

int CGUI::BSPCheckbox(const char *label, bool *state) {
    mu_Id mu_id = mu_get_id(mui_ctx, &state, sizeof(state));
    mu_Rect r = mu_layout_next(mui_ctx);
    mu_Rect box = mu_rect(r.x, r.y, r.h, r.h);
    mu_update_control(mui_ctx, mu_id, r, 0);
    int res = 0;
    BSPWidget(r, res, mu_id);
    //std::shared_ptr<CWallActor> _wall = actors.at(mu_id);
    //CSmartPart* _part = _wall->partList[0];
    if (mui_ctx->mouse_pressed == MU_MOUSE_LEFT && mui_ctx->focus == mu_id) {
        res |= MU_RES_CHANGE;
        *state = !*state;
    }
    // TODO: fix marker, use hud colors, uhhh something other than yes/no
    if (*state) {
        mu_draw_control_text(mui_ctx, "yes", r, MU_COLOR_TEXT, 0);
    }
    else {
        mu_draw_control_text(mui_ctx, "no", r, MU_COLOR_TEXT, 0);
    }
    r = mu_rect(r.x + box.w, r.y, r.w - box.w, r.h);
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
    
    aSound = gHub->GetSoundSampler(hubRate, theSound);
    aSound->SetVolume(FIX(12));
    aLink = gHub->GetSoundLink();
    PlaceSoundLink(aLink, v);
    aSound->SetSoundLink(aLink);
    aSound->Start();
    gHub->ReleaseLink(aLink);
}

void CGUI::mouse(SDL_Event e) {
    //cursor_buttons = SDL_GetMouseState(&cursor_x, &cursor_y);
    cursor_x = e.motion.x;
    cursor_y = e.motion.y;

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
            mouse(event);
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
            mouse(event);
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

bool levelloaded = false;
StateFunction CGUI::_test() {
    std::stringstream fps;
    fps << "frame in: " << dt << "ms";
    mu_label(mui_ctx, fps.str().c_str());
    /*if (BSPButton("PLAY")) {
        itsGame->SendStartCommand();
        active = false;
    }
    return STAY;
    */
    /*else {
        if (BSPButton("LOAD")) {
            itsTui->ExecuteMatchingCommand("/rand avara aa emo ex", itsLocalPlayer);
            levelloaded = true;
        }
    }
    */
    if (BSPButton("ABOUT")) {
        PlaySound(411);
    }
    if (BSPButton("QUIT")) {
        SetActive(false);
        pushQuit();
    }
    
    const char* label = "A text input:";
    int w = text_width(0, label, (int)strlen(label));
    mu_layout_row(mui_ctx, 2, (int[]) { w + 50, 400 }, 0);
    mu_label(mui_ctx, label);
    BSPTextInput("myinputid", teststring);

    const char* label2 = "A checkbox:";
    w = text_width(0, label2, (int)strlen(label2));
    mu_layout_row(mui_ctx, 2, (int[]) { w + 50, 75 }, 0);
    mu_label(mui_ctx, label2);
    BSPCheckbox("checkboxid", &testbool);
    
    return STAY;
}

void CGUI::Render(NVGcontext *ctx) {
    if (!active) return;
    //nvgSave(ctx);
    nvgBeginFrame(ctx, gApplication->fb_size_x, gApplication->fb_size_y, gApplication->pixel_ratio);
    nvgBeginPath(ctx);

    nvgFontFace(ctx, "mono");
    nvgFontSize(ctx, 55);
    LookAtGUI();
    //itsWorld->Render(itsView);
    //screen->Render(ctx);

    mu_Command *cmd = NULL;
    while (mu_next_command(mui_ctx, &cmd)) {
        
        if (cmd->type == MU_COMMAND_RECT) {
            /*
            mu_Rect rect = cmd->rect.rect;
            NVGcolor c = toNVGcolor(cmd->rect.color);
            nvgBeginPath(ctx);
            nvgRect(ctx, cmd->rect.rect.x, cmd->rect.rect.y, cmd->rect.rect.w, cmd->rect.rect.h);
            nvgStrokeColor(ctx, c);
            nvgFillColor(ctx, c);
            nvgStroke(ctx);
            nvgFill(ctx);
            nvgClosePath(ctx);
            */
            
        }
        
        if (cmd->type == MU_COMMAND_TEXT) {
            nvgTextAlign(ctx, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
            nvgFillColor(ctx, toNVGcolor(cmd->text.color));
            nvgText(ctx, cmd->text.pos.x, cmd->text.pos.y, cmd->text.str, NULL);
        }
    }
    NVGcolor c = toNVGcolor(mui_ctx->style->colors[MU_COLOR_BASE]);
    nvgBeginPath(ctx);
    nvgStrokeColor(ctx, c);
    nvgMoveTo(ctx, cursor_x, 0);
    nvgLineTo(ctx, cursor_x, gApplication->fb_size_y);
    nvgStroke(ctx);
    nvgClosePath(ctx);
    
    nvgBeginPath(ctx);
    nvgStrokeColor(ctx, c);
    nvgMoveTo(ctx, 0, cursor_y);
    nvgLineTo(ctx, gApplication->fb_size_x, cursor_y);
    nvgStroke(ctx);
    nvgClosePath(ctx);
    
    nvgEndFrame(ctx);
    //cursorWorld->Render(itsView);
}

void CGUI::Dispose() {
    delete this;
}
