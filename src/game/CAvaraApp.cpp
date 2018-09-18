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

CCAvaraApp::CCAvaraApp() : CApplication("Avara") {
    itsGame = new CAvaraGame(8);
    gCurrentGame = itsGame;
    itsGame->IAvaraGame(this);
    itsGame->UpdateViewRect(mSize.x, mSize.y, mPixelRatio);

    gameNet->ChangeNet(kNullNet, "");

    setLayout(new nanogui::FlowLayout(nanogui::Orientation::Vertical, true, 20, 20));

    playerWindow = new CPlayerWindow(this);
    playerWindow->setFixedWidth(200);

    levelWindow = new CLevelWindow(this);
    levelWindow->setFixedWidth(200);

    networkWindow = new CNetworkWindow(this);
    networkWindow->setFixedWidth(200);

    trackerWindow = new CTrackerWindow(this);
    trackerWindow->setFixedWidth(300);

    rosterWindow = new CRosterWindow(this);
    rosterWindow->setFixedWidth(450);

    performLayout();
}

CCAvaraApp::~CCAvaraApp() {
    itsGame->Dispose();
    DeallocParser();
}

void CCAvaraApp::Done() {
    // This will trigger a clean disconnect if connected.
    gameNet->ChangeNet(kNullNet, "");
    CApplication::Done();
}

void CCAvaraApp::idle() {
    CheckSockets();
    if(itsGame->GameTick()) {
        glClearColor(mBackground[0], mBackground[1], mBackground[2], mBackground[3]);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        drawContents();
        SDL_GL_SwapWindow(mSDLWindow);
    }
}

void CCAvaraApp::drawContents() {
    itsGame->Render(mNVGContext);
}

void CCAvaraApp::WindowResized(int width, int height) {
    itsGame->UpdateViewRect(width, height, mPixelRatio);
    //performLayout();
}

bool CCAvaraApp::handleSDLEvent(SDL_Event &event) {
    if(itsGame->IsPlaying()) {
        itsGame->HandleEvent(event);
        return true;
    }
    else {
        if (rosterWindow->handleSDLEvent(event)) return true;
        return CApplication::handleSDLEvent(event);
    }
}

void CCAvaraApp::drawAll() {
    if (!itsGame->IsPlaying()) {
        rosterWindow->UpdateRoster();
        CApplication::drawAll();
    }
}

bool CCAvaraApp::DoCommand(int theCommand) {
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

OSErr CCAvaraApp::LoadLevel(std::string set, OSType theLevel) {
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
        Fixed pt[3];
        itsGame->itsWorld->OverheadPoint(pt);
        SDL_Log("overhead %f, %f, %f\n", ToFloat(pt[0]), ToFloat(pt[1]), ToFloat(pt[2]));
        itsGame->itsView->yonBound = FIX(10000);
        itsGame->itsView->LookFrom(pt[0] + FIX(100), pt[1] + FIX(200), pt[2] + FIX(150));
        itsGame->itsView->LookAt(pt[0], pt[1], pt[2]);
        itsGame->itsView->PointCamera();
    }

    return noErr;
}

void CCAvaraApp::NotifyUser() {
    // TODO: Bell sound(s)
}

CAvaraGame* CCAvaraApp::GetGame() {
    return itsGame;
}

CNetManager* CCAvaraApp::GetNet() {
    return gameNet;
}

void CCAvaraApp::SetNet(CNetManager *theNet) {
    gameNet = theNet;
}

// STUBBBBBZZZZZ

void CCAvaraApp::SetIndicatorDisplay(short i, short v) {}
void CCAvaraApp::NumberLine(long theNum, short align) {}
void CCAvaraApp::DrawUserInfoPart(short i, short partList) {}
void CCAvaraApp::BrightBox(long frameNum, short position) {}
void CCAvaraApp::MessageLine(short index, short align) {
    SDL_Log("CCAvaraApp::MessageLine(%d)\n", index);
}
void CCAvaraApp::LevelReset() {}
void CCAvaraApp::ParamLine(short index, short align, StringPtr param1, StringPtr param2) {
    SDL_Log("CCAvaraApp::ParamLine(%d)\n", index);
}
void CCAvaraApp::StartFrame(long frameNum) {}
void CCAvaraApp::StringLine(StringPtr theString, short align) {}
void CCAvaraApp::ComposeParamLine(StringPtr destStr, short index, StringPtr param1, StringPtr param2) {}
