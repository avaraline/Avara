#include "CRosterWindow.h"

#include "AvaraDefines.h"
#include "CAbstractPlayer.h"
#include "CAvaraApp.h"
#include "CColorManager.h"
#include "CNetManager.h"
#include "CPlayerManager.h"
#include "CScoreKeeper.h"
#include "Preferences.h"
#include "RGBAColor.h"

#include <nanogui/colorcombobox.h>
#include <nanogui/layout.h>
#include <numeric>
#include <sstream>
#include <stdint.h>
#include <nanogui/tabwidget.h>
using namespace nanogui;

std::vector<Text *> statuses;
std::vector<Text *> chats;
std::vector<ColorComboBox *> colors;
std::string currentLevel;
TabWidget *tabWidget;


std::vector<Text *> scoreTeams;
std::vector<Text *> scoreNames;
std::vector<Text *> scoreExitRanks;
std::vector<Text *> scoreScores;
std::vector<Text *> scoreKills;
std::vector<Text *> scoreLives;

const int CHAT_CHARS = 57;
const int ROSTER_FONT_SIZE = 15;
const int ROSTER_WINDOW_WIDTH = 470;
const int SCORE_FONT_SIZE = 16;
char backspace[1] = {'\b'};
char clearline[1] = {'\x1B'};
char endline[1] = {13};
char bellline[1] = {7};
char checkline[1] = {6};
Widget *chatPanel;

CRosterWindow::CRosterWindow(CApplication *app) : CWindow(app, "Roster") {
    setFixedWidth(470);

    tabWidget = add<TabWidget>();

    //players tab
    Widget *playersLayer = tabWidget->createTab("Players");

    setLayout(new BoxLayout(Orientation::Vertical, Alignment::Fill, 0, 0));

    BoxLayout *blayout = new BoxLayout(Orientation::Vertical, Alignment::Fill, 0, 0);
    AdvancedGridLayout *layout = new AdvancedGridLayout();
    playersLayer->setLayout(blayout);
    auto panel = playersLayer->add<Widget>();
    panel->setLayout(layout);
    theNet = ((CAvaraAppImpl *)gApplication)->GetNet();
    std::vector<long> player_colors = {
        (long) CColorManager::getTeamColor(1).value_or(CColorManager::getDefaultTeamColor()),
        (long) CColorManager::getTeamColor(2).value_or(CColorManager::getDefaultTeamColor()),
        (long) CColorManager::getTeamColor(3).value_or(CColorManager::getDefaultTeamColor()),
        (long) CColorManager::getTeamColor(4).value_or(CColorManager::getDefaultTeamColor()),
        (long) CColorManager::getTeamColor(5).value_or(CColorManager::getDefaultTeamColor()),
        (long) CColorManager::getTeamColor(6).value_or(CColorManager::getDefaultTeamColor()),
        (long) CColorManager::getTeamColor(7).value_or(CColorManager::getDefaultTeamColor()),
        (long) CColorManager::getTeamColor(8).value_or(CColorManager::getDefaultTeamColor())
    };

    for (int i = 0; i < kMaxAvaraPlayers; i++) {
        layout->appendRow(1, 1);
        layout->appendCol(1, 1);
        ColorComboBox *color = panel->add<ColorComboBox>(player_colors);
        //color->setFixedHeight(23);
        color->setSelectedIndex(theNet->teamColors[i]);
        color->setCallback([this, color, i](int selectedIdx) {
            theNet->teamColors[i] = selectedIdx;
            theNet->SendColorChange();
        });
        color->popup()->setSize(nanogui::Vector2i(50, 38 * player_colors.size()));
        layout->setAnchor(color, AdvancedGridLayout::Anchor(0, i * 2));
        Text *status = panel->add<Text>("", false, ROSTER_FONT_SIZE + 2);
        layout->setAnchor(status, AdvancedGridLayout::Anchor(1, i * 2));
        layout->appendRow(1, 1);
        layout->appendCol(1, 1);

        nanogui::Text *chat = panel->add<Text>("", false, ROSTER_FONT_SIZE);
        layout->setAnchor(chat, AdvancedGridLayout::Anchor(0, i * 2 + 1, 2, 1));

        status->setAlignment(Text::Alignment::Right);
        chat->setAlignment(Text::Alignment::Left);

        chat->setFont("mono");

        status->setFixedWidth(120);
        chat->setFixedWidth(550);
        chat->setFixedHeight(30);
        color->setFixedWidth(225);

        statuses.push_back(status);
        chats.push_back(chat);
        colors.push_back(color);
    }

    levelLoaded = playersLayer->add<Text>("", false, 16);
    levelDesigner = playersLayer->add<Text>("", false, 16);
    levelDescription = playersLayer->add<Label>("No level loaded");

    levelLoaded->setAlignment(Text::Alignment::Left);
    levelDesigner->setAlignment(Text::Alignment::Left);
    //levelDescription->setAlignment(Text::Alignment::Left);
    levelDescription->setFixedHeight(90);
    levelDescription->setFixedWidth(450);

    //chat tab
    Widget *chatTab = tabWidget->createTab("Chat");
    chatTab->setLayout(new GridLayout(Orientation::Horizontal, 1, Alignment::Minimum, 0, 0));

    VScrollPanel *scrollPanel = new VScrollPanel(chatTab);
    scrollPanel->setFixedWidth(ROSTER_WINDOW_WIDTH);
    scrollPanel->setFixedHeight(500);

    chatPanel = new Widget(scrollPanel);
    AdvancedGridLayout *chatLayout = new AdvancedGridLayout();
    chatPanel->setLayout(chatLayout);

    //placeholder lines for now
    for (int i = 0; i < 20; i++) {
        chatLayout->appendRow(1, 0.1);
        chatLayout->appendCol(1, 1);

        auto chatLine = chatPanel->add<Label>("");
        chatLine->setFontSize(ROSTER_FONT_SIZE + 2);
        chatLine->setFont("mono");
        chatLine->setFixedWidth(ROSTER_WINDOW_WIDTH - 20);
        chatLine->setFixedHeight(20);

        chatLayout->setAnchor(chatLine, AdvancedGridLayout::Anchor(0, i));
    }

    //chat input
    std::string theName = ((CAvaraAppImpl *)gApplication)->String(kPlayerNameTag);
    chatInput = chatTab->add<Label>(theName + ": ");
    chatInput->setFontSize(ROSTER_FONT_SIZE + 2);
    chatInput->setFont("mono");
    chatInput->setFixedWidth(ROSTER_WINDOW_WIDTH - 20);
    chatInput->setFixedHeight(70);

    //scores tab
    Widget *scoreLayer = tabWidget->createTab("Scores");
    scoreLayer->setLayout(new GridLayout(Orientation::Horizontal, 6, Alignment::Minimum, 0, 0));

    std::vector<std::string> headings = {
        "Team",
        "Name",
        "XP",
        "Score",
        "Kills",
        "Lives"
    };

    for(const auto &heading : headings) {
        auto t = scoreLayer->add<Text>(heading, false, SCORE_FONT_SIZE);
    }

    for (int i = 0; i < kMaxAvaraPlayers; i++) {
        scoreTeams.push_back(scoreLayer->add<Text>("", false, SCORE_FONT_SIZE));
        auto nameText = scoreLayer->add<Text>("", false, SCORE_FONT_SIZE);
        nameText->setFixedWidth(150);
        scoreNames.push_back(nameText);
        scoreExitRanks.push_back(scoreLayer->add<Text>("", false, SCORE_FONT_SIZE));
        auto scoreText = scoreLayer->add<Text>("", false, SCORE_FONT_SIZE);
        scoreText->setFixedWidth(75);
        scoreScores.push_back(scoreText);
        scoreKills.push_back(scoreLayer->add<Text>("", false, SCORE_FONT_SIZE));
        scoreLives.push_back(scoreLayer->add<Text>("", false, SCORE_FONT_SIZE));
    }

    tabWidget->setActiveTab(0);
    currentLevel = ((CAvaraAppImpl *)gApplication)->GetGame()->loadedTag;

    UpdateRoster();

    requestFocus();
}

CRosterWindow::~CRosterWindow() {}

void CRosterWindow::UpdateRoster() {
    CAvaraGame *theGame = ((CAvaraAppImpl *)gApplication)->GetGame();
    if (tabWidget->activeTab() == 0) {
        long maxRtt = 0;
        for (int i = 0; i < kMaxAvaraPlayers; i++) {
            CPlayerManager *thisPlayer = ((CAvaraAppImpl *)gApplication)->GetNet()->playerTable[i];

            std::string theName((char *)thisPlayer->PlayerName() + 1, thisPlayer->PlayerName()[0]);
            if (i != theNet->itsCommManager->myId && theName.length() > 0) {
                long rtt = theNet->itsCommManager->GetMaxRoundTrip(1 << i);
                theName += std::string(" (") + std::to_string(rtt) + " ms)";
                maxRtt = std::max(maxRtt, rtt);
            }
            std::string theStatus = GetStringStatus(thisPlayer);

            std::string theChat = thisPlayer->GetChatString(CHAT_CHARS);

            statuses[i]->setValue(theStatus.c_str());
            chats[i]->setValue(theChat.c_str());
            colors[i]->setSelectedIndex(theNet->teamColors[i]);
            colors[i]->setTextColor(nanogui::Color(
                LongToR(*CColorManager::getTeamTextColor(theNet->teamColors[i] + 1)),
                LongToG(*CColorManager::getTeamTextColor(theNet->teamColors[i] + 1)),
                LongToB(*CColorManager::getTeamTextColor(theNet->teamColors[i] + 1)),
                LongToA(*CColorManager::getTeamTextColor(theNet->teamColors[i] + 1))
            ));
            colors[i]->setCaption(theName.c_str());
            colors[i]->popup()->setAnchorPos(nanogui::Vector2i(235, 68 + 60 * i));
        }

        if (maxRtt > 0 && theNet->IsAutoLatencyEnabled() && !theGame->IsPlaying()) {
            // set initial frame latency from client ping/RTT times
            maxRtt = std::min(maxRtt, long(CLASSICFRAMETIME*2*4));  // max of 4 LT on the UI
            theGame->SetFrameLatency(theGame->RoundTripToFrameLatency(maxRtt), -1);
        }

        if (theGame->loadedTag.compare(currentLevel) != 0) {
            std::string theLevel = theGame->loadedLevel;
            std::string theDesigner = theGame->loadedDesigner;

            if (theLevel.length() > 0) levelLoaded->setValue(theLevel);
            else levelLoaded->setValue("");
            if (theDesigner.length() > 0) levelDesigner->setValue(theDesigner);
            else levelDesigner->setValue("");

            if (theGame->loadedInfo.length() > 0) levelDescription->setCaption(theGame->loadedInfo);
            else levelDescription->setCaption("No additional information about this mission is available.");
            currentLevel = theGame->loadedTag;
        }
    }
    else if (tabWidget->activeTab() == 2) {
        for (int i = 0; i < kMaxAvaraPlayers; i++) {
            CPlayerManager *thisPlayer = theNet->playerTable[i];
            const std::string theName((char *)thisPlayer->PlayerName() + 1, thisPlayer->PlayerName()[0]);
            AvaraScoreRecord theScores = theGame->scoreKeeper->netScores;
            if(theName.size() > 0) {
                scoreTeams[i]->setValue(std::to_string(theNet->teamColors[i]));
                scoreNames[i]->setValue(theName.c_str());
                scoreExitRanks[i]->setValue(std::to_string(theScores.player[i].exitRank));
                scoreScores[i]->setValue(std::to_string(theScores.player[i].points));
                scoreKills[i]->setValue(std::to_string(theScores.player[i].kills));
                scoreLives[i]->setValue(std::to_string(theScores.player[i].lives));
            }
            else {
                scoreTeams[i]->setValue("");
                scoreNames[i]->setValue("");
                scoreExitRanks[i]->setValue("");
                scoreScores[i]->setValue("");
                scoreKills[i]->setValue("");
                scoreLives[i]->setValue("");
            }
        }
    }
}

bool CRosterWindow::DoCommand(int theCommand) {
    return false;
}

std::string CRosterWindow::GetStringStatus(CPlayerManager *player) {
    short status = player->LoadingStatus();
    std::string strStatus;
    if (player->WinFrame() >= 0) {
        long timeTemp = FMulDiv(player->WinFrame(), ((CAvaraAppImpl *)gApplication)->GetGame()->frameTime, 10);
        auto hundreds1 = timeTemp % 10;
        timeTemp /= 10;
        auto hundreds2 = timeTemp % 10;
        timeTemp /= 10;
        auto secs1 = timeTemp % 10;
        timeTemp /= 10;
        auto secs2 = timeTemp % 6;
        timeTemp /= 6;

        std::ostringstream os;
        os << "[" << timeTemp << ":" << secs2 << secs1 << "." << hundreds2 << hundreds1 << "]";

        strStatus = os.str();
        return strStatus;
    }

    if(player->Active() == false) {
        strStatus = "not playing";
    } else if (status == kLConnected) {
        strStatus = "connected";
    } else if (status == kLLoaded) {
        strStatus = "ready";
    } else if (status == kLWaiting) {
        strStatus = "waiting";
    } else if (status == kLTrying) {
        strStatus = "loading";
    } else if (status == kLMismatch) {
        strStatus = "version mismatch";
    } else if (status == kLNotFound) {
        strStatus = "level not found";
    } else if (status == kLPaused) {
        strStatus = "paused";
    } else if (status == kLActive) {
        strStatus = "active";
    } else if (status == kLNoVehicle) {
        strStatus = "HECTOR not available";
    } else {
        strStatus = "";
    }
    return strStatus;
}

void CRosterWindow::SendRosterMessage(int len, char *message) {
    ((CAvaraAppImpl *)gApplication)->GetNet()->SendRosterMessage(len, message);
    chatInput->setCaption(chatInput->caption() + message);
}

void CRosterWindow::ChatLineDelete() {
    chatInput->setCaption(chatInput->caption().substr(0, chatInput->caption().size() - 1));
}

void CRosterWindow::NewChatLine(Str255 playerName, std::string message) {
    std::string name = std::string((char *)playerName + 1, playerName[0]);
    std::string chatLine = name + ": " +  message;

    AdvancedGridLayout *gridLayout = (AdvancedGridLayout*) chatPanel->layout();
    gridLayout->appendRow(1, 0.1);
    gridLayout->appendCol(1, 1);

    auto chatLabel = chatPanel->add<Label>(chatLine);
    chatLabel->setFontSize(ROSTER_FONT_SIZE + 2);
    chatLabel->setFont("mono");
    chatLabel->setFixedWidth(ROSTER_WINDOW_WIDTH - 20);

    gridLayout->setAnchor(chatLabel, AdvancedGridLayout::Anchor(0, gridLayout->rowCount() - 1));

    std::string theName = ((CAvaraAppImpl *)gApplication)->String(kPlayerNameTag);
    Screen* screen = chatLabel->screen();
    NVGcontext* context = screen->nvgContext();
    chatLabel->parent()->performLayout(context);
    chatInput->setCaption(theName + ": ");

    VScrollPanel *scroll = (VScrollPanel*)chatPanel->parent();
    scroll->setScroll(1);
}

bool CRosterWindow::handleSDLEvent(SDL_Event &event) {
    if (event.type == SDL_TEXTINPUT) {
        #ifndef __APPLE__
        // we already sent a checkmark, don't send a v
        if ((SDL_GetModState() & KMOD_ALT) && strcmp(event.text.text, "v") == 0)
            return true;
        #endif
        SendRosterMessage(strlen(event.text.text), event.text.text);
        return true;
    } else if (event.type == SDL_KEYDOWN) {
        //SDL_Log("CRosterWindow::handleSDLEvent SDL_KEYDOWN");

        switch (event.key.keysym.sym) {
            case SDLK_UP:
                ((CAvaraAppImpl *)gApplication)->ChatCommandHistoryUp();
                
                return true;
            case SDLK_DOWN:
                ((CAvaraAppImpl *)gApplication)->ChatCommandHistoryDown();

                return true;
            case SDLK_BACKSPACE:
                SendRosterMessage(1, backspace);
                ChatLineDelete();
                //SDL_Log("CRosterWindow::handleSDLEvent BACK");

                return true;
            case SDLK_RETURN:
                SendRosterMessage(1, endline);
                return true;
            case SDLK_CLEAR:
            case SDLK_DELETE:
                SendRosterMessage(1, clearline);
                //SDL_Log("CRosterWindow::handleSDLEvent CLEAR");

                return true;
            case SDLK_g:
                if (SDL_GetModState() & KMOD_CTRL) {
                    SendRosterMessage(1, bellline);
                    return true;
                } else
                    return false;
            #ifndef __APPLE__
            case SDLK_v:
                if (SDL_GetModState() & KMOD_ALT) {
                    SendRosterMessage(1, checkline);
                    return true;
                } else
                    return false;
            #endif
            default:
                return false;
        }
    } else {
        return false;
    }
}
