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
#include <json.hpp>
#include "httplib.h"

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
    started = SDL_GetTicks();
    anim_timer = 0;
    targetScreen = GUIScreen::Options;
    //SDL_StartTextInput();
    FetchRecents();
}

void CGUI::FetchRecents() {
    recentSets.clear();
    recentLevels.clear();
    CRUD::RecentLevelsList recents = itsApp->itsAPI->GetRecentLevels(MAX_RECENTS);
    for (auto recent : recents) {
        recentSets.push_back(recent.setTag);
        recentLevels.push_back(recent.levelName);
    }
}

std::string CGUI::PlayerStringStatus(CPlayerManager *player) {
    std::string strStatus;
    if (player->WinFrame() >= 0) {
        long timeTemp = FMulDiv(player->WinFrame(), itsGame->frameTime, 10);
        auto hundreds1 = timeTemp % 10;
        timeTemp /= 10;
        auto hundreds2 = timeTemp % 10;
        timeTemp /= 10;
        auto secs1 = timeTemp % 10;
        timeTemp /= 10;
        auto secs2 = timeTemp % 6;
        timeTemp /= 6;

        std::ostringstream os;
        os << "[" << timeTemp << ":" << secs2 << secs1 << "." << hundreds2 << hundreds1 << "]";

        strStatus = os.str();
        return strStatus;
    }

    LoadingState status = player->LoadingStatus();
    PresenceType presence = player->Presence();
    if (presence != kzAway) {
        if (status == kLConnected) {
            strStatus = "connected";
        } else if (status == kLLoaded) {
            strStatus = "loaded";
        } else if (status == kLReady) {
            strStatus = "ready";
        } else if (status == kLWaiting) {
            strStatus = "waiting";
        } else if (status == kLTrying) {
            strStatus = "loading";
        } else if (status == kLMismatch) {
            strStatus = "version mismatch";
        } else if (status == kLNotFound) {
            strStatus = "level not found";
        } else if (status == kLPaused) {
            strStatus = "paused";
        } else if (status == kLActive) {
            strStatus = "active";
        } else if (status == kLNoVehicle) {
            strStatus = "HECTOR not available";
        } else {
            strStatus = "";
        }
    } else if (status != kLNotConnected) {
        strStatus = "away";
    }
    if (presence == kzSpectating) {
        if (player->LoadingStatusIsIn(kLConnected, kLActive, kLReady, kLLoaded, kLPaused)) {
            strStatus = "spectator";
        } else if (strStatus.length() > 0) {
            strStatus += "*";   // make this into an eyeball char?
        }
    }
    return strStatus;
}

void CGUI::TrackerQuery() {
    servers.clear();
    std::string address = gApplication->String(kTrackerAddress);
    httplib::Client client(address.c_str(), 80);
    auto resp = client.Get("/api/v1/games/");
    if (resp && resp->status == 200) {
        json apiData = json::parse(resp->body);
        auto serverCount = apiData["games"].size();
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

            bool password = false;
            if(! game["password"].is_null())
                 password = game["password"].get<bool>();
            ServerInfo s;
            s.address = game["address"].get<std::string>();
            s.players = players;
            s.description = game["description"].get<std::string>();
            s.password = password;
            s.index = i;
            servers.push_back(s);
        }

        serverSummary = std::to_string(serverCount) + " server";
        if(serverCount != 1)
            serverSummary.append("s");
    }
}

void CGUI::Resized() {
    state = STATE_CHANGETO(_transitionScreen);
}

void CGUI::Update() {
    last_t = t;
    t = SDL_GetTicks();
    dt = t - last_t;
    anim_timer += dt;
    if (anim_timer > itsGame->frameTime) {
        //itsGame->itsDepot->RunSliverActions();
        anim_timer = 0;
    }
    unit_x = itsApp->fb_size_x / 8;
    unit_y = itsApp->fb_size_y / 6;
    pad = unit_y / 10;
    prevFocus[0] = focus[0];
    prevFocus[1] = focus[1];
    state = state();
    if (prevFocus[0] != focus[0] && prevFocus[1] != focus[1] && focus[0] != -1 && focus[1] != -1)
        PlaySound(kFootStepSound);
}

GUIItem *CGUI::GetFocused() {
    for (auto it = currentItems.begin(); it != currentItems.end(); ++it) {
        if ((*it).focus) {
            return &(*it);
        }
    }
    return NULL;
}

void CGUI::Select() {
    GUIItem* focus = GetFocused();
    if (focus && focus->action) focus->action();
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

bool CGUI::handleSDLEvent(SDL_Event &event) {
    if (!active) return false;
    switch(event.type) {
        case SDL_MOUSEMOTION: {
            cursor_x = event.motion.x * itsApp->pixel_ratio;
            cursor_y = event.motion.y * itsApp->pixel_ratio;
            return true;
        }
        /*
        case SDL_MOUSEWHEEL: {
            return true;
        }
        case SDL_TEXTINPUT: {
            return true;
        }
        case SDL_KEYDOWN:
        case SDL_KEYUP: {
            return true;
        }
         
        */
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP: {
            int b = event.button.button;
            if (b && event.type == SDL_MOUSEBUTTONDOWN) {
                Select();
            }
            return true;
        }
    }
    return false;
}


StateFunction CGUI::_startup() {
    PlaySound(kIncarnSound);
    return STATE_CHANGETO(_transitionScreen);
}

GUIItem CGUI::ItemDefaults(short ord_x, short ord_y, NVGrect r, std::function<void()>action) {
    GUIItem w;
    w.rect = r;
    w.ord_x = ord_x;
    w.ord_y = ord_y;
    w.focus = false;
    w.action = action;
    w.color = nvgRGBA(20, 20, 20, 160);
    w.textColor = invertColor(w.color);
    return w;
}

void CGUI::Button(const std::string &text, short ord_x, short ord_y, NVGrect r, std::function<void()> action) {
    GUIItem w = ItemDefaults(ord_x, ord_y, r, action);
    w.itemType = GUIItemType::Button;
    w.interactable = true;
    w.text = text;
    currentItems.push_back(w);
}

void CGUI::TextInput(const std::string &text, short ord_x, short ord_y, NVGrect r, std::function<void()> action) {
    GUIItem w = ItemDefaults(ord_x, ord_y, r, action);
    w.itemType = GUIItemType::TextInput;
    w.interactable = true;
    w.text = text;
    currentItems.push_back(w);
}

void CGUI::Pane(NVGrect r) {
    GUIItem p;
    p.itemType = GUIItemType::Pane;
    p.rect = r;
    currentItems.push_back(p);
}

void CGUI::BigButton(const std::string &text, short index, GUIScreen target) {
    BigButton(text, index, [this, target]() {
        targetScreen = target;
    });
}

void CGUI::BigButton(const std::string &text, short index, std::function<void()> action) {
    NVGrect r;
    r.x = unit_x + pad;
    r.y = ((unit_y / 2) + (unit_y * index)) + ((pad * 2) * index);
    r.w = (unit_x * 3) - pad * 2;
    r.h = (unit_y * .75f) - pad * 2;
    
    Button(text, 0, index, r, action);
}

void CGUI::BackButton(std::function<void()> action) {
    NVGrect r;
    r.x = pad;
    r.y = pad;
    r.w = unit_x / 4;
    r.h = r.w;
    std::string back_glyph = "<";
    
    Button(back_glyph, 0, 0, r, action);
}

void CGUI::JustTitleText(const std::string &text) {
    NVGrect r;
    r.x = (itsApp->fb_size_x / 2) - unit_x + pad;
    r.y = (pad * 2);
    r.w = (unit_x * 2) - pad * 2;
    r.h = (unit_y * .5) - pad * 2;
    JustText(text, r, true);
}

void CGUI::JustText(const std::string &text, NVGrect r) {
    JustText(text, r, false);
}

void CGUI::JustText(const std::string &text, NVGrect r, bool bg) {
    GUIItem w;
    w.itemType = GUIItemType::JustText;
    w.rect = r;
    w.text = text;
    if (bg) w.color = nvgRGB(25, 25, 25);
    else {
        w.color = nvgRGBA(0,0,0,0);
        w.textAlign = NVG_ALIGN_LEFT | NVG_ALIGN_CENTER;
    }
    w.textColor = nvgRGB(255, 255, 255);
    w.interactable = false;
    currentItems.push_back(w);
}

void CGUI::JustRect(NVGrect r) {
    JustRect(r, nvgRGBA(20, 20, 20, 160));
}

void CGUI::JustRect(NVGrect r, NVGcolor c) {
    GUIItem w;
    w.itemType = GUIItemType::JustRect;
    w.rect = r;
    w.color = c;
    w.interactable = false;
    currentItems.push_back(w);
}

void CGUI::JustLine(NVGrect r) {
    GUIItem w;
    w.itemType = GUIItemType::JustLine;
    w.rect = r;
    w.color = nvgRGBA(20, 20, 20, 160);
    w.interactable = false;
    currentItems.push_back(w);
}

void CGUI::KeyboardTab(NVGrect r) {
    json keyboardMap = itsApp->Get(kKeyboardMappingTag);
    short idx = 0;
    short secondCol = 0;
    for (auto &pair : keyboardMap.items()) {
        NVGrect keyr;
        keyr.x = r.x + pad;
        keyr.y = r.y + (pad) + (((unit_y / 4) + (pad * .25f)) * idx);
        keyr.h = unit_y / 4;
        keyr.w = r.w / 2;
        if (keyr.y + keyr.h > r.y + r.h) secondCol++;
        if (secondCol > 0) {
            keyr.x = r.x + (r.w / 2) + pad;
            keyr.y = r.y + (pad) + (((unit_y / 4) + (pad * .25f)) * (secondCol - 1));
        }
        
        NVGrect valr;
        valr.x = keyr.x + keyr.w - unit_x;
        valr.y = keyr.y;
        valr.h = unit_y / 4.5;
        valr.w = unit_x / 2;
        std::string action = pair.key();
        std::stringstream keylabel;
        if (pair.value().type() == json::value_t::array) {
            for (auto &key : pair.value()) {
                keylabel << key;
            }
        }
        else {
            keylabel << pair.value();
        }
        std::string keystr = keylabel.str();
        JustText(action, keyr);
        JustText(keystr, valr);
        idx++;
    }
}

void CGUI::KeyboardKeyControl(NVGrect r, nlohmann::json &keyConfigPair) {
    
}

void CGUI::OptionsTab(nlohmann::json &config, NVGrect r) {
    int idx = 0;
    int secondCol = 0;
    for (auto &opt : config.items()) {
        std::string label = opt.value()[0];
        auto tag = opt.value()[1];
        optionTypes o = opt.value()[2];
        NVGrect optr;
        optr.x = r.x + pad;
        optr.y = r.y + (pad * 2) + (((unit_y / 3) + pad) * idx);
        if (optr.y > r.y + r.h) secondCol++;
        if (secondCol > 0) {
            optr.x = r.x + (r.w / 2) + pad;
            optr.y = r.y + (pad * 2) + (((unit_y / 3) + pad) * (secondCol - 1));
        }
        optr.h = unit_y / 3;
        optr.w = r.w / 2;
        
        NVGrect valr;
        valr.x = optr.x + optr.w - unit_x;
        valr.y = optr.y;
        valr.h = unit_y / 3.5;
        valr.w = unit_x / 2;
        
        JustText(label, optr);
        switch (o) {
            case optionTypes::kOptionTypeFloat: {
                float pref = itsApp->Get(tag);
                std::string prefs = std::to_string(pref);
                Button(prefs, secondCol > 0 ? 1 : 2, idx, valr, [](){});
                break;
            }
            case optionTypes::kOptionTypeInteger:
            case optionTypes::kOptionTypeChoice:{
                json choices;
                if (opt.value().size() > 3)
                    choices = opt.value()[3];
                int pref = itsApp->Get(tag);
                std::string selected;
                for (auto &choice : choices.items()) {
                    if (pref == choice.value()[0])
                        selected = choice.value()[1];
                }
                std::string prefs = std::to_string(pref);
                Button(selected, 1, idx, valr, [this, opt](){
                    dropdown = opt;
                    STATE_CHANGETO(_dropDownMode);
                });
                break;
            }
            case optionTypes::kOptionTypeString:{
                std::string pref = itsApp->Get(tag);
                TextInput(pref, 1, idx, valr, [](){});
                break;
            }
            case optionTypes::kOptionTypeColor:{
                std::string pref = itsApp->Get(tag);
                auto color = ARGBColor::Parse(pref);
                if (color.has_value())
                    JustRect(valr, color.value().IntoNVG());
                JustText(pref, valr);
                break;
            }
            case optionTypes::kOptionTypeBool: {
                auto pref = itsApp->Get(tag);
                std::string val = pref ? "yes" : "no";
                Button(val, 1, idx, valr, [](){});
                break;
            }
            case optionTypes::kOptionTypeKeyboard:
                break;
        }
        idx++;
    }
}

StateFunction CGUI::_transitionScreen() {
    SDL_Log("_transitionScreen %d", targetScreen);
    currentItems.clear();
    switch (targetScreen) {
        case GUIScreen::MainMenu: {
            BigButton("Play Online", 0, GUIScreen::Tracker);
            BigButton("Single Player", 1, GUIScreen::Solo);
            BigButton("Options", 2, GUIScreen::Options);
            BigButton("Quit", 3, []() {
                pushQuit();
            });
        }
            break;
        case GUIScreen::Solo: {
            BackButton([this](){ targetScreen = GUIScreen::MainMenu; });
            JustTitleText("Select Level");
        }
            break;
        case GUIScreen::Tracker:
            break;
        case GUIScreen::HostGame:
            break;
        case GUIScreen::Server:
            break;
        case GUIScreen::Options: {
            BackButton([this](){ targetScreen = GUIScreen::MainMenu; });
            JustTitleText("Options");
            // tab rects
            Dim taby = unit_y * .5 + (pad * 2);
            Dim tabx = unit_x / 4 + (pad * 2);
            Dim tabw = itsApp->fb_size_x - tabx - (pad * 2);
            Dim tabcount = optionsScreens.size();
            int i = 0;
            for(auto &tab : optionsScreens.items()) {
                NVGrect tabr;
                tabr.x = tabx + ((tabw / tabcount) * i);
                tabr.y = taby;
                tabr.w = tabw / tabcount - pad;
                tabr.h = unit_y / 3;
                // hilite the active tab
                if (optionsTab == tab.key()) {
                    NVGrect hilite;
                    hilite.x = tabr.x + (pad * .25f);
                    hilite.y = tabr.y + tabr.h + (pad * .6f);
                    hilite.w = tabr.w - (pad * .75f);
                    hilite.h = pad * .75f;
                    JustLine(hilite);
                }
                std::string tab_str(tab.key());
                Button(tab_str, 0, i, tabr, [this, tab_str]() {
                    PlaySound(kFootStepSound);
                    // send us to another tab
                    optionsTab = tab_str;
                    // transition with no target updates everything
                    state = STATE_CHANGETO(_transitionScreen);
                });
                i++;
            }
            // large background rect
            NVGrect paner;
            paner.w = tabw - pad;
            paner.h = itsApp->fb_size_y - unit_y * 2;
            paner.x = tabx;
            paner.y = taby + unit_y / 3 + pad;
            Pane(paner);
            if (optionsTab == "Keyboard")
                KeyboardTab(paner);
            else OptionsTab(optionsScreens[optionsTab], paner);
            //BigButton("Keybinds", 2, Keybind);
        }
            break;
        case GUIScreen::Keybind:
        case GUIScreen::About:
        case GUIScreen::Test:
            break;
    }
    
    currentScreen = targetScreen;
    
    return STATE_CHANGETO(_drawScreen);
}

StateFunction CGUI::_drawScreen() {
    // poorly named, but basically, this state waits for input
    if (currentScreen != targetScreen) {
        PlaySound(kTeleSound);
        return STATE_CHANGETO(_transitionScreen);
    }
    bool any = false;
    for (auto it = currentItems.begin(); it != currentItems.end(); ++it) {
        if (!(*it).interactable) continue;
        if (isInRect(cursor_x, cursor_y, (*it).rect)) {
            (*it).focus = true;
            focus[0] = (*it).ord_x;
            focus[1] = (*it).ord_y;
            any = true;
        }
        else (*it).focus = false;
    }
    if (!any) {
        focus[0] = -1;
        focus[1] = -1;
    }
    return STATE_STAY;
}

StateFunction CGUI::_dropDownMode() {
    for (auto it = currentItems.begin(); it != currentItems.end(); ++it) {
        if ((*it).focus) {
            
        }
    }
    return STATE_STAY;
}

StateFunction CGUI::_textInputMode() {
    
    return STATE_STAY;
}

StateFunction CGUI::_test() {
    std::stringstream fps;
    fps << "frame in: " << dt << "ms";
    
    return STATE_STAY;
}

void CGUI::Render(NVGcontext *ctx) {
    if (!active) return;
    nvgBeginFrame(ctx, gApplication->fb_size_x, gApplication->fb_size_y, gApplication->pixel_ratio);
    nvgBeginPath(ctx);
    nvgFontFace(ctx, "mono");
    for (auto it = currentItems.begin(); it != currentItems.end(); ++it) {
        (*it).Draw(ctx);
    }
    CursorDebug(ctx);
    nvgEndFrame(ctx);
}


void CGUI::CursorDebug(NVGcontext *ctx) {
    Dim sz = pad * 2.4;
    NVGcolor c;
    c.r = c.g = c.b = 0;
    c.a = 255;
    nvgBeginPath(ctx);
    nvgStrokeColor(ctx, c);
    nvgMoveTo(ctx, cursor_x - sz, cursor_y);
    nvgLineTo(ctx, cursor_x + sz, cursor_y);
    nvgStrokeWidth(ctx, 2.0);
    nvgStroke(ctx);
    nvgClosePath(ctx);
    
    nvgBeginPath(ctx);
    nvgStrokeColor(ctx, c);
    nvgMoveTo(ctx, cursor_x, cursor_y - sz);
    nvgLineTo(ctx, cursor_x, cursor_y + sz);
    nvgStrokeWidth(ctx, 2.0);
    nvgStroke(ctx);
    nvgClosePath(ctx);
}
