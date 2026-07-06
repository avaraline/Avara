#pragma once
#include "GUIItem.h"
#include "ARGBColor.h"
#include "AbstractRenderer.h"
#include "CApplication.h"
#include "CBSPPart.h"
#include "CBSPWorld.h"
#include "CDirectObject.h"
#include "CScaledBSP.h"
#include "CSmartBox.h"
#include "CStateFunction.hpp"
#include "CViewParameters.h"
#include "CWallActor.h"
#include "CommandManager.h"
#include "nanovg.h"

#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include "json.hpp"

#define kCursorBSP 801
#define kAvaraLogo 100
#define kBlockBSP 400
#define kOutlineBSP 722

#define kIncarnSound 411
#define kFootStepSound 160
#define kTeleSound 410

static int MAX_RECENTS = 50;

class CAvaraAppImpl;

enum GUIScreen { MainMenu, Solo, Tracker, HostGame, Server, Options, Keybind, About, Test };

struct ServerInfo {
    std::string address;
    std::string description;
    std::string players;
    bool password;
    short index;
};

class CGUI {
public:
    CGUI(CAvaraAppImpl *app);
    bool handleSDLEvent(SDL_Event &event);
    void Render(NVGcontext *ctx);
    void Resized();
    void Update();
    void SetActive(bool a) { active = a; };

    virtual ~CGUI() {}

protected:
    CAvaraGame *itsGame;
    CAvaraAppImpl *itsApp;
    CommandManager *itsTui;
    CPlayerManager *itsLocalPlayer;
    void LookAtGUI();
    void PlaySound(short theSound);
    
    std::vector<ServerInfo> servers;
    std::string serverSummary;
    
    Dim unit_x, unit_y, pad;
    
    GUIScreen currentScreen;
    GUIScreen targetScreen;
    std::string optionsTab = "Graphics";
    
    std::vector<std::string> recentSets;
    std::vector<std::string> recentLevels;
    void FetchRecents();
    std::string PlayerStringStatus(CPlayerManager *player);
    void TrackerQuery();
    
    std::vector<GUIItem> currentItems;

    StateFunction _startup(), _transitionScreen(), _drawScreen(), _test();
    StateFunction _textInputMode(), _dropDownMode();
    StateFunction _modal();

    StateFunction state = std::bind(&CGUI::_startup, this);

    CViewParameters *itsView;
    uint32_t cursor_buttons;
    int cursor_x;
    int cursor_y;
    bool active = true;
    bool selecting = false;
    short focus[2] = {-1, -1};
    short prevFocus[2] = {-1, -1};
    
    json dropdown;
    
    uint64_t started = 0;
    uint64_t t = 0;
    uint64_t last_t = 0;
    uint64_t dt = 0;
    uint16_t anim_timer = 0;
    
    void CursorDebug(NVGcontext *ctx);
    GUIItem ItemDefaults(short ord_x, short ord_y, NVGrect r, std::function<void()> action);
    void Button(const std::string &text, short ord_x, short ord_y, NVGrect r, std::function<void()> action);
    void TextInput(const std::string &text, short ord_x, short ord_y, NVGrect r, std::function<void()> action);
    void BigButton(const std::string &text, short index, GUIScreen target);
    void BigButton(const std::string &text, short index, std::function<void()> action);
    void BackButton(std::function<void()> action);
    void JustRect(NVGrect r);
    void JustRect(NVGrect r, NVGcolor c);
    void JustLine(NVGrect r);
    void JustText(const std::string &text, NVGrect r);
    void JustText(const std::string &text, NVGrect r, bool bg);
    void JustTitleText(const std::string &text);
    void Pane(NVGrect r);
    void OptionsTab(nlohmann::json &config, NVGrect r);
    void KeyboardTab(NVGrect r);
    void KeyboardKeyControl(NVGrect r, nlohmann::json &keyboardConfigPair);
    GUIItem* GetFocused();
    bool IsHover(NVGrect r);
    void Select();
};

// typedef std::function<void(CGUI *c)> GUICall;
