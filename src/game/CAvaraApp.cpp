/*
    Copyright ©1994-1996, Juri Munkki
    All rights reserved.

    File: CAvaraApp.c
    Created: Wednesday, November 16, 1994, 01:26
    Modified: Friday, September 20, 1996, 02:05
*/

#define MAINAVARAAPP

#include "CAvaraApp.h"

#include "AvaraGL.h"
#include "AvaraScoreInterface.h"
#include "AvaraTCP.h"
#include "CAvaraGame.h"
#include "CBSPWorld.h"
#include "CCompactTagBase.h"
#include "CLevelDescriptor.h"
#include "CNetManager.h"
#include "CRC.h"
#include "CSoundMixer.h"
#include "CViewParameters.h"
#include "CommandList.h"
#include "KeyFuncs.h"
#include "LevelLoader.h"
#include "Parser.h"
#include "Preferences.h"
#include "Resource.h"
#include "System.h"

// included while we fake things out
#include "CPlayerManager.h"

CAvaraApp::CAvaraApp() : CApplication("Avara", 1024, 640) {
    setLayout(new nanogui::BoxLayout(nanogui::Orientation::Vertical, nanogui::Alignment::Fill, 20, 20));

    playerWindow = new CPlayerWindow(this);
    playerWindow->setFixedWidth(200);

    levelWindow = new CLevelWindow(this);
    levelWindow->setFixedWidth(200);

    networkWindow = new CNetworkWindow(this);
    networkWindow->setFixedWidth(200);

    itsGame = new CAvaraGame;
    gCurrentGame = itsGame;
    itsGame->IAvaraGame(this);
    itsGame->UpdateViewRect(mSize.x, mSize.y, mPixelRatio);

    gameNet->ChangeNet(kNullNet, "");

    /*
        if(theNet->netOwner == this && theNet->PermissionQuery(kAllowLoadBit, 0))
        {   theNet->SendLoadLevel(theTag);
            DoCommand(kShowRosterWind);
        }
    */

    rosterWindow = new CRosterWindow(this);
    rosterWindow->setFixedWidth(450);
    performLayout();

    rosterWindow->setPosition(nanogui::Vector2i(240,20));
}

CAvaraApp::~CAvaraApp() {
    itsGame->Dispose();
    DeallocParser();
}

void CAvaraApp::Done() {
    // This will trigger a clean disconnect if connected.
    gameNet->ChangeNet(kNullNet, "");
    CApplication::Done();
}

void CAvaraApp::Idle() {
    CheckSockets();
    if(itsGame->GameTick()) {
        glClearColor(mBackground[0], mBackground[1], mBackground[2], mBackground[3]);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        DrawContents();
        SDL_GL_SwapWindow(mSDLWindow);
    }
}

void CAvaraApp::DrawContents() {
    itsGame->Render(mNVGContext);
}

void CAvaraApp::WindowResized(int width, int height) {
    SDL_Log("CAvaraApp::WindowResized(%d, %d)\n", width, height);
    itsGame->UpdateViewRect(width, height, mPixelRatio);
}

bool CAvaraApp::handleSDLEvent(SDL_Event &event) {
    if(itsGame->IsPlaying()) {
        itsGame->HandleEvent(event);
        return true;
    }
    else {
        if (rosterWindow->handleSDLEvent(event)) return true;
        return CApplication::handleSDLEvent(event);
    }
}

void CAvaraApp::drawAll() {
    if (itsGame->IsPlaying()) {
        Idle();
    } else {
        //DrawContents();
        rosterWindow->UpdateRoster();
        CApplication::drawAll();
    }
}

bool CAvaraApp::DoCommand(int theCommand) {
    std::string name = String(kPlayerNameTag);
    Str255 userName;
    userName[0] = name.length();
    BlockMoveData(name.c_str(), userName + 1, name.length());
    SDL_Log("DoCommand %d\n", theCommand);
    switch (theCommand) {
        case kReportNameCmd:
            gameNet->NameChange(userName);
            return true;
            // break;
        case kStartGame:
        case kResumeGame:
            itsGame->SendStartCommand();
            return true;
            // break;
        case kGetReadyToStartCmd:
            break;
        default:
            break;
    }
    return false;
    /*
        default:
            if(	theCommand >= kLatencyToleranceZero &&
                theCommand <= kLatencyToleranceMax)
            {	WriteShortPref(kLatencyToleranceTag, theCommand - kLatencyToleranceZero);
            }
            else
            if(	theCommand >= kRetransmitMin &&
                theCommand <= kRetransmitMax)
            {	WriteShortPref(kUDPResendPrefTag, theCommand - kRetransmitMin);

                if(gameNet->itsCommManager)
                {	gameNet->itsCommManager->OptionCommand(theCommand);
                }
            }
            if(	theCommand >= kSlowestConnectionCmd && theCommand <= kFastestConnectionCmd)
            {	WriteShortPref(kUDPConnectionSpeedTag, theCommand - kSlowestConnectionCmd);

                if(gameNet->itsCommManager)
                {	gameNet->itsCommManager->OptionCommand(theCommand);
                }
            }
            else
                inherited::DoCommand(theCommand);
            break;
    }
    */
}

OSErr CAvaraApp::LoadLevel(std::string set, OSType theLevel) {
    SDL_Log("LOADING LEVEL %d FROM %s\n", theLevel, set.c_str());
    itsGame->LevelReset(false);
    itsGame->loadedTag = theLevel;
    gCurrentGame = itsGame;

    std::string rsrcFile = std::string("levels/") + set + ".r";
    UseResFile(rsrcFile);

    OSType setTag;
    CLevelDescriptor *levels = LoadLevelListFromResource(&setTag);
    CLevelDescriptor *curLevel = levels;
    bool wasLoaded = false;
    while (curLevel) {
        if (curLevel->tag == theLevel) {
            std::string rsrcName((char *)curLevel->access + 1, curLevel->access[0]);
            Handle levelData = GetNamedResource('PICT', rsrcName);
            if (levelData) {
                ConvertToLevelMap(levelData);
                ReleaseResource(levelData);
                wasLoaded = true;
            }
            break;
        }
        curLevel = curLevel->nextLevel;
    }
    levels->Dispose();

    if (wasLoaded) {
        CBSPPart *somePart = itsGame->itsWorld->GetIndPart(10);
        itsGame->itsView->LookFrom(0, FIX(100), FIX(-30));
        itsGame->itsView->LookAtPart(somePart);
        itsGame->itsView->PointCamera();
        // itsGame->Render();
    }

    return noErr;
}

void CAvaraApp::NotifyUser() {
    // TODO: Bell sound(s)
}

// STUBBBBBZZZZZ

void CAvaraApp::SetIndicatorDisplay(short i, short v) {}
void CAvaraApp::NumberLine(long theNum, short align) {}
void CAvaraApp::DrawUserInfoPart(short i, short partList) {}
void CAvaraApp::BrightBox(long frameNum, short position) {}
void CAvaraApp::MessageLine(short index, short align) {
    SDL_Log("CAvaraApp::MessageLine(%d)\n", index);
}
void CAvaraApp::LevelReset() {}
void CAvaraApp::ParamLine(short index, short align, StringPtr param1, StringPtr param2) {
    SDL_Log("CAvaraApp::ParamLine(%d)\n", index);
}
void CAvaraApp::StartFrame(long frameNum) {}
void CAvaraApp::StringLine(StringPtr theString, short align) {}
void CAvaraApp::ComposeParamLine(StringPtr destStr, short index, StringPtr param1, StringPtr param2) {}
