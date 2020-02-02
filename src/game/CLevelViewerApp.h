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
#include "CAvaraApp.h"
#include "CLevelWindow.h"
#include "CNetworkWindow.h"
#include "CPlayerWindow.h"
#include "CRosterWindow.h"
#include "CTrackerWindow.h"
#include "CViewParameters.h"

#include <SDL2/SDL.h>
#include <string>
#include <deque>

class CLevelViewerApp : public CApplication, public CAvaraApp {
private:
    CAvaraGame *itsGame;
    CNetManager *gameNet;
public:
    CLevelWindow *levelWindow;

    std::deque<std::string> messageLines;

    CLevelViewerApp();
    ~CLevelViewerApp();

    virtual std::deque<std::string>& MessageLines() override;
    virtual void idle() override;
    virtual void drawContents() override;

    virtual bool DoCommand(int theCommand) override;
    virtual void WindowResized(int width, int height) override;

    virtual void Done() override;

    virtual bool handleSDLEvent(SDL_Event &event) override;
    virtual void drawAll() override;

    OSErr LoadLevel(std::string set, OSType theLevel) override;
    void NotifyUser() override;
    virtual void AddMessageLine(std::string line);
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

};
