#include "CRosterWindow.h"
#include "CAvaraApp.h"
//#include "CAvaraGame.h"
#include "CNetManager.h"
#include "CPlayerManager.h"
#include "CAbstractPlayer.h"
#include <nanogui/layout.h>
#include <nanogui/text.h>
#include <numeric>
using namespace nanogui;

std::vector<Text*> names;
std::vector<Text*> statuses;
std::vector<Text*> chats;

CRosterWindow::CRosterWindow(CApplication *app) : CWindow(app, "Roster") {
    AdvancedGridLayout *layout = new AdvancedGridLayout();
    setLayout(layout);

    for(int i = 0; i < kMaxAvaraPlayers; i++) {
        layout->appendRow(1, 1);
        layout->appendCol(1, 1);

        nanogui::Text* name = new Text(this, "");
        layout->setAnchor(name, AdvancedGridLayout::Anchor(0, i * 2));
        nanogui::Text* status = new Text(this, "");
        layout->setAnchor(status, AdvancedGridLayout::Anchor(1, i * 2));
        layout->appendRow(1, 1);
        layout->appendCol(1, 1);
        
        nanogui::Text* chat = new nanogui::Text(this, "a very very very long very long stirn gits vyer long string string string");
        layout->setAnchor(chat, AdvancedGridLayout::Anchor(0, i * 2 + 1, 2, 1));
        //layout->appendRow(1,1);
        //new nanogui::Label(this, "");

        name->setAlignment(Text::Alignment::Left);
        status->setAlignment(Text::Alignment::Right);
        chat->setAlignment(Text::Alignment::Left);
        name->setFixedWidth(200);
        status->setFixedWidth(200);
        //chatLabel->setFixedWidth(560);

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
        std::string theStatus = GetStringStatus(status);

        std::string theChat((char *)thisPlayer->lineBuffer + 1, thisPlayer->lineBuffer[0]);

        names[i]->setValue(theName.c_str());
        statuses[i]->setValue(theStatus.c_str());
        chats[i]->setValue(theChat.c_str());
    }
}


bool CRosterWindow::DoCommand(int theCommand) {
    return false;
}

std::string CRosterWindow::GetStringStatus(short status) {
    std::string strStatus;

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

    //const char *fmt = "(%s)";
    //int size = std::snprintf(nullptr, 0, fmt, strStatus);
    //std::vector<char> buffa(size+1);
    //std::snprintf(&buffa[0], buffa.size(), fmt, strStatus);
    //std::string s(buffa.begin(), buffa.end());
    return strStatus;
}