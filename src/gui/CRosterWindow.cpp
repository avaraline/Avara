#include "CRosterWindow.h"

#include "AvaraDefines.h"
#include "CAbstractPlayer.h"
#include "CAvaraApp.h"
#include "CNetManager.h"
#include "CPlayerManager.h"
#include "CScoreKeeper.h"

#include <nanogui/colorcombobox.h>
#include <nanogui/layout.h>
#include <numeric>
#include <sstream>
#include <nanogui/tabwidget.h>
using namespace nanogui;

std::vector<long> player_colors =
    {kGreenTeamColor, kYellowTeamColor, kRedTeamColor, kPinkTeamColor, kPurpleTeamColor, kBlueTeamColor};

std::vector<Text *> statuses;
std::vector<Text *> chats;
std::vector<ColorComboBox *> colors;
OSType currentLevel;
TabWidget *tabWidget;


std::vector<Text *> scoreTeams;
std::vector<Text *> scoreNames;
std::vector<Text *> scoreExitRanks;
std::vector<Text *> scoreScores;
std::vector<Text *> scoreKills;
std::vector<Text *> scoreLives;

const int CHAT_CHARS = 57;
const int ROSTER_FONT_SIZE = 15;
const int SCORE_FONT_SIZE = 16;
bool textInputStarted = false;
char backspace[1] = {'\b'};
char clearline[1] = {'\x1B'};
char endline[1] = {13};
char bellline[1] = {7};
char checkline[1] = {6};

CRosterWindow::CRosterWindow(CApplication *app) : CWindow(app, "Roster") {
    setFixedWidth(470);

    tabWidget = add<TabWidget>();
    Widget *playersLayer = tabWidget->createTab("Players");

    setLayout(new BoxLayout(Orientation::Vertical, Alignment::Fill, 0, 0));

    BoxLayout *blayout = new BoxLayout(Orientation::Vertical, Alignment::Fill, 0, 0);
    AdvancedGridLayout *layout = new AdvancedGridLayout();
    playersLayer->setLayout(blayout);
    auto panel = playersLayer->add<Widget>();
    panel->setLayout(layout);
    theNet = ((CCAvaraApp *)gApplication)->GetNet();
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
        color->popup()->setSize(nanogui::Vector2i(50, 230));
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



    currentLevel = ((CCAvaraApp *)gApplication)->GetGame()->loadedTag;

    UpdateRoster();
}

CRosterWindow::~CRosterWindow() {}

void CRosterWindow::UpdateRoster() {
    CAvaraGame *theGame = ((CCAvaraApp *)gApplication)->GetGame();
    if (tabWidget->activeTab() == 0) {
        for (int i = 0; i < kMaxAvaraPlayers; i++) {
            CPlayerManager *thisPlayer = ((CCAvaraApp *)gApplication)->GetNet()->playerTable[i];

            const std::string theName((char *)thisPlayer->PlayerName() + 1, thisPlayer->PlayerName()[0]);
            short status = thisPlayer->LoadingStatus();
            std::string theStatus = GetStringStatus(status, thisPlayer->WinFrame());

            std::string theChat = thisPlayer->GetChatString(CHAT_CHARS);

            statuses[i]->setValue(theStatus.c_str());
            chats[i]->setValue(theChat.c_str());
            colors[i]->setSelectedIndex(theNet->teamColors[i]);
            colors[i]->setCaption(theName.c_str());
            colors[i]->popup()->setAnchorPos(nanogui::Vector2i(235, 68 + 60 * i));
        }

        if (theGame->loadedTag != currentLevel) {
            std::string theLevel((char* ) theGame->loadedLevel + 1, theGame->loadedLevel[0]);
            std::string theDesigner((char *)theGame->loadedDesigner + 1, theGame->loadedDesigner[0]);
            levelLoaded->setValue(theLevel);
            levelDesigner->setValue(theDesigner);

            std::string desc((char *)theGame->loadedInfo + 1, theGame->loadedInfo[0]);
            if (desc.length() > 0) levelDescription->setCaption(desc);
            currentLevel = theGame->loadedTag;
        }
    }
    else {

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

std::string CRosterWindow::GetStringStatus(short status, Fixed winFrame) {
    std::string strStatus;
    if (winFrame >= 0) {
        long timeTemp = FMulDiv(winFrame, ((CAvaraApp *)gApplication)->GetGame()->frameTime, 10);
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

    if (status == kLConnected) {
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
    ((CCAvaraApp *)gApplication)->GetNet()->SendRosterMessage(len, message);
}

bool CRosterWindow::handleSDLEvent(SDL_Event &event) {
    if (!textInputStarted)
        return false;
    if (event.type == SDL_TEXTINPUT) {
        // we already sent a checkmark, don't send a v
        if ((SDL_GetModState() & KMOD_ALT) && strcmp(event.text.text, "v") == 0)
            return true;
        SendRosterMessage(strlen(event.text.text), event.text.text);
        return true;
    } else if (event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.sym) {
            case SDLK_BACKSPACE:
                SendRosterMessage(1, backspace);
                return true;
            case SDLK_RETURN:
                SendRosterMessage(1, endline);
                return true;
            case SDLK_CLEAR:
            case SDLK_DELETE:
                SendRosterMessage(1, clearline);
                return true;
            case SDLK_g:
                if (SDL_GetModState() & KMOD_CTRL) {
                    SendRosterMessage(1, bellline);
                    return true;
                } else
                    return false;
            case SDLK_v:
                if (SDL_GetModState() & KMOD_ALT) {
                    SendRosterMessage(1, checkline);
                    return true;
                } else
                    return false;
            default:
                return false;
        }
    } else {
        return false;
    }
}
