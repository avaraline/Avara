#pragma once
#include "ARGBColor.h"
#include "CommandManager.h"
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
#include "nanovg.h"

#include <SDL2/SDL.h>
#include <glm/glm.hpp>
extern "C" {
#include "microui.h"
}

#define kCursorBSP 801
#define kAvaraLogo 100
#define kBlockBSP 400
#define kOutlineBSP 722

class CAvaraAppImpl;

class CGUI {
public:
  CGUI(CAvaraAppImpl *app);
  bool handleSDLEvent(SDL_Event &event);
  void Dispose();
  void Render(NVGcontext *ctx);
  void Update();
  void SetActive(bool a) { active = a; };
  void ClearParts() {
    actors.clear();
  }

  virtual ~CGUI() { ClearParts(); }

protected:
  CAvaraGame *itsGame;
  CAvaraAppImpl *itsApp;
  CommandManager *itsTui;
  CPlayerManager *itsLocalPlayer;
  void LookAtGUI();
  void PlaySound(short theSound);

  void mouse(SDL_Event event);
  void kb();
  void joystick();

  short targetScreen;

  StateFunction _startup();
  StateFunction _transitionScreen();
  StateFunction _test();

  StateFunction _inactive();
  StateFunction _active();
  StateFunction _textInput();
  StateFunction _modal();

  StateFunction state = std::bind(&CGUI::_test, this);
  void PlaceGUIPart(CBSPPart *_part, mu_Rect r);

  mu_Context *mui_ctx;
  std::map<mu_Id, std::shared_ptr<CAbstractActor>> actors;
  int BSPButton(std::string label);
  int BSPWidget(mu_Rect r, int res, mu_Id mu_id);
  int BSPTextInput(const char *id, std::string &s);
  int BSPCheckbox(const char *label, bool *state);
  CBSPWorldImpl *itsWorld;
  CBSPWorldImpl *cursorWorld;
  CViewParameters *itsView;
  std::unique_ptr<CScaledBSP> itsCursor;
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
    return gRenderer->ScreenSpaceToWorldSpace(&v);
  }
    
  const glm::vec3 windowToWorld(Point *p) {
    float normalized_x = (((float)p->h / (float)gApplication->win_size_x) * 2.0) - 1.0;
    float normalized_y = ((((float)p->v / (float)gApplication->win_size_y) * 2.0) - 1.0) * -1.0;
    glm::vec4 v = glm::vec4(normalized_x, normalized_y, 1.0, -1);
    return gRenderer->ScreenSpaceToWorldSpace(&v);
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
    p.h = x * gApplication->pixel_ratio;
    p.v = y * gApplication->pixel_ratio;
    return p;
  }

  static const ARGBColor RGBAToLong(mu_Color c) {
      return ARGBColor(0xffffffff);
  }

  static const NVGcolor toNVGcolor(mu_Color other) {
    NVGcolor c;
    c.r = other.r / 255.0;
    c.g = other.g / 255.0;
    c.b = other.b / 255.0;
    c.a = other.a / 255.0;
    return c;
  }
};

enum GUIScreen { MainMenu, Solo, Tracker, HostGame, Server, Options, Keybind, About, Test };

// typedef std::function<void(CGUI *c)> GUICall;
