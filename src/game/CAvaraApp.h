/*
    Copyright ©1994-1996, Juri Munkki
    All rights reserved.

    File: CAvaraApp.h
    Created: Wednesday, November 16, 1994, 01:25
    Modified: Monday, September 2, 1996, 10:23
*/

#pragma once

#include "CApplication.h"
//#include "PolyColor.h"
#include "CHUD.h"
#include "CLevelWindow.h"
#include "CNetworkWindow.h"
#include "CServerWindow.h"
#include "CPlayerWindow.h"
#include "CRosterWindow.h"
#include "CTrackerWindow.h"
#include "CommandManager.h"
#include "Messages.h"
#include "CRUD.h"

#include <SDL2/SDL.h>
#include <string>
#include <deque>
#include <json.hpp>
#include <thread>
#include <mutex>
#include <iterator>
#include <memory>

using json = nlohmann::json;

struct ControllerAxis {
    float last;
    float value;
    uint8_t active : 1;
    uint8_t last_active : 1;
    uint8_t toggled : 1;
};

struct ControllerStick {
    uint16_t clamp_inner;
    uint16_t clamp_outer;
    uint32_t elapsed;
    ControllerAxis x;
    ControllerAxis y;
};

struct ControllerTrigger {
    uint16_t clamp_low;
    uint16_t clamp_high;
    uint32_t elapsed;
    ControllerAxis t;
};

struct ControllerSticks {
    ControllerStick left;
    ControllerStick right;
};

struct ControllerTriggers {
    ControllerTrigger left;
    ControllerTrigger right;
};

class CAvaraGame;
class CNetManager;

class CAvaraApp {
public:
    virtual bool DoCommand(int theCommand) = 0;
    virtual void GameStarted(LevelInfo &loadedLevel) = 0;
    virtual void MessageLine(short index, MsgAlignment align) = 0;
    virtual void AddMessageLine(std::string lines, MsgAlignment align = MsgAlignment::Left, MsgCategory category = MsgCategory::System) = 0;
    virtual void RenderContents() = 0;
    virtual std::deque<MsgLine>& MessageLines() = 0;
    virtual void DrawUserInfoPart(short i, short partList) = 0;
    virtual void ParamLine(short index, MsgAlignment align, StringPtr param1, StringPtr param2 = NULL) = 0;
    virtual void StartFrame(FrameNumber frameNum) = 0;
    virtual void BrightBox(FrameNumber frameNum, short position) = 0;
    virtual void LevelReset() = 0;
    virtual long Number(const std::string name) = 0;
    virtual bool Boolean(const std::string name) = 0;
    virtual OSErr LoadLevel(std::string set, std::string leveltag, CPlayerManager *sendingPlayer) = 0;
    virtual void ComposeParamLine(StringPtr destStr, short index, StringPtr param1, StringPtr param2) = 0;
    virtual void NotifyUser() = 0;
    virtual json Get(const std::string name) = 0;
    virtual void Set(const std::string name, const std::string value) = 0;
    virtual void Set(const std::string name, long value) = 0;
    virtual void Set(const std::string name, json value) = 0;
    virtual CNetManager* GetNet() = 0;
    virtual void SetNet(CNetManager*) = 0;
    virtual SDL_Window* sdlWindow() = 0;
    virtual void StringLine(std::string theString, MsgAlignment align) = 0;
    virtual CAvaraGame* GetGame() = 0;
    virtual void Done() = 0;
    virtual void BroadcastCommand(int theCommand) = 0;
    virtual CommandManager* GetTui() = 0;
    virtual uint32_t ControllerEventType() = 0;
    virtual void Rumble(Fixed hitEnergy) = 0;
};
class CAvaraAppImpl : public CApplication, public CAvaraApp {
private:
    std::unique_ptr<CAvaraGame> itsGame;
    
    CNetManager *gameNet;
    CommandManager *itsTui;
    std::unique_ptr<CHUD> ui;
    
public:
    std::unique_ptr<CRUD> itsAPI;
    CPlayerWindow *playerWindow;
    CLevelWindow *levelWindow;
    CNetworkWindow *networkWindow;
    CServerWindow *serverWindow;
    CRosterWindow *rosterWindow;
    CTrackerWindow *trackerWindow;
    
    SDL_GameController *controller; // currently paired controller
    uint32_t controllerBaseEvent; // registered with SDL_RegisterEvents
    uint32_t lastControllerEvent; // time of last controller axis polling
    uint32_t controllerPollMillis;
    
    ControllerSticks sticks;
    ControllerTriggers triggers;

    std::deque<MsgLine> messageLines;
    Fixed previewAngle, previewRadius;
    bool animatePreview;
    
    CAvaraAppImpl();
    ~CAvaraAppImpl();
    
    long nextTrackerUpdate;
    json trackerState;
    bool trackerUpdatePending;
    std::thread *trackerThread;
    
    virtual std::deque<MsgLine>& MessageLines() override;
    virtual void idle() override;
    virtual void drawContents() override;
    virtual void RenderContents() override;
    
    virtual bool DoCommand(int theCommand) override;
    virtual void WindowResized(int width, int height) override;
    virtual void PrefChanged(std::string name) override;

    virtual void Done() override;
    
    virtual bool handleSDLEvent(SDL_Event &event) override;
    virtual void drawAll() override;
    OSErr LoadLevel(std::string set, std::string levelTag, CPlayerManager *sendingPlayer) override;
    virtual void NotifyUser() override;
    
    virtual void AddMessageLine(std::string lines, MsgAlignment align = MsgAlignment::Left, MsgCategory category = MsgCategory::System) override;
    virtual void GameStarted(LevelInfo &loadedLevel) override;

    // From CInfoPanel
    virtual void SetIndicatorDisplay(short i, short v);
    virtual void NumberLine(long theNum, short align);
    virtual void DrawUserInfoPart(short i, short partList) override;
    virtual void BrightBox(FrameNumber frameNum, short position) override;
    virtual void MessageLine(short index, MsgAlignment align) override;
    virtual void LevelReset() override;
    virtual void ParamLine(short index, MsgAlignment align, StringPtr param1, StringPtr param2) override;
    virtual void StartFrame(FrameNumber frameNum) override;
    virtual void StringLine(std::string theString, MsgAlignment align) override;
    virtual void ComposeParamLine(StringPtr destStr, short index, StringPtr param1, StringPtr param2) override;
    virtual void SetNet(CNetManager*) override;
    virtual CNetManager* GetNet() override;
    virtual CAvaraGame* GetGame() override;
    virtual CommandManager* GetTui() override;
    
    virtual void BroadcastCommand(int theCommand) override { CApplication::BroadcastCommand(theCommand); }
    virtual json Get(const std::string name) override { return CApplication::Get(name); }
    virtual void Set(const std::string name, const std::string value) override { CApplication::Set(name, value); }
    virtual void Set(const std::string name, long value) override { CApplication::Set(name, value); }
    virtual void Set(const std::string name, json value) override { CApplication::Set(name, value); }
    virtual SDL_Window* sdlWindow() override { return CApplication::sdlWindow(); }
    virtual long Number(const std::string name) override { return CApplication::Number(name); }
    virtual bool Boolean(const std::string name) override { return CApplication::Boolean(name); }
    template <class T> T Get(const std::string name) { return CApplication::Get<T>(name); }
    
    void TrackerUpdate();
    std::string TrackerPayload();
    
    virtual uint32_t ControllerEventType() override { return controllerBaseEvent; }
    virtual void Rumble(Fixed hitEnergy) override;
};
