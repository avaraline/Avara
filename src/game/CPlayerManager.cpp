/*
    Copyright ©1995-1996, Juri Munkki
    All rights reserved.

    File: CPlayerManager.c
    Created: Saturday, March 11, 1995, 05:50
    Modified: Sunday, September 15, 1996, 20:51
*/
// #define ENABLE_FPS_DEBUG  // uncomment if you want to see FPS_DEBUG output for this file

#include "CPlayerManager.h"

#include "CAbstractPlayer.h"
#include "CCommManager.h"
#include "CUDPComm.h"
#include "CIncarnator.h"
#include "CRandomIncarnator.h"
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
#include "Debug.h"

#include <SDL2/SDL.h>
#include <utf8.h>

CPlayerManager* CPlayerManager::theLocalPlayer;

void CPlayerManagerImpl::IPlayerManager(CAvaraGame *theGame, short id, CNetManager *aNetManager) {
    // Rect	*mainScreenRect;

    itsGame = theGame;
    itsPlayer = NULL;
    slot = id;
    presence = kzAvailable;

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
        {"pause", 1 << kfuPauseGame},
        {"abort", 1 << kfuAbortGame},
        {"loadMissile", 1 << kfuLoadMissile},
        {"loadGrenade", 1 << kfuLoadGrenade},
        {"fire", 1 << kfuFireWeapon},
        {"boost", 1 << kfuBoostEnergy},
        {"verticalMotion", 1 << kfuVerticalMotion},
        {"aimForward", 1 << kfuAimForward},
        {"lookLeft", 1 << kfuLookLeft},
        {"lookRight", 1 << kfuLookRight},
        {"zoomIn", 1 << kfuZoomIn},
        {"zoomOut", 1 << kfuZoomOut},
        {"scoutView", 1 << kfuScoutView},
        {"scoutControl", 1 << kfuScoutControl},
        {"spectateNext", 1 << kfuSpectateNext},
        {"spectatePrevious", 1 << kfuSpectatePrevious},
        {"scoreboard", 1 << kfuScoreboard},
        {"chatMode", 1 << kfuTypeText},
        {"debug1", 1 << kfuDebug1},
        {"debug2", 1 << kfuDebug2}};

    // Read the keyboard mapping prefs.
    json keys = itsGame->itsApp->Get(kKeyboardMappingTag);
    json newMap;
    for (json::iterator it = keys.begin(); it != keys.end(); ++it) {
        if (!commandBits[it.key()].is_number())
            continue;
        newMap[it.key()] = it.value();
        uint32_t cmd = commandBits[it.key()];
        json commandKeys = it.value();
        if (it.value().is_array() == false) {
            commandKeys = json::array({it.value()});
        }
        for (json::iterator kit = commandKeys.begin(); kit != commandKeys.end(); ++kit) {
            std::string name = (*kit).get<std::string>();
            SDL_Keycode key = SDL_GetKeyFromName(name.c_str());
            if(key == SDLK_UNKNOWN) {
                if(name == "Right Mouse") {
                    keyMap[SDL_SCANCODE_APP1] |= cmd;
                }
                else if(name == "Middle Mouse") {
                    keyMap[SDL_SCANCODE_APP2] |= cmd;
                }
            }
            else {
                SDL_Scancode scan = SDL_GetScancodeFromKey(key);
                keyMap[scan] |= cmd;
            }
        }
    }
    itsGame->itsApp->Set(kKeyboardMappingTag, newMap);

    // mainScreenRect = &(*GetMainDevice())->gdRect;
    // mouseCenterPosition.h = (mainScreenRect->left + mainScreenRect->right) / 2;
    // mouseCenterPosition.v = (mainScreenRect->top + mainScreenRect->bottom) / 2;

    // theRoster = aRoster;
    theNetManager = aNetManager;
    levelCRC = 0;
    levelTag = "";
    position = id;
    itsPlayer = NULL;

    mugPict = NULL;
    mugSize = -1;
    mugState = 0;

    NetDisconnect();
    SetLocal();

    prevKeyboardActive = keyboardActive;
}

void CPlayerManagerImpl::SetPlayer(CAbstractPlayer *thePlayer) {
    itsPlayer = thePlayer;
}

uint32_t CPlayerManagerImpl::DoMouseControl(Point *deltaMouse, Boolean doCenter) {
    int x, y;
    uint32_t state = SDL_GetMouseState(&x, &y);
    deltaMouse->h = x - mouseCenterPosition.h;
    deltaMouse->v = y - mouseCenterPosition.v;
    deltaMouse->h *= itsGame->sensitivity;
    deltaMouse->v *= itsGame->sensitivity;
    if (itsGame->moJoOptions & kFlipAxis) {
        deltaMouse->v = -deltaMouse->v;
	}
    SDL_WarpMouseInWindow(itsGame->itsApp->sdlWindow(), mouseCenterPosition.h, mouseCenterPosition.v);
    return state;
}

Boolean CPlayerManagerImpl::TestKeyPressed(short funcCode) {
    CPlayerManagerImpl* localManager = ((CPlayerManagerImpl*)itsGame->GetLocalPlayer()->itsManager);
    return TESTFUNC(funcCode, localManager->keysDown);
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

uint32_t CPlayerManagerImpl::GetKeyBits() {
    uint32_t keys = 0;
    const uint8_t *state = SDL_GetKeyboardState(NULL);
    for (auto it = keyMap.begin(); it != keyMap.end(); ++it) {
        if (state[it->first])
            keys |= it->second;
    }
    return keys;
}

void CPlayerManagerImpl::HandleEvent(SDL_Event &event) {
    // Events coming in are for the next frame to be sent.
    // FrameFunction *ff = &frameFuncs[(FUNCTIONBUFFERS - 1) & (itsGame->frameNumber + 1)];

    switch (event.type) {
        case SDL_KEYDOWN:
            // only add to keysDown if it's NOT from a keyboard repeat
            if (event.key.repeat == 0) {
                HandleKeyDown(keyMap[event.key.keysym.scancode]);
            }

            if (TESTFUNC(kfuTypeText, keysDown)) {
                keyboardActive = !keyboardActive;
                if (keyboardActive) {
                    SDL_StartTextInput();
                }
                else {
                    inputBuffer.push_back('\r');
                    SDL_StopTextInput();
                }
            }

            // Handle delete and backspace in chat mode
            if (keyboardActive &&
                event.key.keysym.sym == SDLK_BACKSPACE) {
                inputBuffer.push_back('\b');
            }

            if (keyboardActive &&
                (event.key.keysym.sym == SDLK_DELETE ||
                 event.key.keysym.sym == SDLK_CLEAR)) {
                inputBuffer.push_back('\x1B');
            }

            break;
        case SDL_TEXTINPUT:
            if (keyboardActive) {
                for(char* a_char = event.text.text; *a_char; ++a_char) {
                    inputBuffer.push_back(*a_char);
                }
            }
            break;
        case SDL_KEYUP:
            HandleKeyUp(keyMap[event.key.keysym.scancode]);
            break;
        case SDL_MOUSEBUTTONDOWN:
            if(event.button.button == SDL_BUTTON_RIGHT) {
                HandleKeyDown(keyMap[SDL_SCANCODE_APP1]);
            }
            else if(event.button.button == SDL_BUTTON_MIDDLE) {
                HandleKeyDown(keyMap[SDL_SCANCODE_APP2]);
            }
            else {
                buttonStatus |= kbuWentDown;
                buttonStatus |= kbuIsDown;
            }

            break;
        case SDL_MOUSEBUTTONUP:
            if(event.button.button == SDL_BUTTON_RIGHT) {
                HandleKeyUp(keyMap[SDL_SCANCODE_APP1]);
            }
            else if(event.button.button == SDL_BUTTON_MIDDLE) {
                HandleKeyUp(keyMap[SDL_SCANCODE_APP2]);
            }
            else {
                buttonStatus |= kbuWentUp;
                buttonStatus &= ~kbuIsDown;
            }
            break;
        case SDL_MOUSEMOTION:
            int xrel = event.motion.xrel, yrel = event.motion.yrel;
            xrel *= itsGame->sensitivity;
            yrel *= itsGame->sensitivity;
            if (itsGame->moJoOptions & kFlipAxis) {
                yrel = -yrel;
            }
            mouseX += xrel;
            mouseY += yrel;
            break;
    }
}

void CPlayerManagerImpl::HandleKeyDown(uint32_t keyFunc) {
    keysDown |= keyFunc;
    // only set the keyFunc bit in dupKeysHeld if the same bit in keysHeld is already set
    dupKeysHeld |= (keysHeld & keyFunc);
    keysHeld |= keyFunc;
    FPS_DEBUG("*** HandleKeyDown fn=" << itsGame->frameNumber <<
              std::hex << std::setfill('0') <<
              ", keyDown = 0x" << std::setw(8) << keyFunc <<
              ", keysHeld = 0x" << std::setw(8) << keysHeld <<
              ", dupKeysHeld = 0x" << std::setw(8) << dupKeysHeld << "\n" << std::dec);
}

void CPlayerManagerImpl::HandleKeyUp(uint32_t keyFunc) {
    keysUp |= (keyFunc & keysHeld);
    // if dupKeysHeld bit is already set, then keysHeld bit remains set until another key is released
    // note: this logic was added to support a more repeatable "superjump" capability isn't necessarily limited to jumping
    keysHeld &= ~(keyFunc & ~dupKeysHeld);
    // regardless of how many keys might be held, mark all duplicate keys as unheld
    // to avoid a bunch of KeyUp commands from causing strange behaviors (e.g. superDUPERjump)
    dupKeysHeld &= ~keyFunc;
    FPS_DEBUG("*** HandleKeyUp   fn=" << itsGame->frameNumber <<
              std::hex << std::setfill('0') <<
              ", keyUp   = 0x" << std::setw(8) << keyFunc <<
              ", keysHeld = 0x" << std::setw(8) << keysHeld <<
              ", dupKeysHeld = 0x" << std::setw(8) << dupKeysHeld << "\n" << std::dec);
}

void CPlayerManagerImpl::SendFrame() {
    // guard against overwriting frames that aren't done yet
    if (itsGame->topSentFrame >= itsGame->frameNumber + FUNCTIONBUFFERS - 1) {
        // this shouldn't happen because the latency maxes at 8.0*8(max frame rate) = 64 < FUNCTIONBUFFERS
        SDL_Log("frameFuncs BUFFER IS FULL, not sending new frames until we catch up! <-- tell Head you saw this.");
        return;
    }

    // Sends the next game frame.
    itsGame->topSentFrame += 1;

    FrameNumber ffi = itsGame->topSentFrame;
    FrameFunction *ff = &frameFuncs[(FUNCTIONBUFFERS - 1) & ffi];

    ff->validFrame = itsGame->topSentFrame;

    if (!keyboardActive) {
        ff->ft.down = keysDown;
        ff->ft.up = keysUp;
        ff->ft.held = keysHeld;
        ff->ft.msgChar = 0;
        prevKeyboardActive = keyboardActive;
    }
    else {
        ff->ft.down = 0;
        ff->ft.up = 0;
        ff->ft.held = 0;
        if (prevKeyboardActive == false) {
            prevKeyboardActive = true;
            ff->ft.down |= 1 << kfuTypeText;
        }
    }

    if (!inputBuffer.empty()) {
        ff->ft.msgChar = inputBuffer.front();
        inputBuffer.pop_front();
    }
    else {
        ff->ft.msgChar = 0;
    }

    ff->ft.mouseDelta.h = mouseX;
    ff->ft.mouseDelta.v = mouseY;
    ff->ft.buttonStatus = buttonStatus;

    // if in playback mode, update the FunctionTable right before sending the frame
    PlaybackAndRecord(ff->ft);

    CCommManager *theComm = theNetManager->itsCommManager.get();

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
        #define DONT_SEND_FRAME 0  // to help testing specific packet-loss cases, 1111 ~= 18sec
        #if DONT_SEND_FRAME > 0
            if (theNetManager->itsCommManager->myId == 1) {
                if (ffi >= DONT_SEND_FRAME) {
                    if (ffi < DONT_SEND_FRAME+1) {
                        outPacket->distribution &= ~1; // don't send packet from player 2 to player 1
                        // fake incrementing the serialNumber on the connection we aren't sending to
                        dynamic_cast<CUDPComm*>(theComm)->connections[0].serialNumber += kSerialNumberStepSize;
                    }
                    if (theNetManager->activePlayersDistribution & 1) {
                        // this should stop logging after player 1 aborts
                        SDL_Log("ffi = %u, distro = %hx\n", ffi, outPacket->distribution);
                    }
                }
            }
        #endif
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

void CPlayerManagerImpl::ResendFrame(FrameNumber theFrame, short requesterId, short commandCode) {
    CCommManager *theComm;
    PacketInfo *outPacket;
    FrameFunction *ff;

    theComm = theNetManager->itsCommManager.get();

    // short ffi = theFrame * itsGame->fpsScale;
    short ffi = theFrame;
    ff = &frameFuncs[(FUNCTIONBUFFERS - 1) & ffi];
    // ff = &frameFuncs[(FUNCTIONBUFFERS - 1) & theFrame];

    if (ff->validFrame == theFrame) {
        outPacket = theComm->GetPacket();
        if (outPacket) {
            // this method used by both requester and sender...
            if (commandCode == kpKeyAndMouseRequest) {
                SDL_Log("CPlayerManagerImpl::ResendFrame: asking for frame %d from slot %hd\n", theFrame, requesterId);
            } else {
                SDL_Log("CPlayerManagerImpl::ResendFrame: re-sending frame %d  to  slot %hd\n", theFrame, requesterId);
            }
            outPacket->command = commandCode;
            outPacket->distribution = 1 << requesterId;

            FrameFunctionToPacket(ff, outPacket, slot);

            outPacket->flags |= kpUrgentFlag;
            theComm->WriteAndSignPacket(outPacket);
        }
    } else //	Ask me later packet
    {
        SDL_Log("CPlayerManagerImpl::ResendFrame frame %d not in FUNCTIONBUFFERS, value = %d\n", theFrame, ff->validFrame);
        theComm->SendUrgentPacket(1 << requesterId, kpAskLater, 0, 0, theFrame, 0, 0);
    }
}

void CPlayerManagerImpl::ResumeGame() {
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
    SetLocal();

    keysDown = keysUp = keysHeld = dupKeysHeld = 0;
    mouseX = mouseY = 0;
    buttonStatus = 0;
}

void CPlayerManagerImpl::ProtocolHandler(struct PacketInfo *thePacket) {
    uint32_t *pd;
    uint16_t *spd;
    FrameFunction *ff;
    FrameNumber frameNumber;
    char p1;

    p1 = thePacket->p1;
    frameNumber = thePacket->p3;
    DBG_Log("q2", "inserting into FUNCTIONBUFFERS[%hd] << frame %d\n", slot, frameNumber);

    pd = (uint32_t *)thePacket->dataBuffer;
    FrameNumber ffi = frameNumber;
    ff = &frameFuncs[(FUNCTIONBUFFERS - 1) & ffi];
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

void CPlayerManagerImpl::ViewControl() {
    if (itsPlayer)
        itsPlayer->ControlViewPoint();
}

void CPlayerManagerImpl::Dispose() {
    CDirectObject::Dispose();
}

void CPlayerManagerImpl::SendResendRequest(short askCount) {
#if DONT_SEND_FRAME > 0
#else
    if (/* theNetManager->fastTrack.addr.value || */ askCount >= 0) {
        theNetManager->playerTable[theNetManager->itsCommManager->myId]->ResendFrame(
            itsGame->frameNumber, slot, kpKeyAndMouseRequest);
    }
#endif
}

size_t CPlayerManagerImpl::SkipLostPackets() {
    CUDPComm* theComm = dynamic_cast<CUDPComm*>(theNetManager->itsCommManager.get());
    size_t remaining = theComm->SkipLostPackets(slot);
    DBG_Log("q", "SkipLostPackets: remaining on queue[%d] = %zu", slot, remaining);
    return remaining;
}

FunctionTable *CPlayerManagerImpl::GetFunctions() {
    // SDL_Log("CPlayerManagerImpl::GetFunctions, %u, %hd\n", itsGame->frameNumber, slot);
    FrameNumber ffi = (itsGame->frameNumber);
    short i = (FUNCTIONBUFFERS - 1) & ffi;
    static int ASK_INTERVAL = MSEC_TO_TICK_COUNT(500);
    static int WAITING_MESSAGE_COUNT = 4;

    // if player is finished don't wait for their frames to sync up
    if (frameFuncs[i].validFrame != itsGame->frameNumber && itsPlayer->lives > 0) {
        long firstTime = askAgainTime = TickCount();
        long quickTick = firstTime;
        short askCount = 0;

        itsGame->didWait = true;

        if (frameFuncs[(FUNCTIONBUFFERS - 1) & (i + 1)].validFrame < itsGame->frameNumber) {
            // if next frame hasn't arrived yet, don't ask for resend on this one RIGHT away
            askAgainTime += 2 + (rand() & 3);  // 2-5 ticks = 33-83ms = 2.1-5.2 frames (16ms)
        }

        while (frameFuncs[i].validFrame < itsGame->frameNumber) {
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
                SendResendRequest(askCount++);
//                Debug::Toggle("nq");
                // if we get the packet from the Resend above, it might be stuck on the end of the readQ waiting for
                // a packet that is lost, so skip 1 lost packet at a time until it frees up the queue again
                SkipLostPackets();

                askAgainTime = quickTick + ASK_INTERVAL;

                if (askCount == WAITING_MESSAGE_COUNT) {
                    SDL_Log("Waiting for '%s' to resend frame #%u\n", GetPlayerName().c_str(), itsGame->frameNumber);
                    itsGame->itsApp->ParamLine(kmWaitingForPlayer, MsgAlignment::Center, playerName);
                    itsGame->itsApp->RenderContents();  // force render now so message shows up
                    // TODO: waiting for player dialog
                    // InitCursor();
                    // gApplication->SetCommandParams(STATUSSTRINGSLISTID, kmWaitingPlayers, true, 0);
                    // gApplication->BroadcastCommand(kBusyStartCmd);
                }

                SDL_Log("frameNumber = %d, validFrame = %d, askCount = %d, askAgainTime = %ld ticks\n",
                        itsGame->frameNumber, frameFuncs[i].validFrame, askCount, askAgainTime-quickTick);
            }

            // allow immediate abort after the kmWaitingForPlayer message displays
            if (askCount >= WAITING_MESSAGE_COUNT && TestKeyPressed(kfuAbortGame)) {
                itsGame->statusRequest = kAbortStatus;
                break;
            }
        }

        // give up after newer frames appear in the frameFuncs buffer because that
        // indicates the frame buffer has wrapped around and this frame won't be arriving
        // ... should probably only happen for spectators whose connections can't keep up
        if (frameFuncs[i].validFrame > itsGame->frameNumber) {
            itsGame->statusRequest = kAbortStatus;
            itsGame->itsApp->AddMessageLine("Exiting game - frame buffer full",
                                            MsgAlignment::Center, MsgCategory::Error);
        }


        if (askCount >= WAITING_MESSAGE_COUNT && frameFuncs[i].validFrame == itsGame->frameNumber) {
            gApplication->BroadcastCommand(kBusyEndCmd);
            itsGame->itsApp->AddMessageLine("...resuming game", MsgAlignment::Center);
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

void CPlayerManagerImpl::RosterKeyPress(unsigned char c) {
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

void	CPlayerManagerImpl::FlushMessageText(
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

bool EndsWithCheckmark(const char* c, size_t len) {
    static size_t checkLen = strlen(checkMark_utf8);
    return len >= checkLen &&
        strncmp(&c[len-checkLen], checkMark_utf8, checkLen) == 0;
}

void CPlayerManagerImpl::RosterMessageText(short len, const char *c) {
    // checkmark can come in as the utf8 3-byte string, look for it at the end of the message
    bool endsWithCheckmark = EndsWithCheckmark(c, len);
    while (len--) {
        unsigned char theChar;

        theChar = *c++;
        std::string chatText = GetChatLine();

        switch (theChar) {
            case 6:
                // ✓
                lineBuffer.insert(lineBuffer.end(), checkMark_utf8, checkMark_utf8 + strlen(checkMark_utf8));
                SetPlayerReady(true);
                break;
            case 7:
                // Δ
                lineBuffer.insert(lineBuffer.end(), triangle_utf8, triangle_utf8 + strlen(triangle_utf8));
                itsGame->itsApp->NotifyUser();
                break;
            case 8:
                if (lineBuffer.size()) {
                    auto i = lineBuffer.end();
                    try {
                        utf8::prior(i, lineBuffer.begin());
                    }
                    catch (const utf8::exception& utfcpp_ex) {
                        std::cerr << utfcpp_ex.what() << "\n";
                        lineBuffer.clear();
                    }
                    lineBuffer = std::deque<char>(lineBuffer.begin(), i);
                    // not ready if player deletes char
                    SetPlayerReady(false);
                }
                break;
            case 13:
                // ¬
                ((CAvaraAppImpl*)itsGame->itsApp)->rosterWindow->NewChatLine(playerName, GetChatLine());

                lineBuffer.insert(lineBuffer.end(), lThing_utf8, lThing_utf8 + strlen(lThing_utf8));
                // FlushMessageText(true);
                ((CAvaraAppImpl*)itsGame->itsApp)->GetTui()->ExecuteMatchingCommand(chatText, this);
                break;
            case 27:  // clearChat_utf8
                lineBuffer.clear();
                SetPlayerReady(false);
                break;
            default:
                if (theChar >= 32) {
                    lineBuffer.push_back(theChar);
                    if (lineBuffer.size() > 220) {
                        // FlushMessageText(true);
                        auto i = lineBuffer.begin();
                        try {
                            utf8::advance(i, 55, lineBuffer.end());
                        }
                        catch (const utf8::exception& utfcpp_ex) {
                            std::cerr << utfcpp_ex.what();
                            lineBuffer.clear();
                        }
                        lineBuffer = std::deque<char>(i, lineBuffer.end());
                    }
                    SetPlayerReady(endsWithCheckmark);
                }
                break;
        }
    }

    // FlushMessageText(false);
}

std::string CPlayerManagerImpl::GetChatLine() {
    std::string theChat(lineBuffer.begin(), lineBuffer.end());
    std::size_t found = theChat.find_last_of("\xC2\xAC", theChat.length() - 2);
    if(found == std::string::npos)
        found = 0;
    else
        found += 2;

    return theChat.substr(found);
}

std::string CPlayerManagerImpl::GetChatString(int maxChars) {
    std::string theChat(lineBuffer.begin(), lineBuffer.end());
    auto i = theChat.begin();
    int over;
    try {
        over = std::max((int)utf8::distance(theChat.begin(), theChat.end()) - maxChars, 0);
    }
    catch (const utf8::exception& utfcpp_ex) {
        // this happens usually because we try to access lineBuffer while a
        // utf8 charater is being added. the library doesn't like
        // half of a codepoint and throws an exception
        // TODO: Lock lineBuffer
        std::cerr << utfcpp_ex.what();
        over = 0;
    }
    if (over) {
        try {
            utf8::advance(i, over, theChat.end());
        }
        catch (const utf8::exception& utfcpp_ex) {
            std::cerr << utfcpp_ex.what();
            return theChat;
        }
    }
    return std::string(i, theChat.end());
}

bool CPlayerManagerImpl::CalculateIsLocalPlayer() {
    return theNetManager->itsCommManager->myId == slot;
}

void CPlayerManagerImpl::GameKeyPress(char theChar) {
    theNetManager->ReceiveRosterMessage(slot, 1, &theChar);
}

void CPlayerManagerImpl::NetDisconnect() {
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

void CPlayerManagerImpl::ChangeName(StringPtr theName) {
    StringPtr lastChar;

    if (loadingStatus == kLNotConnected) {
        loadingStatus = kLConnected;
        // theRoster->InvalidateArea(kOnePlayerBox, position);
    }

    if (strncmp((char*)&playerName[1], (char*)&theName[1], size_t(theName[0])) != 0) {
        SDL_Log("player #%d name set to '%.*s'\n", slot+1, int(theName[0]), &theName[1]);
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

void CPlayerManagerImpl::SetPosition(short pos) {
    position = pos;
    // theRoster->InvalidateArea(kUserBox, pos);
    // theRoster->InvalidateArea(kOnePlayerBox, pos);
}

void CPlayerManagerImpl::LoadStatusChange(short serverCRC, OSErr serverErr, std::string serverTag) {
    short oldStatus;

    if (loadingStatus != kLNotConnected && loadingStatus != kLActive && presence != kzAway)
    {
        oldStatus = loadingStatus;

        if (serverErr || levelErr) {
            if (levelErr == 1)
                loadingStatus = kLTrying;
            else if (serverErr == 1)
                loadingStatus = kLWaiting;
            else
                loadingStatus = kLNotFound;
        } else {
            if (serverCRC == levelCRC && serverTag.compare(levelTag) == 0) {
                SDL_Log("Setting loadingStatus = kLLoaded for slot = %d\n", slot);
                loadingStatus = kLLoaded;
#if 0
                oldHeld[0] = oldHeld[1] = 0;

                for(i=0;i<FUNCTIONBUFFERS;i++)
                {	frameFuncs[i].validFrame = -1;
                }
#endif
            } else {
                if (serverTag.compare(levelTag) == 0) {
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

CAbstractPlayer *CPlayerManagerImpl::ChooseActor(CAbstractPlayer *actorList, short myTeamColor) {
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
        itsPlayer = new CWalkerActor;
        itsPlayer->IAbstractActor();
        itsPlayer->BeginScript();
        ProgramLongVar(iTeam, myTeamColor);
        FreshCalc(); //	Parser call.
        itsPlayer->EndScript();
        itsPlayer->isInLimbo = true;
        itsPlayer->limboCount = 0;
        itsPlayer->itsManager = this;

        itsPlayer->teamMask = myTeamMask;
        itsPlayer->Reincarnate();

        if (itsPlayer->isInLimbo) {
            itsPlayer->Dispose();
            itsPlayer = NULL;
        } else {
            itsPlayer->AddToGame();
        }
    }

    return actorList;
}

Boolean CPlayerManagerImpl::IncarnateInAnyColor() {
    for (short i = 1; i <= kMaxTeamColors; i++) {
        itsPlayer = new CWalkerActor;
        itsPlayer->IAbstractActor();
        itsPlayer->BeginScript();
        ProgramLongVar(iTeam, i);
        FreshCalc(); //	Parser call.
        itsPlayer->EndScript();
        itsPlayer->isInLimbo = true;
        itsPlayer->limboCount = 0;
        itsPlayer->itsManager = this;

        itsPlayer->teamMask = 1 << i;  // set in case Incarnators discriminate on color
        itsPlayer->Reincarnate();

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

CAbstractPlayer *CPlayerManagerImpl::TakeAnyActor(CAbstractPlayer *actorList) {
    CAbstractPlayer *nextPlayer;

    nextPlayer = (CAbstractPlayer *)actorList->nextActor;

    playerColor = actorList->teamColor;

    itsPlayer = actorList;
    itsPlayer->itsManager = this;
    itsPlayer->AddToGame();

    return nextPlayer;
}
void CPlayerManagerImpl::SetPlayerStatus(LoadingState newStatus, PresenceType newPresence, FrameNumber theWin) {
    winFrame = theWin;

    loadingStatus = newStatus;
    presence = newPresence;
}

void CPlayerManagerImpl::SetPlayerReady(bool isReady) {
    // toggle between kLLoaded and kLReady but not to/from other states
    if (loadingStatus == kLLoaded && isReady) {
        loadingStatus = kLReady;
        itsGame->StartIfReady();
    } else if (loadingStatus == kLReady && !isReady) {
        loadingStatus = kLLoaded;
    }
}

bool CPlayerManagerImpl::IsAway() {
    return (presence == kzAway);
}

void CPlayerManagerImpl::AbortRequest() {
    theNetManager->activePlayersDistribution &= ~(1 << slot);
    if (isLocalPlayer) {
        itsGame->statusRequest = kAbortStatus;
    }
    // will call RemoveFromGame after game loop exits
}

void CPlayerManagerImpl::RemoveFromGame() {
    theNetManager->activePlayersDistribution &= ~(1 << slot);
    theNetManager->itsCommManager->SendUrgentPacket(kdEveryone, kpRemoveMeFromGame, 0, 0, 0, 0, 0);
}

void CPlayerManagerImpl::DeadOrDone() {
    theNetManager->deadOrDonePlayers |= 1 << slot;
}

short CPlayerManagerImpl::GetStatusChar() {
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

short CPlayerManagerImpl::GetMessageIndicator() {
    if (itsPlayer && itsPlayer->chatMode && loadingStatus == kLActive) {
        return 11;
    } else {
        return -1;
    }
}

void CPlayerManagerImpl::StoreMugShot(Handle mugHandle) {
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

Handle CPlayerManagerImpl::GetMugShot() {
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
void	CPlayerManagerImpl::GetLoadingStatusString(
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

void CPlayerManagerImpl::SpecialColorControl() {
    if (itsPlayer) {
        switch (spaceCount) {
            case 2:
                itsPlayer->SetSpecialColor(ColorManager::getSpecialBlackColor());
                break;
            case 3:
                itsPlayer->SetSpecialColor(ColorManager::getSpecialWhiteColor());
                break;
        }
    }
}

short CPlayerManagerImpl::Slot() {
    return slot;
}
void CPlayerManagerImpl::SetLocal() {
    isLocalPlayer = (theNetManager->itsCommManager->myId == slot);
    if (isLocalPlayer) {
        CPlayerManagerImpl::theLocalPlayer = this;
    }
}
Boolean CPlayerManagerImpl::IsLocalPlayer() {
    return isLocalPlayer;
}
short CPlayerManagerImpl::Position() {
    return position;
}
Str255& CPlayerManagerImpl::PlayerName() {
    return playerName;
}
std::string CPlayerManagerImpl::GetPlayerName() {
    return std::string((char *)playerName + 1, playerName[0]);
}
std::deque<char>& CPlayerManagerImpl::LineBuffer() {
    return lineBuffer;
}
CAbstractPlayer* CPlayerManagerImpl::GetPlayer() {
    return itsPlayer;
}
short CPlayerManagerImpl::IsRegistered() {
    return isRegistered;
}
void CPlayerManagerImpl::IsRegistered(short reg) {
    isRegistered = reg;
}
Str255& CPlayerManagerImpl::PlayerRegName() {
    return playerRegName;
}
LoadingState CPlayerManagerImpl::LoadingStatus() {
    return loadingStatus;
}
PresenceType CPlayerManagerImpl::Presence() {
    return presence;
}
void CPlayerManagerImpl::SetPresence(PresenceType newPresence) {
    presence = newPresence;
}
short CPlayerManagerImpl::LevelCRC() {
    return levelCRC;
}
OSErr CPlayerManagerImpl::LevelErr() {
    return levelErr;
}
std::string CPlayerManagerImpl::LevelTag() {
    return levelTag;
}
void CPlayerManagerImpl::LevelCRC(short crc) {
    levelCRC = crc;
}
void CPlayerManagerImpl::LevelErr(OSErr err) {
    levelErr = err;
}
void CPlayerManagerImpl::LevelTag(std::string t) {
    levelTag = t;
}
Fixed CPlayerManagerImpl::RandomKey() {
    return randomKey;
}
void CPlayerManagerImpl::RandomKey(Fixed rk) {
    randomKey = rk;
}
short CPlayerManagerImpl::PlayerColor() {
    return playerColor;
}
PlayerConfigRecord& CPlayerManagerImpl::TheConfiguration() {
    return theConfiguration;
}
Handle CPlayerManagerImpl::MugPict() {
    return mugPict;
}
void CPlayerManagerImpl::MugPict(Handle h) {
    mugPict = h;
}
void CPlayerManagerImpl::MugSize(long s) {
    mugSize = s;
}
void CPlayerManagerImpl::MugState(long s) {
    mugState = s;
}
long CPlayerManagerImpl::MugSize() {
    return mugSize;
}
long CPlayerManagerImpl::MugState() {
    return mugState;
}
FrameNumber CPlayerManagerImpl::WinFrame() {
    return winFrame;
}
void CPlayerManagerImpl::IncrementAskAgainTime(int amt) {
    askAgainTime += amt;
}
void CPlayerManagerImpl::SetShowScoreboard(bool b) {
    showScoreboard = b;
}
bool CPlayerManagerImpl::GetShowScoreboard() {
    return showScoreboard;
}

void CPlayerManagerImpl::PlaybackAndRecord(FunctionTable &ft) {
    if (isLocalPlayer && itsGame->keysFromStdin) {
        FunctionTable replayFt;
        // de-serialize the FunctionTable from stdin
        std::cin >> replayFt;
        // merge the playback FunctionTable in with the actual keyboard entry
        ft.merge(replayFt);
    }

    if (isLocalPlayer && itsGame->keysToStdout) {
        static bool headerWritten = false;
        if (!headerWritten) {
            std::cout << "# keysDn  keysUp  keysHeld ms.v ms.h btn\n";
            headerWritten = true;
        }
        // comment out any record that initiates a game pause
        if (TESTFUNC(kfuPauseGame, ft.down)) {
            std::cout << "#pause>";
        }
        // serialize the FunctionTable to stdout
        std::cout << ft;
    }
}
