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
#include "InfoMessages.h"
#include "Beeper.h"

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

void CCAvaraApp::NotifyUser() {
    // TODO: Bell sound(s)
    SDL_Log("BEEP!!!\n");
    Beep();
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

void CCAvaraApp::AddMessageLine(std::string line) {
    SDL_Log("Message: %s", line.c_str());
    messageLines.push_back(line);
    if (messageLines.size() > 5) {
        messageLines.pop_front();
    }
}
void CCAvaraApp::MessageLine(short index, short align) {
    SDL_Log("CAvaraApp::MessageLine(%d)\n", index);
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
std::deque<std::string> CCAvaraApp::GetMessageLines() {
    return messageLines;
}
void CCAvaraApp::LevelReset() {}
void CCAvaraApp::ParamLine(short index, short align, StringPtr param1, StringPtr param2) {
    SDL_Log("CAvaraAppImpl::ParamLine(%d)\n", index);
    std::ostringstream buffa;
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
void CCAvaraApp::StartFrame(long frameNum) {}
void CCAvaraApp::StringLine(StringPtr theString, short align) {
    AddMessageLine(std::string((char* ) theString + 1, theString[0]).c_str());
}
void CCAvaraApp::ComposeParamLine(StringPtr destStr, short index, StringPtr param1, StringPtr param2) {
    ParamLine(index, 0, param1, param2);
}
void CCAvaraApp::SetIndicatorDisplay(short i, short v) {}
void CCAvaraApp::NumberLine(long theNum, short align) {}
void CCAvaraApp::DrawUserInfoPart(short i, short partList) {}
void CCAvaraApp::BrightBox(long frameNum, short position) {}