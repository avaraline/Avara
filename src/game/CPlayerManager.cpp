/*
    Copyright ©1995-1996, Juri Munkki
    All rights reserved.

    File: CPlayerManager.c
    Created: Saturday, March 11, 1995, 05:50
    Modified: Sunday, September 15, 1996, 20:51
*/

#include "CPlayerManager.h"

#include "CAbstractPlayer.h"
#include "CCommManager.h"
#include "CIncarnator.h"
#include "CNetManager.h"
#include "CWalkerActor.h"
#include "CommDefs.h"

//#include "CRosterWindow.h"
//#include "JAMUtil.h"
#include "CApplication.h"
#include "CommandList.h"
//#include "CInfoPanel.h"
#include "InfoMessages.h"
#include "Parser.h"
#include "Preferences.h"
#include "System.h"

#include <SDL2/SDL.h>

void CPlayerManager::IPlayerManager(CAvaraGame *theGame, short id, CNetManager *aNetManager) {
    // Rect	*mainScreenRect;

    itsGame = theGame;
    itsPlayer = NULL;
    slot = id;

    int width, height;
    SDL_GetWindowSize(itsGame->itsApp->sdlWindow(), &width, &height);
    mouseCenterPosition.h = width / 2;
    mouseCenterPosition.v = height / 2;
    oldMouse.h = mouseCenterPosition.h;
    oldMouse.v = mouseCenterPosition.v;
    lastMouseControlTime = 0;

    // Mirrors what's in Preferences.h for kKeyboardMappingTag
    json commandBits = {{"forward", 1 << kfuForward},
        {"backward", 1 << kfuReverse},
        {"left", 1 << kfuLeft},
        {"right", 1 << kfuRight},
        {"jump", 1 << kfuJump},
        {"abort", 1 << kfuAbortGame},
        {"loadMissile", 1 << kfuLoadMissile},
        {"loadGrenade", 1 << kfuLoadGrenade},
        {"fire", 1 << kfuFireWeapon},
        {"boost", 1 << kfuBoostEnergy},
        {"aimForward", 1 << kfuAimForward},
        {"scoutView", 1 << kfuScoutView},
        {"scoutControl", 1 << kfuScoutControl},
        {"chatMode", 1 << kfuTypeText}};

    // Read the keyboard mapping prefs.
    json keys = itsGame->itsApp->Get(kKeyboardMappingTag);
    json newMap;
    for (json::iterator it = keys.begin(); it != keys.end(); ++it) {
        if (!commandBits[it.key()].is_number())
            continue;
        newMap[it.key()] = it.value();
        uint32_t cmd = commandBits[it.key()];
        if (it.value().is_array()) {
            for (json::iterator kit = it.value().begin(); kit != it.value().end(); ++kit) {
                SDL_Keycode key = SDL_GetKeyFromName((*kit).get<std::string>().c_str());
                SDL_Scancode scan = SDL_GetScancodeFromKey(key);
                keyMap[scan] |= cmd;
            }
        } else {
            SDL_Keycode key = SDL_GetKeyFromName(it.value().get<std::string>().c_str());
            SDL_Scancode scan = SDL_GetScancodeFromKey(key);
            keyMap[scan] |= cmd;
        }
    }
    itsGame->itsApp->Set(kKeyboardMappingTag, newMap);

    //SDL_StartTextInput();
    //SDL_StopTextInput();

    // mainScreenRect = &(*GetMainDevice())->gdRect;
    // mouseCenterPosition.h = (mainScreenRect->left + mainScreenRect->right) / 2;
    // mouseCenterPosition.v = (mainScreenRect->top + mainScreenRect->bottom) / 2;

    // theRoster = aRoster;
    theNetManager = aNetManager;
    levelCRC = 0;
    levelTag = 0;
    position = id;
    itsPlayer = NULL;

    mugPict = NULL;
    mugSize = -1;
    mugState = 0;

    NetDisconnect();
    isLocalPlayer = false;
    prevKeyboardActive = keyboardActive;
}

void CPlayerManager::SetPlayer(CAbstractPlayer *thePlayer) {
    itsPlayer = thePlayer;
}

uint32_t CPlayerManager::DoMouseControl(Point *deltaMouse, Boolean doCenter) {
    int x, y;
    uint32_t state = SDL_GetMouseState(&x, &y);
    deltaMouse->h = x - mouseCenterPosition.h;
    deltaMouse->v = y - mouseCenterPosition.v;
    if (itsGame->sensitivity > 0) {
        deltaMouse->h <<= itsGame->sensitivity;
        deltaMouse->v <<= itsGame->sensitivity;
    } else if (itsGame->sensitivity < 0) {
        deltaMouse->h >>= -itsGame->sensitivity;
        deltaMouse->v >>= -itsGame->sensitivity;
    }
    SDL_WarpMouseInWindow(itsGame->itsApp->sdlWindow(), mouseCenterPosition.h, mouseCenterPosition.v);
    return state;
}

Boolean CPlayerManager::TestHeldKey(short funcCode) {
    /*
    long			m0, m1;
    KeyMap			keyMap;
    char			*keyMapP;
    long			*mapPtr, *keyFun;
    short			i,j;
    Handle			mapRes;

    m0 = 0x80000000L >> funcCode;
    m1 = 0x80000000L >> (funcCode - 32);

    mapRes = itsGame->mapRes;
    mapPtr = (long *)*mapRes;
    keyMapP = (Ptr)keyMap;
    GetKeys(keyMap);

    for(i=0;i<128;i+=8)
    {	char	bits;

        bits = *keyMapP++;
        if(bits)
        {	j = 8;
            while(j-- && bits)
            {	if(bits < 0)
                {	keyFun = &mapPtr[2*(i+j)];
                    if((keyFun[0] & m0) || (keyFun[1] & m1))
                        return true;
                }
                bits += bits;
            }
        }
    }
    */
    return false;
}

void FrameFunctionToPacket(FrameFunction *ff, PacketInfo *outPacket, short slot);

void FrameFunctionToPacket(FrameFunction *ff, PacketInfo *outPacket, short slot) {
    FunctionTable *ft;
    uint32_t *pd;
    short *spd;

    ft = &ff->ft;

    outPacket->p1 = ft->buttonStatus | kDataFlagMask;
    outPacket->p2 = ff->ft.msgChar;
    outPacket->p3 = ff->validFrame;

    pd = (uint32_t *)outPacket->dataBuffer;

    if (ft->down) {
        outPacket->p1 &= ~kNoDownData;
        *pd++ = htonl(ft->down);
    }
    if (ft->up) {
        outPacket->p1 &= ~kNoUpData;
        *pd++ = htonl(ft->up);
    }
    if (ft->held) {
        outPacket->p1 &= ~kNoHeldData;
        *pd++ = htonl(ft->held);
    }
    spd = (short *)pd;

    if (ft->mouseDelta.v) {
        outPacket->p1 &= ~kNoMouseV;
        *spd++ = htons(ft->mouseDelta.v);
    }

    if (ft->mouseDelta.h) {
        outPacket->p1 &= ~kNoMouseH;
        *spd++ = htons(ft->mouseDelta.h);
    }

    outPacket->dataLen = ((char *)spd) - outPacket->dataBuffer;
}

//#define	AVARA_EVENT_MASK	(mDownMask+mUpMask+keyDownMask+keyUpMask+autoKeyMask)

uint32_t CPlayerManager::GetKeyBits() {
    uint32_t keys = 0;
    const uint8_t *state = SDL_GetKeyboardState(NULL);
    for (auto it = keyMap.begin(); it != keyMap.end(); ++it) {
        if (state[it->first])
            keys |= it->second;
    }
    return keys;
}

void CPlayerManager::HandleEvent(SDL_Event &event) {
    // Events coming in are for the next frame to be sent.
    // FrameFunction *ff = &frameFuncs[(FUNCTIONBUFFERS - 1) & (itsGame->frameNumber + 1)];

    switch (event.type) {
        case SDL_KEYDOWN:
            keysDown |= keyMap[event.key.keysym.scancode];
            keysHeld |= keyMap[event.key.keysym.scancode];

            if (TESTFUNC(kfuTypeText, keysDown)) {
                keyboardActive = !keyboardActive;
                if (keyboardActive) {
                    SDL_StartTextInput();
                }
                else {
                    SDL_StopTextInput();
                }
            }
            break;
        case SDL_TEXTINPUT:
            if (keyboardActive) {
                for(char* a_char = event.text.text; *a_char; ++a_char) {
                    inputBuffer.push_back(*a_char);
                }
            }
        case SDL_KEYUP:
            keysUp |= keyMap[event.key.keysym.scancode];
            keysHeld &= ~keyMap[event.key.keysym.scancode];
            break;
        case SDL_MOUSEBUTTONDOWN:
            buttonStatus |= kbuWentDown;
            buttonStatus |= kbuIsDown;
            break;
        case SDL_MOUSEBUTTONUP:
            buttonStatus |= kbuWentUp;
            buttonStatus &= ~kbuIsDown;
            break;
        case SDL_MOUSEMOTION:
            int xrel = event.motion.xrel, yrel = event.motion.yrel;
            if (itsGame->sensitivity > 0) {
                xrel <<= itsGame->sensitivity;
                yrel <<= itsGame->sensitivity;
            } else if (itsGame->sensitivity < 0) {
                xrel >>= -itsGame->sensitivity;
                yrel >>= -itsGame->sensitivity;
            }
            mouseX += xrel;
            mouseY += yrel;
            break;
    }
}

void CPlayerManager::SendFrame() {
    // Sends the next game frame.
    itsGame->topSentFrame++;

    FrameFunction *ff = &frameFuncs[(FUNCTIONBUFFERS - 1) & itsGame->topSentFrame];

    ff->validFrame = itsGame->topSentFrame;

    if (!keyboardActive) {
        ff->ft.down = keysDown;
        ff->ft.up = keysUp;
        ff->ft.held = keysHeld;
        ff->ft.msgChar = 0;
        prevKeyboardActive = keyboardActive;
    }
    else {
        if (!inputBuffer.empty()) {
            ff->ft.msgChar = inputBuffer.front();
            inputBuffer.pop_front();
        }
        else {
            ff->ft.msgChar = 0;
        }
        ff->ft.down = 0;
        ff->ft.up = 0;
        ff->ft.held = 0;
        if (prevKeyboardActive == false) {
            prevKeyboardActive = true;
            ff->ft.down |= 1 << kfuTypeText;
        }
    }

    ff->ft.mouseDelta.h = mouseX;
    ff->ft.mouseDelta.v = mouseY;
    ff->ft.buttonStatus = buttonStatus;

    CCommManager *theComm = theNetManager->itsCommManager;

    PacketInfo *outPacket = theComm->GetPacket();
    if (outPacket) { // long	*pd;

        outPacket->command = kpKeyAndMouse;
        outPacket->distribution = theNetManager->activePlayersDistribution;

        FrameFunctionToPacket(ff, outPacket, slot);

        if (ff->ft.msgChar) {
            short othersDistribution;

            othersDistribution = kdEveryone & ~theNetManager->activePlayersDistribution;
            theNetManager->FlushMessageBuffer();
            theComm->SendPacket(othersDistribution, kpRosterMessage, 0, 0, 0, 1, &ff->ft.msgChar);
        }

        // theNetManager->FastTrackDeliver(outPacket);
        outPacket->flags |= kpUrgentFlag;
        theComm->WriteAndSignPacket(outPacket);
    }

    keysDown = keysUp = 0;
    mouseX = mouseY = 0;
    buttonStatus = 0;

    // Clear out the next frame for events coming in after this
    // FrameFunction *next = &frameFuncs[(FUNCTIONBUFFERS - 1) & (itsGame->topSentFrame + 1)];
    // SDL_memset(next, 0, sizeof(FrameFunction));
    // next->ft.held = ff->ft.held;
}

void CPlayerManager::ResendFrame(long theFrame, short requesterId, short commandCode) {
    CCommManager *theComm;
    PacketInfo *outPacket;
    FrameFunction *ff;

    theComm = theNetManager->itsCommManager;

    ff = &frameFuncs[(FUNCTIONBUFFERS - 1) & theFrame];

    if (ff->validFrame == theFrame) {
        outPacket = theComm->GetPacket();
        if (outPacket) {
            long *pd;

            outPacket->command = commandCode;
            outPacket->distribution = 1 << requesterId;

            FrameFunctionToPacket(ff, outPacket, slot);

            outPacket->flags |= kpUrgentFlag;
            theComm->WriteAndSignPacket(outPacket);
        }
    } else //	Ask me later packet
    {
        SDL_Log("CPlayerManager::ResendFrame - ask me later\n");
        theComm->SendUrgentPacket(1 << requesterId, kpAskLater, 0, 0, theFrame, 0, 0);
    }
}

void CPlayerManager::ResumeGame() {
    short i;

    for (i = 0; i < FUNCTIONBUFFERS; i++) {
        SDL_memset(&frameFuncs[i], 0, sizeof(FrameFunction));
        frameFuncs[i].validFrame = -1;
    }

    bufferStart = 0;
    bufferEnd = 0;
    keyboardActive = false;

    oldHeld = 0;
    oldModifiers = 0;
    oldButton = 0;
    isLocalPlayer = (theNetManager->itsCommManager->myId == slot);

    keysDown = keysUp = keysHeld = 0;
    mouseX = mouseY = 0;
    buttonStatus = 0;
}

void CPlayerManager::ProtocolHandler(struct PacketInfo *thePacket) {
    uint32_t *pd;
    uint16_t *spd;
    FrameFunction *ff;
    uint32_t frameNumber;
    char p1;

    p1 = thePacket->p1;
    frameNumber = thePacket->p3;

    pd = (uint32_t *)thePacket->dataBuffer;
    ff = &frameFuncs[(FUNCTIONBUFFERS - 1) & frameNumber];
    ff->validFrame = frameNumber;

    ff->ft.down = (p1 & kNoDownData) ? 0 : ntohl(*pd++);
    ff->ft.up = (p1 & kNoUpData) ? 0 : ntohl(*pd++);
    ff->ft.held = (p1 & kNoHeldData) ? 0 : ntohl(*pd++);

    spd = (uint16_t *)pd;

    ff->ft.mouseDelta.v = (p1 & kNoMouseV) ? 0 : ntohs(*spd++);
    ff->ft.mouseDelta.h = (p1 & kNoMouseH) ? 0 : ntohs(*spd++);

    ff->ft.buttonStatus = p1 & ~kDataFlagMask;
    ff->ft.msgChar = thePacket->p2;
}

void CPlayerManager::ViewControl() {
    if (itsPlayer)
        itsPlayer->ControlViewPoint();
}

void CPlayerManager::Dispose() {
    CDirectObject::Dispose();
}

void CPlayerManager::SendResendRequest(short askCount) {
    if (/* theNetManager->fastTrack.addr.value || */ askCount > 0) {
        theNetManager->playerTable[theNetManager->itsCommManager->myId]->ResendFrame(
            itsGame->frameNumber, slot, kpKeyAndMouseRequest);
    }
}

FunctionTable *CPlayerManager::GetFunctions() {
    // SDL_Log("CPlayerManager::GetFunctions\n");
    short i = (FUNCTIONBUFFERS - 1) & itsGame->frameNumber;

    if (frameFuncs[i].validFrame != itsGame->frameNumber) {
        long quickTick;
        long firstTime = askAgainTime = TickCount();
        short askCount = 0;

        itsGame->didWait = true;

        if (frameFuncs[(FUNCTIONBUFFERS - 1) & (i + 1)].validFrame < itsGame->frameNumber) {
            askAgainTime += 5 + (FRandom() & 3);
        }

        do {
            theNetManager->ProcessQueue();

            // While we're waiting for packets, process key/mouse events so they don't build up.
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                theNetManager->HandleEvent(event);
            }

            if (itsGame->canPreSend && (long)(SDL_GetTicks() - itsGame->nextScheduledFrame) >= 0) {
                itsGame->canPreSend = false;
                theNetManager->FrameAction();
            }

            quickTick = TickCount();

            if (quickTick - askAgainTime >= 0) {
                if (quickTick - firstTime > 120 && TestHeldKey(kfuAbortGame)) {
                    itsGame->statusRequest = kAbortStatus;
                    break;
                }

                askAgainTime = quickTick + 300; //	Five seconds
                SendResendRequest(askCount++);
                if (askCount == 2) {
                    itsGame->itsApp->ParamLine(kmWaitingForPlayer, centerAlign, playerName, NULL);
                    // TODO: waiting for player dialog
                    // InitCursor();
                    // gApplication->SetCommandParams(STATUSSTRINGSLISTID, kmWaitingPlayers, true, 0);
                    // gApplication->BroadcastCommand(kBusyStartCmd);
                }

                //				itsGame->timer.currentStep += 2;
            }

            if (askCount > 1) {
                /* TODO: waiting dialog
                gApplication->BroadcastCommand(kBusyTimeCmd);
                if(gApplication->commandResult)
                {	itsGame->statusRequest = kAbortStatus;
                    break;
                }
                */
            }

        } while (frameFuncs[i].validFrame != itsGame->frameNumber);

        if (askCount > 1) {
            gApplication->BroadcastCommand(kBusyEndCmd);
            // HideCursor();
        }

        if (quickTick != firstTime) {
            if (quickTick > firstTime + 3 || itsGame->longWait) {
                itsGame->veryLongWait = true;
            }

            itsGame->longWait = true;
        }
    }

    return &frameFuncs[i].ft;
}

void CPlayerManager::RosterKeyPress(unsigned char c) {
    short i;

    switch (c) {
        /*
        case 7:
            theNetManager->Beep();
            RosterKeyPress('‚àÜ');
            break;
        case 13:	//	Return
            RosterKeyPress('¨');
            RosterKeyPress(' ');
            break;
        case 9:		//	Tab
            RosterKeyPress(' ');
            RosterKeyPress(' ');
            break;
        case 8:		//	Backspace
            if(message[0]) --message[0];
            break;
        case 27:	//	Escape
            message[0] = 0;
            break;
        */
        default:
            if (c >= 32) {
                if (message[0] < kMaxMessageChars) {
                    message[++message[0]] = c;
                } else {
                    for (i = 1; i < kMaxMessageChars; i++) {
                        message[i] = message[i + 1];
                    }
                    message[kMaxMessageChars] = c;
                }
            }
            break;
    }
}

/*
static
OSErr	WordWrapString(
    StringPtr	fullLine)
{
    if(StringWidth(fullLine) <= kChatMessageWidth)
    {	return	0;
    }
    else
    {	short	relen;

        relen = fullLine[0];

        while(relen)
        {	unsigned char	theChar;

            theChar = fullLine[relen--];
            if(theChar <= 32 || theChar == '†')	//	option-space
            {	short	width;

                width = TextWidth(fullLine, 1, relen);
                if(width < kChatMessageWidth / 2)
                {	return TruncString(kChatMessageWidth, fullLine, smTruncEnd);
                }
                else if(width <= kChatMessageWidth)
                {	fullLine[0] = relen+2;
                    fullLine[relen+2] = ' ';
                    return 1;
                }
            }
        }
    }

    return TruncString(kChatMessageWidth, fullLine, smTruncEnd);
}

void	CPlayerManager::FlushMessageText(
    Boolean	forceAll)
{
    Str255			fullLine;
    short			maxLen;
    short			baseLen;
    GrafPtr			savedPort;
    TextSettings	savedTexs;
    OSErr			err;

    GetPort(&savedPort);
    SetPort(theRoster->itsWindow);
    GetSetTextSettings(&savedTexs, geneva, 0, 9, srcOr);

    do
    {
        BlockMoveData(playerName, fullLine, playerName[0] + 1);

#if 0
        while(fullLine[0] && fullLine[fullLine[0]] == 32)
        {	fullLine[0]--;
        }
#endif
        TruncString(kChatMessageWidth/3, fullLine, smTruncEnd);

        fullLine[++fullLine[0]] = ':';
        fullLine[++fullLine[0]] = ' ';

        baseLen = fullLine[0];
        maxLen = lineBuffer[0];

        if(maxLen + baseLen > 250)
        {	maxLen = 250 - baseLen;
        }

        BlockMoveData(lineBuffer+1, fullLine + 1 + baseLen, maxLen);
        fullLine[0] += maxLen;
#if 1
        err = WordWrapString(fullLine);
#else
        err = TruncString(kChatMessageWidth, fullLine, smTruncEnd);
#endif

        if(err == 1 || forceAll)
        {	short	outLen;


            if(err == 1)
            {	outLen = fullLine[0] - baseLen - 1;
            }
            else
            {	outLen = maxLen;
            }

            if(outLen != 0)	theRoster->NewChatLine(fullLine);

            BlockMoveData(lineBuffer + outLen + 1, lineBuffer + 1, lineBuffer[0] - outLen);
            lineBuffer[0] -= outLen;

            if(err == 0)	forceAll = false;
        }
    } while(err == 1 || forceAll);

    isLocalPlayer = (theNetManager->itsCommManager->myId == slot);

    if(isLocalPlayer)
    {	theRoster->SetChatLine(fullLine);
    }

    RestoreTextSettings(&savedTexs);
    SetPort(savedPort);
}
*/

void CPlayerManager::RosterMessageText(short len, char *c) {
    while (len--) {
        unsigned char theChar;

        theChar = *c++;

        switch (theChar) {
            case 8:
                //if (lineBuffer[0])
                //    lineBuffer[0]--;
                if (lineBuffer.size()) {
                    lineBuffer.pop_back();
                }
                break;
            case 13:
                // FlushMessageText(true);
                break;
            case 27:
                lineBuffer.clear();
                break;
            default:
                if (theChar >= 32) {
                    lineBuffer.push_back(theChar);
                    if (lineBuffer[0] > 220) { // FlushMessageText(true);
                        for (int i = 0; i < 50; ++i)
                        {
                            lineBuffer.pop_front();
                        }
                    }
                }
                break;
        }
    }

    // FlushMessageText(false);
}

void CPlayerManager::GameKeyPress(char theChar) {
    theNetManager->ReceiveRosterMessage(slot, 1, &theChar);
}

void CPlayerManager::NetDisconnect() {
    if (itsPlayer) {
        if (theNetManager->isPlaying) {
            itsPlayer->netDestruct = true;
        } else {
            itsPlayer->Dispose();
        }
    }

    if (mugPict)
        DisposeHandle(mugPict);

    mugPict = NULL;
    mugSize = -1;
    mugState = 0;

    globalLocation.h = 0;
    globalLocation.v = 0;

    message[0] = 0;
    //lineBuffer[0] = 0;
    lineBuffer.clear();
    playerName[0] = 0;
    spaceCount = 0;
    playerRegName[0] = 0;
    isRegistered = false;
    loadingStatus = kLNotConnected;
    winFrame = -1;
    // theRoster->InvalidateArea(kUserBox, position);
    // theRoster->InvalidateArea(kOnePlayerBox, position);
}

void CPlayerManager::ChangeNameAndLocation(StringPtr theName, Point location) {
    StringPtr lastChar;

    if (loadingStatus == kLNotConnected) {
        loadingStatus = kLConnected;
        // theRoster->InvalidateArea(kOnePlayerBox, position);
    }

    if (location.h != globalLocation.h || location.v != globalLocation.v) {
        globalLocation = location;
        // theRoster->InvalidateArea(kFullMapBox, position);
        // theRoster->InvalidateArea(kMapInfoBox, position);
    }

    BlockMoveData(theName, playerName, theName[0] + 1);

    lastChar = playerName + playerName[0];
    spaceCount = 0;
    while (lastChar > playerName && *lastChar-- == ' ') {
        spaceCount++;
    }

    playerName[0] -= spaceCount;

    // theRoster->InvalidateArea(kUserBoxTopLine, position);
    // FlushMessageText(false);
}

void CPlayerManager::SetPosition(short pos) {
    position = pos;
    // theRoster->InvalidateArea(kUserBox, pos);
    // theRoster->InvalidateArea(kOnePlayerBox, pos);
}

void CPlayerManager::LoadStatusChange(short serverCRC, OSErr serverErr, OSType serverTag) {
    short oldStatus;

    if (loadingStatus != kLNotConnected && loadingStatus != kLActive) {
        oldStatus = loadingStatus;

        if (serverErr || levelErr) {
            if (levelErr == 1)
                loadingStatus = kLTrying;
            else if (serverErr == 1)
                loadingStatus = kLWaiting;
            else
                loadingStatus = kLNotFound;
        } else {
            if (serverCRC == levelCRC && serverTag == levelTag) {
                short i;

                SDL_Log("Setting loadingStatus = kLLoaded\n");
                loadingStatus = kLLoaded;
#if 0
                oldHeld[0] = oldHeld[1] = 0;

                for(i=0;i<FUNCTIONBUFFERS;i++)
                {	frameFuncs[i].validFrame = -1;
                }
#endif
            } else {
                if (serverTag == levelTag) {
                    loadingStatus = kLMismatch;
                }
            }
        }

        if (oldStatus != loadingStatus) {
            if (loadingStatus != kLConnected)
                winFrame = -1;

            // theRoster->InvalidateArea(kUserBoxTopLine, position);
        }
    }
}

CAbstractPlayer *CPlayerManager::ChooseActor(CAbstractPlayer *actorList, short myTeamColor) {
    CAbstractPlayer *newList;
    CAbstractPlayer **tailList;
    CAbstractPlayer *nextPlayer;
    short myTeamMask;

    itsPlayer = NULL;
    playerColor = myTeamColor;
    myTeamMask = 1 << myTeamColor;

    newList = NULL;
    tailList = &newList;

    while (actorList) {
        nextPlayer = (CAbstractPlayer *)actorList->nextActor;

        if (actorList->teamColor == myTeamColor) { //	Good enough for me.

            itsPlayer = actorList;
            itsPlayer->itsManager = this;
            itsPlayer->PlayerWasMoved();
            itsPlayer->BuildPartProximityList(itsPlayer->location, itsPlayer->proximityRadius, kSolidBit);
            itsPlayer->AddToGame();

            myTeamColor = -999; //	Ignore the rest.
        } else {
            *tailList = actorList;
            tailList = (CAbstractPlayer **)&actorList->nextActor;
        }
        actorList = nextPlayer;
    }

    *tailList = NULL; //	Terminate list.
    actorList = newList;

    if (itsPlayer == NULL) {
        CIncarnator *spotAvailable;

        itsPlayer = new CWalkerActor;
        itsPlayer->IAbstractActor();
        itsPlayer->BeginScript();
        ProgramLongVar(iTeam, myTeamColor);
        FreshCalc(); //	Parser call.
        itsPlayer->EndScript();
        itsPlayer->isInLimbo = true;
        itsPlayer->limboCount = 0;
        itsPlayer->itsManager = this;

        spotAvailable = itsGame->incarnatorList;
        while (spotAvailable) {
            if (spotAvailable->enabled && spotAvailable->colorMask & myTeamMask) {
                itsPlayer->Reincarnate(spotAvailable);
                if (!itsPlayer->isInLimbo) {
                    break;
                }
            }
            spotAvailable = spotAvailable->nextIncarnator;
        }

        if (itsPlayer->isInLimbo) {
            itsPlayer->Dispose();
            itsPlayer = NULL;
        } else {
            itsPlayer->AddToGame();
        }
    }

    return actorList;
}

Boolean CPlayerManager::IncarnateInAnyColor() {
    short i;
    CIncarnator *spotAvailable;

    for (i = 1; i <= kMaxTeamColors; i++) {
        itsPlayer = new CWalkerActor;
        itsPlayer->IAbstractActor();
        itsPlayer->BeginScript();
        ProgramLongVar(iTeam, i);
        FreshCalc(); //	Parser call.
        itsPlayer->EndScript();
        itsPlayer->isInLimbo = true;
        itsPlayer->limboCount = 0;
        itsPlayer->itsManager = this;

        spotAvailable = itsGame->incarnatorList;
        while (spotAvailable) {
            if (spotAvailable->enabled && spotAvailable->colorMask & (1 << i)) {
                itsPlayer->Reincarnate(spotAvailable);
                if (!itsPlayer->isInLimbo) {
                    break;
                }
            }
            spotAvailable = spotAvailable->nextIncarnator;
        }

        if (itsPlayer->isInLimbo) {
            itsPlayer->Dispose();
            itsPlayer = NULL;
        } else {
            itsPlayer->AddToGame();
            playerColor = i;
            return true;
        }
    }

    return false;
}

CAbstractPlayer *CPlayerManager::TakeAnyActor(CAbstractPlayer *actorList) {
    CAbstractPlayer *nextPlayer;

    nextPlayer = (CAbstractPlayer *)actorList->nextActor;

    playerColor = actorList->teamColor;

    itsPlayer = actorList;
    itsPlayer->itsManager = this;
    itsPlayer->AddToGame();

    return nextPlayer;
}
void CPlayerManager::SetPlayerStatus(short newStatus, long theWin) {
    winFrame = theWin;

    if (newStatus != loadingStatus) {
        if (loadingStatus == kLNotConnected) { // theRoster->InvalidateArea(kOnePlayerBox, position);
        }

        loadingStatus = newStatus;
        // theRoster->InvalidateArea(kUserBoxTopLine, position);
    }
}

void CPlayerManager::AbortRequest() {
    theNetManager->activePlayersDistribution &= ~(1 << slot);
    if (isLocalPlayer) {
        itsGame->statusRequest = kAbortStatus;
    }
}

void CPlayerManager::DeadOrDone() {
    theNetManager->deadOrDonePlayers |= 1 << slot;
}

short CPlayerManager::GetStatusChar() {
    if (itsPlayer == NULL || (loadingStatus != kLActive && loadingStatus != kLPaused)) {
        return -1;
    } else {
        if (itsPlayer->winFrame >= 0) {
            return 12;
        } else {
            if (itsPlayer->lives < 10) {
                return itsPlayer->lives;
            } else {
                return 10;
            }
        }
    }
}

short CPlayerManager::GetMessageIndicator() {
    if (itsPlayer && itsPlayer->chatMode && loadingStatus == kLActive) {
        return 11;
    } else {
        return -1;
    }
}

void CPlayerManager::StoreMugShot(Handle mugHandle) {
    if (mugPict) {
        DisposeHandle(mugPict);
    }

    mugPict = mugHandle;

    if (mugHandle) {
        mugSize = GetHandleSize(mugHandle);
        mugState = mugSize;
    } else {
        mugSize = -2;
        mugState = 0;
    }
}

Handle CPlayerManager::GetMugShot() {
    Handle result = NULL;

    if (mugSize == -1) {
        if (loadingStatus != kLNotConnected) {
            mugSize = -2;

            isLocalPlayer = (theNetManager->itsCommManager->myId == slot);

            if (isLocalPlayer) {
                gApplication->BroadcastCommand(kGiveMugShotCmd);
            } else {
                theNetManager->itsCommManager->SendPacket(1L << slot, kpGetMugShot, 0, 0, 0, 0, NULL);
            }
        }
    }

    if (mugPict && mugState == mugSize) {
        result = mugPict;
    }

    return result;
}

/*
void	CPlayerManager::GetLoadingStatusString(
    StringPtr	theStr)
{

    if(loadingStatus == kLConnected && winFrame >= 0)
    {	long	timeTemp = FMulDiv(winFrame, itsGame->frameTime, 10);
        char	mins[2];
        char	secs[2];
        char	hundreds[2];

        hundreds[0] = '0' + timeTemp % 10;
        timeTemp /= 10;
        hundreds[1] = '0' + timeTemp % 10;
        timeTemp /= 10;

        secs[0] = '0' + timeTemp % 10;
        timeTemp /= 10;
        secs[1] = '0' + timeTemp % 6;
        timeTemp /= 6;

        NumToString(timeTemp, theStr+1);
        theStr[0] = theStr[1] + 1;
        theStr[1] = '[';

        theStr[++theStr[0]] = ':';
        theStr[++theStr[0]] = secs[1];
        theStr[++theStr[0]] = secs[0];
        theStr[++theStr[0]] = '.';
        theStr[++theStr[0]] = hundreds[1];
        theStr[++theStr[0]] = hundreds[0];
        theStr[++theStr[0]] = ']';
    }
    else
    {	GetIndString(theStr, kRosterStringListID, loadingStatus);
    }
}
*/

void CPlayerManager::SpecialColorControl() {
    if (itsPlayer) {
        long repColor = -1;

        switch (spaceCount) {
            case 2:
                repColor = kSpecialBlackColor;
                break;
            case 3:
                repColor = kSpecialWhiteColor;
                break;
        }

        if (repColor >= 0) {
            itsPlayer->SetSpecialColor(repColor);
        }
    }
}
