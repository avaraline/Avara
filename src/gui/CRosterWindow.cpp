#include "CRosterWindow.h"

#include "AvaraDefines.h"
#include "CAbstractPlayer.h"
#include "CAvaraApp.h"
#include "ColorManager.h"
#include "CNetManager.h"
#include "CPlayerManager.h"
#include "CScoreKeeper.h"
#include "Preferences.h"
#include "ARGBColor.h"
#include "Debug.h"

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
    std::vector<long> colorOptions = {
        static_cast<long>(ColorManager::getTeamColor(1).value_or(ColorManager::getDefaultTeamColor()).GetRaw()),
        static_cast<long>(ColorManager::getTeamColor(2).value_or(ColorManager::getDefaultTeamColor()).GetRaw()),
        static_cast<long>(ColorManager::getTeamColor(3).value_or(ColorManager::getDefaultTeamColor()).GetRaw()),
        static_cast<long>(ColorManager::getTeamColor(4).value_or(ColorManager::getDefaultTeamColor()).GetRaw()),
        static_cast<long>(ColorManager::getTeamColor(5).value_or(ColorManager::getDefaultTeamColor()).GetRaw()),
        static_cast<long>(ColorManager::getTeamColor(6).value_or(ColorManager::getDefaultTeamColor()).GetRaw()),
        static_cast<long>(ColorManager::getTeamColor(7).value_or(ColorManager::getDefaultTeamColor()).GetRaw()),
        static_cast<long>(ColorManager::getTeamColor(8).value_or(ColorManager::getDefaultTeamColor()).GetRaw())
    };

    for (int i = 0; i < kMaxAvaraPlayers; i++) {
        layout->appendRow(1, 1);
        layout->appendCol(1, 1);
        ColorComboBox *color = panel->add<ColorComboBox>(colorOptions);
        //color->setFixedHeight(23);
        color->setSelectedIndex(theNet->teamColors[i]);
        color->setCallback([this, i](int selectedIdx) {
            theNet->teamColors[i] = selectedIdx;
            theNet->SendColorChange();
        });
        color->popup()->setSize(nanogui::Vector2i(50, 38 * colorOptions.size()));
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
        chatLayout->appendRow(1, 0.1f);
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
        scoreLayer->add<Text>(heading, false, SCORE_FONT_SIZE);
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
    currentLevel = ((CAvaraAppImpl *)gApplication)->GetGame()->loadedFilename;

    UpdateRoster();

    requestFocus();
}

CRosterWindow::~CRosterWindow() {}

void CRosterWindow::UpdateRoster() {
    CAvaraGame *theGame = ((CAvaraAppImpl *)gApplication)->GetGame();
    if (tabWidget->activeTab() == 0) {
        long maxRtt = 0;
        for (int i = 0; i < kMaxAvaraPlayers; i++) {
            CPlayerManager *thisPlayer = ((CAvaraAppImpl *)gApplication)->GetNet()->playerTable[i].get();

            std::string theName((char *)thisPlayer->PlayerName() + 1, thisPlayer->PlayerName()[0]);
            if (i != theNet->itsCommManager->myId && theName.length() > 0) {
                long rtt = theNet->itsCommManager->GetMaxRoundTrip(1 << i);
                float loss = 100*theNet->itsCommManager->GetMaxMeanReceiveCount(1 << i);
                std::ostringstream os;
                os << theName << " (" << rtt << "ms";
                if (Debug::IsEnabled("loss")) {
                    os << "/" << std::setprecision(loss < 2 ? 1 : 0) << std::fixed << loss << "%";
                }
                os << ")";
                theName = os.str();
                maxRtt = std::max(maxRtt, rtt);
            }
            std::string theStatus = GetStringStatus(thisPlayer);

            std::string theChat = thisPlayer->GetChatString(CHAT_CHARS);

            statuses[i]->setValue(theStatus.c_str());
            chats[i]->setValue(theChat.c_str());
            colors[i]->setSelectedIndex(theNet->teamColors[i]);
            colors[i]->setTextColor(nanogui::Color(
                (*ColorManager::getTeamTextColor(theNet->teamColors[i] + 1)).GetR(),
                (*ColorManager::getTeamTextColor(theNet->teamColors[i] + 1)).GetG(),
                (*ColorManager::getTeamTextColor(theNet->teamColors[i] + 1)).GetB(),
                (*ColorManager::getTeamTextColor(theNet->teamColors[i] + 1)).GetA()
            ));
            colors[i]->setCaption(theName.c_str());
            colors[i]->popup()->setAnchorPos(nanogui::Vector2i(235, 68 + 60 * i));
        }

        if (maxRtt > 0 && theNet->IsAutoLatencyEnabled() && !theGame->IsPlaying()) {
            // set initial frame latency from client ping/RTT times
            maxRtt = std::min(maxRtt+CLASSICFRAMETIME, long(CLASSICFRAMETIME*2*4));  // max of 4 LT on the UI
            theGame->SetFrameLatency(theGame->RoundTripToFrameLatency(maxRtt), -1);
        }

        if (theGame->loadedFilename.compare(currentLevel) != 0) {
            std::string theLevel = theGame->loadedLevel;
            std::string theDesigner = theGame->loadedDesigner;

            if (theLevel.length() > 0) levelLoaded->setValue(theLevel);
            else levelLoaded->setValue("");
            if (theDesigner.length() > 0) levelDesigner->setValue(theDesigner);
            else levelDesigner->setValue("");

            if (theGame->loadedInfo.length() > 0) levelDescription->setCaption(theGame->loadedInfo);
            else levelDescription->setCaption("No additional information about this mission is available.");
            currentLevel = theGame->loadedFilename;
        }
    }
    else if (tabWidget->activeTab() == 2) {
        for (int i = 0; i < kMaxAvaraPlayers; i++) {
            AvaraScoreRecord theScores = theGame->scoreKeeper->netScores;
            scoreNames[i]->setValue(theGame->scoreKeeper->playerNames[i]);
            if(theScores.player[i].lives >= 0) {
                scoreTeams[i]->setValue(std::to_string(theNet->teamColors[i]));
                scoreExitRanks[i]->setValue(std::to_string(theScores.player[i].exitRank));
                scoreScores[i]->setValue(std::to_string(theScores.player[i].points));
                scoreKills[i]->setValue(std::to_string(theScores.player[i].kills));
                scoreLives[i]->setValue(std::to_string(theScores.player[i].lives));
            }
            else {
                scoreTeams[i]->setValue("");
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

    LoadingState status = player->LoadingStatus();
    PresenceType presence = player->Presence();
    if (presence != kzAway) {
        if (status == kLConnected) {
            strStatus = "connected";
        } else if (status == kLLoaded) {
            strStatus = "loaded";
        } else if (status == kLReady) {
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
    } else if (status != kLNotConnected) {
        strStatus = "away";
    }
    if (presence == kzSpectating) {
        if (player->LoadingStatusIsIn(kLConnected, kLActive, kLReady, kLLoaded, kLPaused)) {
            strStatus = "spectator";
        } else if (strStatus.length() > 0) {
            strStatus += "*";   // make this into an eyeball char?
        }
    }
    return strStatus;
}

void CRosterWindow::SendRosterMessage(std::string& message) {
    SendRosterMessage(message.length(), (char*)message.c_str());
}

void CRosterWindow::SendRosterMessage(const char* message) {
    // message assumed to be NULL terminated
    SendRosterMessage(strlen(message), (char*)message);
}

void CRosterWindow::SendRosterMessage(size_t len, char *message) {
    ((CAvaraAppImpl *)gApplication)->GetNet()->SendRosterMessage(len, message);
    std::string newCaption = chatInput->caption();
    newCaption.append(message, len);  // message not always null-terminated, use len!
    chatInput->setCaption(newCaption);
}

void CRosterWindow::ChatLineDelete() {
    chatInput->setCaption(chatInput->caption().substr(0, chatInput->caption().size() - 2));
}

void CRosterWindow::ResetChatPrompt() {
    std::string theName = ((CAvaraAppImpl *)gApplication)->String(kPlayerNameTag);
    chatInput->setCaption(ChatPromptFor(theName));
}
std::string CRosterWindow::ChatPromptFor(std::string theName) {
    int len = 9;
    std::string paddedName = theName + "        ";
    return paddedName.substr(0, len) + ": ";
}
void CRosterWindow::NewChatLine(Str255 playerName, std::string message) {
    std::string name = std::string((char *)playerName + 1, playerName[0]);
    std::string chatLine = ChatPromptFor(name) + message;

    AdvancedGridLayout *gridLayout = (AdvancedGridLayout*) chatPanel->layout();
    gridLayout->appendRow(1, 0.1f);
    gridLayout->appendCol(1, 1);

    auto chatLabel = chatPanel->add<Label>(chatLine);
    chatLabel->setFontSize(ROSTER_FONT_SIZE + 2);
    chatLabel->setFont("mono");
    chatLabel->setFixedWidth(ROSTER_WINDOW_WIDTH - 20);

    gridLayout->setAnchor(chatLabel, AdvancedGridLayout::Anchor(0, gridLayout->rowCount() - 1));
    ResetChatPrompt();

    Screen* screen = chatLabel->screen();
    NVGcontext* context = screen->nvgContext();
    chatLabel->parent()->performLayout(context);

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
                SendRosterMessage(1, clearline);
                ResetChatPrompt();
                ((CAvaraAppImpl *)gApplication)->GetTui()->HistoryOlder();

                return true;
            case SDLK_DOWN:
                SendRosterMessage(1, clearline);
                ResetChatPrompt();
                ((CAvaraAppImpl *)gApplication)->GetTui()->HistoryNewer();

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
                ResetChatPrompt();
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

void CRosterWindow::PrefChanged(std::string name) {
    std::vector<long> colorOptions = {
        static_cast<long>(ColorManager::getTeamColor(1).value_or(ColorManager::getDefaultTeamColor()).GetRaw()),
        static_cast<long>(ColorManager::getTeamColor(2).value_or(ColorManager::getDefaultTeamColor()).GetRaw()),
        static_cast<long>(ColorManager::getTeamColor(3).value_or(ColorManager::getDefaultTeamColor()).GetRaw()),
        static_cast<long>(ColorManager::getTeamColor(4).value_or(ColorManager::getDefaultTeamColor()).GetRaw()),
        static_cast<long>(ColorManager::getTeamColor(5).value_or(ColorManager::getDefaultTeamColor()).GetRaw()),
        static_cast<long>(ColorManager::getTeamColor(6).value_or(ColorManager::getDefaultTeamColor()).GetRaw()),
        static_cast<long>(ColorManager::getTeamColor(7).value_or(ColorManager::getDefaultTeamColor()).GetRaw()),
        static_cast<long>(ColorManager::getTeamColor(8).value_or(ColorManager::getDefaultTeamColor()).GetRaw())
    };
    for (int i = 0; i < colors.size(); i++) {
        auto current = colors[i]->selectedIndex();
        colors[i]->setItems(colorOptions);
        colors[i]->setSelectedIndex(current);
        colors[i]->setNeedsLayout();
    }
}
