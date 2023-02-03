#include "CHUD.h"
#include "CAbstractPlayer.h"
#include "CNetManager.h"
#include "CAvaraGame.h"
#include "ColorManager.h"
#include "CPlayerManager.h"
#include "AvaraDefines.h"
#include "CScoreKeeper.h"
#include "RGBAColor.h"

#include <stdint.h>

CHUD::CHUD(CAvaraGame *game) {
    itsGame = game;
}

const int CHAT_CHARS = 40;
const NVGcolor BACKGROUND_COLOR = nvgRGBA(30, 30, 30, 180);

bool sortByScore(std::pair<PlayerScoreRecord, int> i, std::pair<PlayerScoreRecord, int> j) {
    if(i.first.points == j.first.points) {
        return i.second < j.second;
    }
    else {
        return i.first.points > j.first.points;
    }
}

void CHUD::DrawScore(int playingCount, int chudHeight, CViewParameters *view, NVGcontext *ctx) {
    CAbstractPlayer *player = itsGame->GetLocalPlayer();
    CPlayerManager *playerManager = itsGame->FindPlayerManager(player);

    if(playingCount > 0 && (playerManager->GetShowScoreboard() || itsGame->gameStatus != kPlayingStatus)) {
        AvaraScoreRecord theScores = itsGame->scoreKeeper->localScores;
        CNetManager *net = itsGame->itsApp->GetNet();
        float colorBoxWidth = 30.0;
        //int bufferWidth = view->viewPixelDimensions.h;
        int bufferHeight = view->viewPixelDimensions.v;
        float boardWidth = 620;
        float boardHeight = 60 + (colorBoxWidth + 10) * playingCount;
        float x = 20;
        float y = bufferHeight-chudHeight-boardHeight - 20;
        float fontsz_m = 28.0, fontsz_s = 18.0;
        uint32_t longTeamColor;
        float teamColorRGB[3];
        NVGcolor aliveColor = nvgRGBA(255, 255, 255, 255);
        NVGcolor deadColor = nvgRGBA(165, 165, 165, 255);
        NVGcolor highColor = nvgRGBA(255, 0, 0, 255);
        float colWidth = 70;
        float rankWidth = 40;

        //use netscores when not in the game
        if(itsGame->gameStatus != kPlayingStatus && itsGame->gameStatus != kPauseStatus) {
            theScores = itsGame->scoreKeeper->netScores;
        }

        //sort by highscore
        int16_t highKills = 0;
        int16_t highWins = 0;
        std::vector<std::pair<PlayerScoreRecord, int> > sortedPlayers;
        for (int i = 0; i < kMaxAvaraPlayers; ++i) {
            sortedPlayers.push_back(std::make_pair(theScores.player[i], i));
            if(net->playerTable[i]->GetPlayer()) {
                highKills = std::max(highKills, theScores.player[i].kills);
                highWins = std::max(highWins, theScores.player[i].serverWins);
            }
        }
        std::sort(sortedPlayers.begin(), sortedPlayers.end(), sortByScore);

        //draw score box
        nvgBeginPath(ctx);
        nvgFillColor(ctx, nvgRGBA(20, 20, 20, 160));
        nvgRoundedRect(ctx, x, y, boardWidth, boardHeight, 4.0);
        nvgFill(ctx);

        //header box
        nvgBeginPath(ctx);
        nvgFillColor(ctx, nvgRGBA(0, 0, 0, 160));
        nvgRoundedRect(ctx, x, y, boardWidth, 40, 4.0);
        nvgFill(ctx);

        //drop shadow
        NVGpaint shadowPaint = nvgBoxGradient(ctx, x,y+2, boardWidth,boardHeight, 8, 10, nvgRGBA(0,0,0,128), nvgRGBA(0,0,0,0));
        nvgBeginPath(ctx);
        nvgRect(ctx, x-10,y-10, boardWidth+20,boardHeight+30);
        nvgRoundedRect(ctx, x,y, boardWidth,boardHeight, 4.0);
        nvgPathWinding(ctx, NVG_HOLE);
        nvgFillPaint(ctx, shadowPaint);
        nvgFill(ctx);

        x+=10;

        //score column text settings
        nvgFillColor(ctx, aliveColor);
        nvgTextAlign(ctx, NVG_ALIGN_RIGHT | NVG_ALIGN_TOP);
        nvgFontSize(ctx, fontsz_s);

        //column titles
        y+= 10;
        nvgText(ctx, x + colorBoxWidth + colWidth + 10, y, "Player", NULL);
        nvgText(ctx, x + colorBoxWidth + colWidth*4, y, "Score", NULL);
        nvgText(ctx, x + colorBoxWidth + colWidth*5, y, "Kills", NULL);
        nvgText(ctx, x + colorBoxWidth + colWidth*6, y, "Lives", NULL);
        nvgText(ctx, x + colorBoxWidth + colWidth*7, y, "Wins", NULL);
        nvgText(ctx, x + colorBoxWidth + colWidth*8, y, "RT(ms)", NULL);
        y+= 45;

        int playerRank = 0;
        nvgFontSize(ctx, fontsz_m);
        int16_t previousScore = -32768;
        for (int i = 0; i < kMaxAvaraPlayers; i++) {
            int playerTableIndex = sortedPlayers[i].second;
            CPlayerManager *thisPlayer = net->playerTable[playerTableIndex];
            const std::string playerName((char *)thisPlayer->PlayerName() + 1, thisPlayer->PlayerName()[0]);
            std::string ping = "--";
            longTeamColor = *ColorManager::getTeamColor(net->teamColors[playerTableIndex] + 1);
            LongToRGBA(longTeamColor, teamColorRGB, 3);
            NVGcolor textColor = aliveColor;

            if(playerName.size() > 0 && thisPlayer->GetPlayer() != NULL) {
                if(theScores.player[playerTableIndex].points != previousScore)
                    playerRank++;

                //int playerLives = thisPlayer->GetPlayer()->lives;
                int playerLives = theScores.player[playerTableIndex].lives;
                if(thisPlayer->IsLocalPlayer()) {
                    //self highlight
                    nvgBeginPath(ctx);
                    nvgRoundedRect(ctx, x - 5, y-3, boardWidth - 10, colorBoxWidth + 6, 4.0);
                    nvgFillColor(ctx, nvgRGBA(128, 255, 0, 210));
                    nvgFill(ctx);
                }
                else {
                    ping = std::to_string(net->itsCommManager->GetMaxRoundTrip(1 << playerTableIndex));
                }

                //player color box
                nvgBeginPath(ctx);
                nvgRoundedRect(ctx, x + rankWidth + 10, y, colorBoxWidth, colorBoxWidth, 3.0);
                nvgFillColor(ctx, nvgRGBAf(teamColorRGB[0], teamColorRGB[1], teamColorRGB[2], 1.0));
                nvgFill(ctx);

                //show winnner
                if(thisPlayer->WinFrame() > 0) {
                    std::string checkMark("\u221A");
                    nvgFillColor(ctx, aliveColor);
                    nvgText(ctx, x + rankWidth + 33, y, checkMark.c_str(), NULL);
                }

                //score text settings
                textColor = aliveColor;
                if(playerLives == 0 && (itsGame->gameStatus == kPlayingStatus || itsGame->gameStatus == kPauseStatus))
                    textColor = deadColor;

                nvgFillColor(ctx, textColor);
                nvgTextAlign(ctx, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
                nvgFontSize(ctx, fontsz_m);

                //rank
                std::string rank = std::to_string(playerRank);
                switch (playerRank) {
                    case 1: rank += "ST"; break;
                    case 2: rank += "ND"; break;
                    case 3: rank += "RD"; break;
                    default: rank += "TH"; break;
                }
                nvgText(ctx, x, y, rank.c_str(), NULL);

                //player name and scores
                nvgText(ctx, x + colorBoxWidth + rankWidth + 15, y, playerName.c_str(), NULL);

                nvgTextAlign(ctx, NVG_ALIGN_RIGHT | NVG_ALIGN_TOP);
                nvgText(ctx, x + colorBoxWidth + colWidth*4, y, std::to_string(theScores.player[playerTableIndex].points).c_str(), NULL);

                //kills
                if(highKills > 0 && theScores.player[playerTableIndex].kills == highKills)
                    nvgFillColor(ctx, highColor);
                nvgText(ctx, x + colorBoxWidth + colWidth*5, y, std::to_string(theScores.player[playerTableIndex].kills).c_str(), NULL);
                nvgFillColor(ctx, textColor);

                //lives
                nvgText(ctx, x + colorBoxWidth + colWidth*6, y, std::to_string(playerLives).c_str(), NULL);

                //wins
                if(highWins > 0 && theScores.player[playerTableIndex].serverWins == highWins)
                    nvgFillColor(ctx, highColor);
                nvgText(ctx, x + colorBoxWidth + colWidth*7, y, std::to_string(theScores.player[playerTableIndex].serverWins).c_str(), NULL);
                nvgFillColor(ctx, textColor);

                //ping
                nvgText(ctx, x + colorBoxWidth + colWidth*8, y, ping.c_str(), NULL);

                y += colorBoxWidth + 10;

                previousScore = theScores.player[playerTableIndex].points;
             }
        }
    }
}

void CHUD::DrawLevelName(CViewParameters *view, NVGcontext *ctx) {
    std::string level = itsGame->loadedLevel;
    if(itsGame->gameStatus != kPlayingStatus && level.length() > 0) {
        int bufferWidth = view->viewPixelDimensions.h;
        int bufferHeight = view->viewPixelDimensions.v;

        float x = 0.0;
        float y = bufferHeight - 130.0;

        float padding = 5.0;

        float bounds[4];
        
        nvgBeginPath(ctx);
        nvgFontFace(ctx, "mono");
        nvgFontSize(ctx, 45.0);
        nvgTextBounds(ctx, x, y, level.c_str(), NULL, bounds);

        float textw = (bounds[2] - bounds[0]);
        float texth = (bounds[3] - bounds[1]);

        x = (bufferWidth / 2) - (textw / 2);
        nvgFillColor(ctx, BACKGROUND_COLOR);
        nvgRect(ctx, x - padding, y - padding, textw + (padding * 2), texth);
        nvgFill(ctx);
        nvgFillColor(ctx, nvgRGBA(255,255,255,220));
        nvgTextAlign(ctx, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
        nvgText(ctx, x, y - padding, level.c_str(), NULL);
    }
}

void CHUD::DrawPaused(CViewParameters *view, NVGcontext *ctx) {
    if(itsGame->gameStatus == kPauseStatus) {
        int bufferWidth = view->viewPixelDimensions.h;
        int bufferHeight = view->viewPixelDimensions.v;
        int centerX = bufferWidth / 2;
        int centerY = bufferHeight / 2;
        float barWidth = 25.0;
        float barHeight = 100.0;

        //draw circle
        nvgBeginPath(ctx);
        nvgCircle(ctx, centerX, centerY, 100.0);
        nvgFillColor(ctx, nvgRGBA(0,0,0,180));
        nvgFill(ctx);

        //vertical pause bars
        nvgBeginPath(ctx);
        nvgRoundedRect(ctx, centerX + 20, centerY - (barHeight/2), barWidth, barHeight, 10);
        nvgRoundedRect(ctx, centerX - 20 - barWidth, centerY - (barHeight/2), barWidth, barHeight, 10);
        nvgFillColor(ctx, nvgRGBA(255,255,255,180));
        nvgFill(ctx);
    }
}

void CHUD::Render(CViewParameters *view, NVGcontext *ctx) {
    int bufferWidth = view->viewPixelDimensions.h, bufferHeight = view->viewPixelDimensions.v;
    nvgBeginFrame(ctx, bufferWidth, bufferHeight, view->viewPixelRatio);
    CAbstractPlayer *player = itsGame->GetLocalPlayer();
    CAbstractPlayer *spectatePlayer = itsGame->GetSpectatePlayer();
    CNetManager *net = itsGame->itsApp->GetNet();

    DrawLevelName(view, ctx);
    if (itsGame->gameStatus != kPlayingStatus) return;
    DrawPaused(view, ctx);

    int playerCount = 0;
    int playingCount = 0;
    for (int i = 0; i < kMaxAvaraPlayers; i++) {
        CPlayerManager *thisPlayer = net->playerTable[i];
        std::string playerName((char *)thisPlayer->PlayerName() + 1, thisPlayer->PlayerName()[0]);
        if (playerName.length() > 0) {
            playerCount++;

            if(thisPlayer->GetPlayer() != NULL) {
                playingCount++;
            }
        }
    }

    int playerSlots = std::max(6, playerCount);
    int chudHeight = 13 * playerSlots;
    DrawScore(playingCount, chudHeight, view, ctx);


    nvgBeginPath(ctx);
    nvgRect(ctx, 0, bufferHeight - chudHeight, bufferWidth, chudHeight);
    nvgFillColor(ctx, BACKGROUND_COLOR);
    nvgFill(ctx);

    float fontsz_m = 15.0, fontsz_s = 10.0;
    nvgFontFace(ctx, "mono");

    float mX = 20.0;
    float mY = (bufferHeight - chudHeight + 8);
    short textAlign = NVG_ALIGN_LEFT;
    for (auto i : itsGame->itsApp->MessageLines()) {
        switch (i.align) {
            case MsgAlignment::Left:
                mX = 20.0;
                textAlign = NVG_ALIGN_LEFT;
                break;
            case MsgAlignment::Center:
                mX = ((bufferWidth / 2.0) - 100.0) / 2.0;
                textAlign = NVG_ALIGN_CENTER;
                break;
            case MsgAlignment::Right:
                mX = ((bufferWidth / 2.0) - 100.0) - 20.0;
                textAlign = NVG_ALIGN_RIGHT;
                break;
        }
        nvgBeginPath(ctx);
        nvgTextAlign(ctx, textAlign | NVG_ALIGN_TOP);
        nvgFontSize(ctx, fontsz_m);
        nvgFillColor(ctx, LongToNVG(
            ColorManager::getMessageColor(i.category)
        ));
        nvgText(ctx, mX, mY, i.text.c_str(), NULL);
        mY += 11;
    }

    float pY;
    uint32_t longTeamColor;
    float teamColorRGB[3];
    float colorBoxAlpha = 1.0;
    for (int i = 0; i < kMaxAvaraPlayers; i++) {
        CPlayerManager *thisPlayer = net->playerTable[i];
        std::string playerName((char *)thisPlayer->PlayerName() + 1, thisPlayer->PlayerName()[0]);
        if (playerName.length() < 1) continue;
        pY = (bufferHeight - chudHeight + 8) + (11 * i);
        longTeamColor = *ColorManager::getTeamColor(net->teamColors[i] + 1);
        LongToRGBA(longTeamColor, teamColorRGB, 3);
        std::string playerChat = thisPlayer->GetChatString(CHAT_CHARS);
        NVGcolor textColor = nvgRGBA(255, 255, 255, 255);

        //check for not playing
        if(thisPlayer->IsAway()) {
            textColor = nvgRGBA(255, 255, 255, 150);
            colorBoxAlpha = 0.5;
        }
        else {
            textColor = nvgRGBA(255, 255, 255, 255);
            colorBoxAlpha = 1.0;
        }

        //player color box
        float colorBoxWidth = 10.0;
        nvgBeginPath(ctx);

        //highlight player if spectating
        if (spectatePlayer != NULL && thisPlayer->GetPlayer() == spectatePlayer) {
            textColor = LongToNVG(*ColorManager::getTeamTextColor(net->teamColors[i] + 1));
            colorBoxWidth = 150.0;
        }

        nvgRect(ctx, bufferWidth - 160, pY, colorBoxWidth, 10.0);
        nvgFillColor(ctx, nvgRGBAf(teamColorRGB[0], teamColorRGB[1], teamColorRGB[2], colorBoxAlpha));
        nvgFill(ctx);

        //player name
        nvgFillColor(ctx, textColor);
        nvgTextAlign(ctx, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
        nvgFontSize(ctx, fontsz_m);
        nvgText(ctx, bufferWidth - 148, pY - 3, playerName.c_str(), NULL);

        short status = thisPlayer->GetStatusChar();
        if (status >= 0) {
            std::string playerLives = std::to_string(status);
            if (status == 10) playerLives = "%";
            if (status == 12) playerLives = "C";
            nvgTextAlign(ctx, NVG_ALIGN_RIGHT | NVG_ALIGN_TOP);
            nvgFontSize(ctx, fontsz_s);
            nvgFillColor(ctx, nvgRGBA(255, 255, 255, 255));
            nvgText(ctx, bufferWidth - 162, pY - 3, playerLives.c_str(), NULL);
            if (thisPlayer->GetMessageIndicator() > 0) {
                nvgText(ctx, bufferWidth - 162, pY + 3, "<", NULL);
            }
        }

        nvgTextAlign(ctx, NVG_ALIGN_RIGHT | NVG_ALIGN_TOP);
        nvgFontSize(ctx, fontsz_m);
        nvgFillColor(ctx, nvgRGBA(255, 255, 255, 255));
        nvgText(ctx, bufferWidth - 168, pY - 3, playerChat.c_str(), NULL);

        //spectating onscreen name
        if(spectatePlayer != NULL && thisPlayer->GetPlayer() == spectatePlayer) {
            int x = 20;
            int y = 20;
            float fontsz_m = 24.0;
            float bounds[4];
            std::string specMessage("Spectating " + playerName);

            nvgBeginPath(ctx);
            nvgFontFace(ctx, "mono");
            nvgTextAlign(ctx, NVG_ALIGN_MIDDLE | NVG_ALIGN_BOTTOM);
            nvgFontSize(ctx, fontsz_m);
            nvgTextBounds(ctx, x,y, specMessage.c_str(), NULL, bounds);

            //draw box for text
            nvgBeginPath(ctx);
            nvgRoundedRect(ctx, x, y, (bounds[2]-bounds[0])+10, 28.0, 3.0);
            nvgFillColor(ctx, BACKGROUND_COLOR);
            nvgFill(ctx);

            //draw text
            nvgFillColor(ctx, nvgRGBA(255, 255, 255, 255));
            nvgText(ctx, x + 5, y + 14, specMessage.c_str(), NULL);
        }
    }

    if (!player)
        return;

    if(spectatePlayer != NULL)
        player = spectatePlayer;

    int i, j;
    float g1X = (bufferWidth / 2.0) - 60.0;
    float gY = bufferHeight - 60.0;
    float g2X1 = (bufferWidth / 2.0) + 15.0;
    float g2Y1 = bufferHeight - 60.0;
    float g2X2 = (bufferWidth / 2.0) + 15.0 + 22.5;
    float g2Y2 = bufferHeight - 60.0 + 7.5;
    float g2X3 = (bufferWidth / 2.0) + 15.0 + 15.0;
    float g2Y3 = bufferHeight - 60.0 + 30.0;
    float g2X4 = (bufferWidth / 2.0) + 15.0 - 7.5;
    float g2Y4 = bufferHeight - 60.0 + 22.5;
    char scoreText[20];
    char timeText[9];
    float full = 30.0;
    float half = 15.0;
    float w_pow = 15.0;
    int secs, mins;
    long timeTemp = itsGame->timeInSeconds;
    float energy = sqrt((float)player->energy / (float)player->maxEnergy) * full;
    float pow_l = ((float)player->gunEnergy[1] / (float)player->fullGunEnergy) * half;
    float pow_r = ((float)player->gunEnergy[0] / (float)player->fullGunEnergy) * half;
    float shields = sqrt((float)player->shields / (float)player->maxShields) * full;
    float g2s = 3.75;
    float g1[4][4][2] = {{{g1X, gY}, {g1X, gY + energy}, {g1X + energy, gY + energy}, {g1X, gY}},
        {{g1X + half, gY + half},
            {g1X + half - pow_l, gY + half - pow_l},
            {g1X + half - pow_l + w_pow, gY + half - pow_l},
            {g1X + half + w_pow, gY + half}},
        {{g1X + half, gY + half},
            {g1X + half + pow_r, gY + half + pow_r},
            {g1X + half + pow_r + w_pow, gY + half + pow_r},
            {g1X + half + w_pow, gY + half}},
        {{g1X + w_pow + full - shields, gY + full - shields},
            {g1X + full + w_pow, gY + full},
            {g1X + full + w_pow, gY + full},
            {g1X + w_pow + full, gY + full - shields}}};
    float g2[4][4][4][2] = {{// boosters
                                {{g2X1 + (g2s * 0), g2Y1 + (g2s * 0)},
                                    {g2X1 + (g2s * 1), g2Y1 + (g2s * 1)},
                                    {g2X1 + (g2s * 7), g2Y1 + (g2s * 1)},
                                    {g2X1 + (g2s * 8), g2Y1 + (g2s * 0)}},
                                {{g2X1 + (g2s * 1), g2Y1 + (g2s * 1)},
                                    {g2X1 + (g2s * 2), g2Y1 + (g2s * 2)},
                                    {g2X1 + (g2s * 6), g2Y1 + (g2s * 2)},
                                    {g2X1 + (g2s * 7), g2Y1 + (g2s * 1)}},
                                {{g2X1 + (g2s * 2), g2Y1 + (g2s * 2)},
                                    {g2X1 + (g2s * 3), g2Y1 + (g2s * 3)},
                                    {g2X1 + (g2s * 5), g2Y1 + (g2s * 3)},
                                    {g2X1 + (g2s * 6), g2Y1 + (g2s * 2)}},
                                {{g2X1 + (g2s * 3), g2Y1 + (g2s * 3)},
                                    {g2X1 + (g2s * 4), g2Y1 + (g2s * 4)},
                                    {g2X1 + (g2s * 4), g2Y1 + (g2s * 4)},
                                    {g2X1 + (g2s * 5), g2Y1 + (g2s * 3)}}},
        {// grenades
            {{g2X2 - (g2s * 0), g2Y2 + (g2s * 0)},
                {g2X2 - (g2s * 1), g2Y2 + (g2s * 1)},
                {g2X2 - (g2s * 1), g2Y2 + (g2s * 7)},
                {g2X2 - (g2s * 0), g2Y2 + (g2s * 8)}},
            {{g2X2 - (g2s * 1), g2Y2 + (g2s * 1)},
                {g2X2 - (g2s * 2), g2Y2 + (g2s * 2)},
                {g2X2 - (g2s * 2), g2Y2 + (g2s * 6)},
                {g2X2 - (g2s * 1), g2Y2 + (g2s * 7)}},
            {{g2X2 - (g2s * 2), g2Y2 + (g2s * 2)},
                {g2X2 - (g2s * 3), g2Y2 + (g2s * 3)},
                {g2X2 - (g2s * 3), g2Y2 + (g2s * 5)},
                {g2X2 - (g2s * 2), g2Y2 + (g2s * 6)}},
            {{g2X2 - (g2s * 3), g2Y2 + (g2s * 3)},
                {g2X2 - (g2s * 4), g2Y2 + (g2s * 4)},
                {g2X2 - (g2s * 4), g2Y2 + (g2s * 4)},
                {g2X2 - (g2s * 3), g2Y2 + (g2s * 5)}}},
        {// lives
            {{g2X3 - (g2s * 0), g2Y3 - (g2s * 0)},
                {g2X3 - (g2s * 1), g2Y3 - (g2s * 1)},
                {g2X3 - (g2s * 7), g2Y3 - (g2s * 1)},
                {g2X3 - (g2s * 8), g2Y3 - (g2s * 0)}},
            {{g2X3 - (g2s * 1), g2Y3 - (g2s * 1)},
                {g2X3 - (g2s * 2), g2Y3 - (g2s * 2)},
                {g2X3 - (g2s * 6), g2Y3 - (g2s * 2)},
                {g2X3 - (g2s * 7), g2Y3 - (g2s * 1)}},
            {{g2X3 - (g2s * 2), g2Y3 - (g2s * 2)},
                {g2X3 - (g2s * 3), g2Y3 - (g2s * 3)},
                {g2X3 - (g2s * 5), g2Y3 - (g2s * 3)},
                {g2X3 - (g2s * 6), g2Y3 - (g2s * 2)}},
            {{g2X3 - (g2s * 3), g2Y3 - (g2s * 3)},
                {g2X3 - (g2s * 4), g2Y3 - (g2s * 4)},
                {g2X3 - (g2s * 4), g2Y3 - (g2s * 4)},
                {g2X3 - (g2s * 5), g2Y3 - (g2s * 3)}}},
        {// missiles
            {{g2X4 + (g2s * 0), g2Y4 - (g2s * 0)},
                {g2X4 + (g2s * 1), g2Y4 - (g2s * 1)},
                {g2X4 + (g2s * 1), g2Y4 - (g2s * 7)},
                {g2X4 + (g2s * 0), g2Y4 - (g2s * 8)}},
            {{g2X4 + (g2s * 1), g2Y4 - (g2s * 1)},
                {g2X4 + (g2s * 2), g2Y4 - (g2s * 2)},
                {g2X4 + (g2s * 2), g2Y4 - (g2s * 6)},
                {g2X4 + (g2s * 1), g2Y4 - (g2s * 7)}},
            {{g2X4 + (g2s * 2), g2Y4 - (g2s * 2)},
                {g2X4 + (g2s * 3), g2Y4 - (g2s * 3)},
                {g2X4 + (g2s * 3), g2Y4 - (g2s * 5)},
                {g2X4 + (g2s * 2), g2Y4 - (g2s * 6)}},
            {{g2X4 + (g2s * 3), g2Y4 - (g2s * 3)},
                {g2X4 + (g2s * 4), g2Y4 - (g2s * 4)},
                {g2X4 + (g2s * 4), g2Y4 - (g2s * 4)},
                {g2X4 + (g2s * 3), g2Y4 - (g2s * 5)}}}};
    NVGcolor g1c[] = {
        LongToNVG(ColorManager::getEnergyGaugeColor()),
        LongToNVG(ColorManager::getPlasmaGauge1Color()),
        LongToNVG(ColorManager::getPlasmaGauge2Color()),
        LongToNVG(ColorManager::getShieldGaugeColor())
    };
    NVGcolor g2c[] = {
        LongToNVG(ColorManager::getPinwheel1Color()),
        LongToNVG(ColorManager::getPinwheel2Color()),
        LongToNVG(ColorManager::getPinwheel3Color()),
        LongToNVG(ColorManager::getPinwheel4Color())
    };
    for (i = 0; i < 4; i++) { // referred to as GrafPanel in original Avara
        nvgBeginPath(ctx);
        nvgMoveTo(ctx, g1[i][0][0], g1[i][0][1]);
        nvgLineTo(ctx, g1[i][1][0], g1[i][1][1]);
        nvgLineTo(ctx, g1[i][2][0], g1[i][2][1]);
        nvgLineTo(ctx, g1[i][3][0], g1[i][3][1]);
        nvgClosePath(ctx);
        nvgFillColor(ctx, g1c[i]);
        nvgFill(ctx);
    }
    for (i = 0; i < 4; i++) { // "pinwheel" indicator
        for (j = 0; j < 4; j++) {
            if (i == 0 && j >= player->boostsRemaining)
                continue;
            if (i == 1 && j >= (player->grenadeCount + 1) / 2)
                continue;
            if (i == 2 && j >= player->lives)
                continue;
            if (i == 3 && j >= player->missileCount)
                continue;
            nvgBeginPath(ctx);
            nvgMoveTo(ctx, g2[i][j][0][0], g2[i][j][0][1]);
            nvgLineTo(ctx, g2[i][j][1][0], g2[i][j][1][1]);
            nvgLineTo(ctx, g2[i][j][2][0], g2[i][j][2][1]);
            nvgLineTo(ctx, g2[i][j][3][0], g2[i][j][3][1]);
            nvgClosePath(ctx);
            nvgFillColor(ctx, g2c[j]);
            nvgFill(ctx);
        }
    }
    if (itsGame->veryLongWait) {
        nvgBeginPath(ctx);
        nvgRect(ctx, g1X, gY, 8.0, 8.0);
        nvgFillColor(ctx, LongToNVG(ColorManager::getNetDelay2Color()));
        nvgFill(ctx);
    }
    if (itsGame->longWait) {
        nvgBeginPath(ctx);
        nvgRect(ctx, g1X + 4, gY + 4, 8.0, 8.0);
        nvgFillColor(ctx, LongToNVG(ColorManager::getNetDelay1Color()));
        nvgFill(ctx);
    }
    nvgFillColor(ctx, nvgRGBA(255, 255, 255, 255));
    nvgFontSize(ctx, fontsz_m);
    nvgTextAlign(ctx, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
    if(player->itsManager != NULL) {
        snprintf(scoreText, sizeof(scoreText), "%ld", itsGame->scores[player->itsManager->Slot()]);
        nvgText(ctx, g1X + 22.5, gY + full + 10.0, scoreText, NULL);
    }

    secs = timeTemp % 60;
    timeTemp /= 60;
    mins = timeTemp % 60;
    timeTemp /= 60;
    snprintf(timeText, sizeof(timeText), "%ld:%02d:%02d", timeTemp, mins, secs);
    nvgFillColor(ctx, nvgRGBA(255, 255, 255, 255));
    nvgTextAlign(ctx, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
    nvgText(ctx, g2X1 + 7.5, gY + full + 10.0, timeText, NULL);

    nvgEndFrame(ctx);
}
