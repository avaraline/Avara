#include "CRosterWindow.h"
#include "CAvaraApp.h"
//#include "CAvaraGame.h"
#include "CNetManager.h"
#include "CPlayerManager.h"
#include "CAbstractPlayer.h"
#include <nanogui/layout.h>
#include <nanogui/text.h>
#include <sstream>
#include <numeric>
using namespace nanogui;

std::vector<Text*> names;
std::vector<Text*> statuses;
std::vector<Text*> chats;

const int CHAT_CHARS = 63;
const int ROSTER_FONT_SIZE = 13;
bool textInputStarted = false;
char backspace[1] = {'\b'};
char clearline[1] = {'\027'};
char endline[2] = {'\108', ' '};
char bellline[1] = {'\175'};
char checkline[1] = {'\393'};

CRosterWindow::CRosterWindow(CApplication *app) : CWindow(app, "Roster") {
    AdvancedGridLayout *layout = new AdvancedGridLayout();
    setLayout(layout);

    for(int i = 0; i < kMaxAvaraPlayers; i++) {
        layout->appendRow(1, 1);
        layout->appendCol(1, 1);

        nanogui::Text* name = new Text(this, "", false, ROSTER_FONT_SIZE);
        layout->setAnchor(name, AdvancedGridLayout::Anchor(0, i * 2));
        nanogui::Text* status = new Text(this, "", false, ROSTER_FONT_SIZE);
        layout->setAnchor(status, AdvancedGridLayout::Anchor(1, i * 2));
        layout->appendRow(1, 1);
        layout->appendCol(1, 1);

        nanogui::Text* chat = new nanogui::Text(this, "", true, ROSTER_FONT_SIZE);
        layout->setAnchor(chat, AdvancedGridLayout::Anchor(0, i * 2 + 1, 2, 1));
        //layout->appendRow(1,1);
        //new nanogui::Label(this, "");

        name->setAlignment(Text::Alignment::Left);
        status->setAlignment(Text::Alignment::Right);
        chat->setAlignment(Text::Alignment::Left);
        name->setFont("mono");
        chat->setFont("mono");
        status->setFont("mono");
        name->setFixedWidth(225);
        status->setFixedWidth(225);

        //nameLabel->setVisible(false);
        //statusLabel->setVisible(false);
        //chatLabel->setVisible(false);
        names.push_back(name);
        statuses.push_back(status);
        chats.push_back(chat);
    }

    UpdateRoster();
}



CRosterWindow::~CRosterWindow() {
}

void CRosterWindow::UpdateRoster() {
    for(int i = 0; i < kMaxAvaraPlayers; i++) {
        CPlayerManager *thisPlayer = ((CAvaraApp *)gApplication)->gameNet->playerTable[i];

        const std::string theName((char *)thisPlayer->playerName + 1, thisPlayer->playerName[0]);

        short status = thisPlayer->loadingStatus;
        std::string theStatus = GetStringStatus(status, thisPlayer->winFrame);

        std::string theChat(thisPlayer->lineBuffer.begin(), thisPlayer->lineBuffer.end());
        if (theChat.length() > CHAT_CHARS) {
            theChat = theChat.substr(theChat.length() - CHAT_CHARS, CHAT_CHARS);
        }

        names[i]->setValue(theName.c_str());
        statuses[i]->setValue(theStatus.c_str());
        chats[i]->setValue(theChat.c_str());
    }
}


bool CRosterWindow::DoCommand(int theCommand) {
    return false;
}

std::string CRosterWindow::GetStringStatus(short status, Fixed winFrame) {

    std::string strStatus;
    if (winFrame >= 0) {
        long timeTemp = FMulDiv(winFrame, ((CAvaraApp *)gApplication)->itsGame->frameTime, 10);
        auto hundreds1 = timeTemp % 10;
        timeTemp /= 10;
        auto hundreds2 = timeTemp % 10;
        timeTemp /= 10;
        auto secs1 = timeTemp % 10;
        timeTemp /= 10;
        auto secs2 = timeTemp % 6;
        timeTemp /= 6;

        std::ostringstream os;
        os << "[" << timeTemp
        << ":" << secs2 << secs1
        << "." << hundreds2 << hundreds1
        << "]";

        strStatus = os.str();
        return strStatus;
    }

    if (status == kLConnected) {
        strStatus = "connected";
    }
    else if (status == kLLoaded) {
        strStatus = "ready";
    }
    else if (status == kLWaiting) {
        strStatus = "waiting";
    }
    else if (status == kLTrying) {
        strStatus = "loading";
    }
    else if (status == kLMismatch) {
        strStatus = "version mismatch";
    }
    else if (status == kLNotFound) {
        strStatus = "level not found";
    }
    else if (status == kLPaused) {
        strStatus = "paused";
    }
    else if (status == kLActive) {
        strStatus = "active";
    }
    else if (status == kLNoVehicle) {
        strStatus = "HECTOR not available";
    }
    else {
        strStatus = "";
    }
    return strStatus;
}

bool CRosterWindow::mouseEnterEvent(const nanogui::Vector2i &p, bool enter) {
    if (enter && !textInputStarted) {
        SDL_StartTextInput();
        textInputStarted = true;
    }
    if (!enter && textInputStarted) {
        SDL_StartTextInput();
        textInputStarted = false;
    }
    return true;
};

void CRosterWindow::SendRosterMessage(int len, char* message) {
    ((CAvaraApp *)gApplication)->gameNet->SendRosterMessage(len, message);
}

bool CRosterWindow::handleSDLEvent(SDL_Event &event) {
    if(!textInputStarted) return false;
    if (event.type == SDL_TEXTINPUT) {
        SendRosterMessage(strlen(event.text.text), event.text.text);
        return true;
    }
    else if (event.type == SDL_KEYDOWN) {
        switch(event.key.keysym.sym) {
            case SDLK_BACKSPACE:
                SendRosterMessage(1, backspace);
                return true;
            case SDLK_RETURN:
                SendRosterMessage(2, endline);
                return true;
            case SDLK_CLEAR:
            case SDLK_DELETE:
                SendRosterMessage(1, clearline);
                return true;
            case SDLK_g:
                if (SDL_GetModState() & KMOD_CTRL) {
                    SendRosterMessage(1, bellline);
                    return true;
                }
                else return false;
            case SDLK_v:
                if (SDL_GetModState() & KMOD_ALT) {
                    SendRosterMessage(1, checkline);
                    return true;
                }
                else return false;
            default:
                return false;
        }
    }
    else {
        return false;
    }
}