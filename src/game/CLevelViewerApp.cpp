/*
    Copyright ©1994-1996, Juri Munkki
    All rights reserved.

    File: CAvaraApp.c
    Created: Wednesday, November 16, 1994, 01:26
    Modified: Friday, September 20, 1996, 02:05
*/

#define MAINAVARAAPP

#include "CLevelViewerApp.h"

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
#include "InfoMessages.h"
#include "Beeper.h"
#include "httplib.h"
#include <chrono>

// included while we fake things out
#include "CPlayerManager.h"

CLevelViewerApp::CLevelViewerApp() : CApplication("Avara") {
    itsGame = new CAvaraGame(64);
    gCurrentGame = itsGame;
    itsGame->IAvaraGame(this);
    itsGame->UpdateViewRect(mSize.x, mSize.y, mPixelRatio);

    gameNet->ChangeNet(kNullNet, "");

    setLayout(new nanogui::FlowLayout(nanogui::Orientation::Vertical, true, 20, 20));

    levelWindow = new CLevelWindow(this);
    levelWindow->setFixedWidth(200);

    performLayout();

    gameNet->SendLoadLevel("aa-normal", 1);
}

CLevelViewerApp::~CLevelViewerApp() {
    itsGame->Dispose();
    DeallocParser();
}

void CLevelViewerApp::Done() {
    // This will trigger a clean disconnect if connected.
    gameNet->ChangeNet(kNullNet, "");
    CApplication::Done();
}

void CLevelViewerApp::idle() {
    CheckSockets();
    /*
    if(gameNet->playerTable[0]->LoadingStatus() == kLLoaded) {
        gameNet->SendStartCommand();
    }
    */
    if(itsGame->GameTick()) {
        glClearColor(mBackground[0], mBackground[1], mBackground[2], mBackground[3]);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        drawContents();
        SDL_GL_SwapWindow(mSDLWindow);
    }
}

void CLevelViewerApp::drawContents() {
    itsGame->Render(mNVGContext);
}

void CLevelViewerApp::WindowResized(int width, int height) {
    itsGame->UpdateViewRect(width, height, mPixelRatio);
    //performLayout();
}

bool CLevelViewerApp::handleSDLEvent(SDL_Event &event) {
    if(itsGame->IsPlaying()) {
        itsGame->HandleEvent(event);
        return true;
    }
    else {
        return CApplication::handleSDLEvent(event);
    }
}

void CLevelViewerApp::drawAll() {
    if (!itsGame->IsPlaying()) {
        CApplication::drawAll();
    }
}

bool CLevelViewerApp::DoCommand(int theCommand) {
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
            if( theCommand >= kLatencyToleranceZero &&
                theCommand <= kLatencyToleranceMax)
            {   WriteShortPref(kLatencyToleranceTag, theCommand - kLatencyToleranceZero);
            }
            else
            if( theCommand >= kRetransmitMin &&
                theCommand <= kRetransmitMax)
            {   WriteShortPref(kUDPResendPrefTag, theCommand - kRetransmitMin);

                if(gameNet->itsCommManager)
                {   gameNet->itsCommManager->OptionCommand(theCommand);
                }
            }
            if( theCommand >= kSlowestConnectionCmd && theCommand <= kFastestConnectionCmd)
            {   WriteShortPref(kUDPConnectionSpeedTag, theCommand - kSlowestConnectionCmd);

                if(gameNet->itsCommManager)
                {   gameNet->itsCommManager->OptionCommand(theCommand);
                }
            }
            else
                inherited::DoCommand(theCommand);
            break;
    }
    */
}


OSErr CLevelViewerApp::LoadLevel(std::string set, OSType theLevel) {
    SDL_Log("LOADING LEVEL %d FROM %s\n", theLevel, set.c_str());
    itsGame->LevelReset(false);
    itsGame->loadedTag = theLevel;
    gCurrentGame = itsGame;

    char byte1 =  theLevel & 0x000000ff;
    char byte2 = (theLevel & 0x0000ff00) >> 8;
    char byte3 = (theLevel & 0x00ff0000) >> 16;
    char byte4 = (theLevel & 0xff000000) >> 24;

    std::string test = std::string({byte4, byte3, byte2, byte1});
    SDL_Log("%s", test.c_str());

    std::string svgdir = std::string("levels/") + set + "_svg/";
    SDL_Log("%s", svgdir.c_str());
    SVGConvertToLevelMap();
    return noErr;
}

/*
OSErr CLevelViewerApp::LoadLevel(std::string set, OSType theLevel) {
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
            BlockMoveData(curLevel->name, itsGame->loadedLevel, curLevel->name[0] + 1);
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
*/

void CLevelViewerApp::NotifyUser() {
    // TODO: Bell sound(s)
    SDL_Log("BEEP!!!\n");
    Beep();
}

CAvaraGame* CLevelViewerApp::GetGame() {
    return itsGame;
}

CNetManager* CLevelViewerApp::GetNet() {
    return gameNet;
}
void CLevelViewerApp::SetNet(CNetManager *theNet) {
    gameNet = theNet;
}

void CLevelViewerApp::AddMessageLine(std::string line) {
    SDL_Log("Message: %s", line.c_str());
    messageLines.push_back(line);
    if (messageLines.size() > 5) {
        messageLines.pop_front();
    }
}
void CLevelViewerApp::MessageLine(short index, short align) {
    SDL_Log("CLevelViewerApp::MessageLine(%d)\n", index);
    switch(index) {
        case kmWelcome1:
        case kmWelcome2:
        case kmWelcome3:
        case kmWelcome4:
            AddMessageLine("Welcome to Avara.");
            break;
        case kmStarted:
            AddMessageLine("Starting new game.");
            break;
        case kmRestarted:
            AddMessageLine("Resuming game.");
            break;
        case kmAborted:
            AddMessageLine("Aborted.");
            break;
        case kmWin:
            AddMessageLine("Mission complete.");
            break;
        case kmGameOver:
            AddMessageLine("Game over.");
            break;
        case kmSelfDestruct:
            AddMessageLine("Self-destruct activated.");
            break;
        case kmFragmentAlert:
            AddMessageLine("ALERT: Reality fragmentation detected!");
            break;
        case kmRefusedLogin:
            AddMessageLine("Login refused.");
            break;
    }

}

std::deque<std::string>& CLevelViewerApp::MessageLines() {
    return messageLines;
}
void CLevelViewerApp::LevelReset() {}
void CLevelViewerApp::ParamLine(short index, short align, StringPtr param1, StringPtr param2) {
    SDL_Log("CLevelViewerApp::ParamLine(%d)\n", index);
    std::stringstream buffa;
    std::string a = std::string((char *)param1 + 1, param1[0]);
    std::string b;
    if (param2) b = std::string((char *)param2 + 1, param2[0]);

    switch(index) {
        case kmPaused:
            buffa << "Game paused by " << a << ".";
            break;
        case kmWaitingForPlayer:
            buffa << "Waiting for " << a << ".";
            break;
        case kmAKilledBPlayer:
            buffa << a << " killed " << b << ".";
            break;
        case kmUnavailableNote:
            buffa << a << " is busy.";
            break;
        case kmStartFailure:
            buffa << a << " wasn't ready.";
            break;
    }

    AddMessageLine(buffa.str());
}
void CLevelViewerApp::StartFrame(long frameNum) {}

void CLevelViewerApp::StringLine(StringPtr theString, short align) {
    AddMessageLine(std::string((char* ) theString + 1, theString[0]).c_str());
}

void CLevelViewerApp::ComposeParamLine(StringPtr destStr, short index, StringPtr param1, StringPtr param2) {
    ParamLine(index, 0, param1, param2);
}

void CLevelViewerApp::SetIndicatorDisplay(short i, short v) {}
void CLevelViewerApp::NumberLine(long theNum, short align) {}
void CLevelViewerApp::DrawUserInfoPart(short i, short partList) {}
void CLevelViewerApp::BrightBox(long frameNum, short position) {}
