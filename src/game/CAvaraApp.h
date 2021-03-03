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
#include "CLevelWindow.h"
#include "CNetworkWindow.h"
#include "CServerWindow.h"
#include "CPlayerWindow.h"
#include "CRosterWindow.h"
#include "CTrackerWindow.h"

#include <SDL2/SDL.h>
#include <string>
#include <deque>
#include <json.hpp>
#include <thread>
#include <mutex>

using json = nlohmann::json;


class CAvaraGame;
class CNetManager;

class CAvaraApp {
public:
    virtual bool DoCommand(int theCommand) = 0;
    virtual void GameStarted(std::string set, std::string level) = 0;
    virtual void MessageLine(short index, short align) = 0;
    virtual void AddMessageLine(std::string line) = 0;
    virtual std::deque<std::string>& MessageLines() = 0;
    virtual void DrawUserInfoPart(short i, short partList) = 0;
    virtual void ParamLine(short index, short align, StringPtr param1, StringPtr param2) = 0;
    virtual void StartFrame(long frameNum) = 0;
    virtual void BrightBox(long frameNum, short position) = 0;
    virtual void LevelReset() = 0;
    virtual long Number(const std::string name) = 0;
    virtual bool Boolean(const std::string name) = 0;
    virtual OSErr LoadLevel(std::string set, OSType theLevel, CPlayerManager *sendingPlayer) = 0;
    virtual OSErr LoadSVGLevel(std::string set, OSType theLevel) = 0;
    virtual void ComposeParamLine(StringPtr destStr, short index, StringPtr param1, StringPtr param2) = 0;
    virtual void NotifyUser() = 0;
    virtual json Get(const std::string name) = 0;
    virtual void Set(const std::string name, const std::string value) = 0;
    virtual void Set(const std::string name, long value) = 0;
    virtual void Set(const std::string name, json value) = 0;
    virtual CNetManager* GetNet() = 0;
    virtual void SetNet(CNetManager*) = 0;
    virtual SDL_Window* sdlWindow() = 0;
    virtual void StringLine(StringPtr theString, short align) = 0;
    virtual CAvaraGame* GetGame() = 0;
    virtual void Done() = 0;
    virtual void BroadcastCommand(int theCommand) = 0;
};
class CAvaraAppImpl : public CApplication, public CAvaraApp {
private:
    CAvaraGame *itsGame;
    CNetManager *gameNet;

public:
    CPlayerWindow *playerWindow;
    CLevelWindow *levelWindow;
    CNetworkWindow *networkWindow;
    CServerWindow *serverWindow;
    CRosterWindow *rosterWindow;
    CTrackerWindow *trackerWindow;

    std::deque<std::string> messageLines;

    CAvaraAppImpl();
    ~CAvaraAppImpl();

    long nextTrackerUpdate;
    json trackerState;
    bool trackerUpdatePending;
    std::thread *trackerThread;

    virtual std::deque<std::string>& MessageLines() override;
    virtual void idle() override;
    virtual void drawContents() override;

    virtual bool DoCommand(int theCommand) override;
    virtual void WindowResized(int width, int height) override;

    virtual void Done() override;

    virtual bool handleSDLEvent(SDL_Event &event) override;
    virtual void drawAll() override;
    OSErr LoadSVGLevel(std::string set, OSType theLevel) override;
    OSErr LoadLevel(std::string set, OSType theLevel, CPlayerManager *sendingPlayer) override;
    void NotifyUser() override;
    virtual void AddMessageLine(std::string line) override;
    virtual void GameStarted(std::string set, std::string level) override;

    // From CInfoPanel
    virtual void SetIndicatorDisplay(short i, short v);
    virtual void NumberLine(long theNum, short align);
    virtual void DrawUserInfoPart(short i, short partList) override;
    virtual void BrightBox(long frameNum, short position) override;
    virtual void MessageLine(short index, short align) override;
    virtual void LevelReset() override;
    virtual void ParamLine(short index, short align, StringPtr param1, StringPtr param2) override;
    virtual void StartFrame(long frameNum) override;
    virtual void StringLine(StringPtr theString, short align) override;
    virtual void ComposeParamLine(StringPtr destStr, short index, StringPtr param1, StringPtr param2) override;
    virtual void SetNet(CNetManager*) override;
    virtual CNetManager* GetNet() override;
    virtual CAvaraGame* GetGame() override;
    virtual void BroadcastCommand(int theCommand) override { CApplication::BroadcastCommand(theCommand); }
    virtual json Get(const std::string name) override { return CApplication::Get(name); }
    virtual void Set(const std::string name, const std::string value) override { CApplication::Set(name, value); }
    virtual void Set(const std::string name, long value) override { CApplication::Set(name, value); }
    virtual void Set(const std::string name, json value) override { CApplication::Set(name, value); }
    virtual SDL_Window* sdlWindow() override { return CApplication::sdlWindow(); }
    virtual long Number(const std::string name) override { return CApplication::Number(name); }
    virtual bool Boolean(const std::string name) override { return CApplication::Boolean(name); }

    void TrackerUpdate();
    std::string TrackerPayload();
};
