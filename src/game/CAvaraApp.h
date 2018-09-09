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
#include "CPlayerWindow.h"

#include <SDL2/SDL.h>
#include <string>

class CAvaraApp : public CApplication {
public:
    class CAvaraGame *itsGame;
    class CNetManager *gameNet;

    CPlayerWindow *playerWindow;
    CLevelWindow *levelWindow;
    CNetworkWindow *networkWindow;

    CAvaraApp();
    ~CAvaraApp();

    virtual void Idle();
    virtual void DrawContents();
    virtual bool DoCommand(int theCommand);
    virtual void WindowResized(int width, int height);

    virtual bool handleSDLEvent(SDL_Event &event);
    virtual void drawAll();

    OSErr LoadLevel(std::string set, OSType theLevel);

    // From CInfoPanel
    virtual void SetIndicatorDisplay(short i, short v);
    virtual void NumberLine(long theNum, short align);
    virtual void DrawUserInfoPart(short i, short partList);
    virtual void BrightBox(long frameNum, short position);
    virtual void MessageLine(short index, short align);
    virtual void LevelReset();
    virtual void ParamLine(short index, short align, StringPtr param1, StringPtr param2);
    virtual void StartFrame(long frameNum);
    virtual void StringLine(StringPtr theString, short align);
    virtual void ComposeParamLine(StringPtr destStr, short index, StringPtr param1, StringPtr param2);
};
