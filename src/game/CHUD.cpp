#include "CHUD.h"
#include "AssetManager.h"
#include "AbstractRenderer.h"
#include "CAbstractPlayer.h"
#include "CPlayerActor.h"
#include "CAvaraGame.h"
#include "ColorManager.h"
#include "CPlayerManager.h"
#include "AvaraDefines.h"
#include "CScoreKeeper.h"
#include "ARGBColor.h"
#include "CSmartPart.h"

#include <stdint.h>

CHUD::CHUD(CAvaraGame *game) {
    itsGame = game;
}

const int CHAT_CHARS = 40;
const NVGcolor BACKGROUND_COLOR = nvgRGBA(30, 30, 30, 180);

const int EHUD_SIGHT_MAX_DISTANCE = 1000;

bool sortByScore(std::pair<PlayerScoreRecord, int> i, std::pair<PlayerScoreRecord, int> j) {
    if(i.first.points == j.first.points) {
        return i.second < j.second;
    }
    else {
        return i.first.points > j.first.points;
    }
}

void CHUD::LoadImages(NVGcontext *ctx) {
    std::string imgPath = AssetManager::GetImagePath(NoPackage, "atlas.png");
    images = nvgCreateImage(ctx, imgPath.c_str(), 0);
}

void CHUD::DrawScore(std::vector<CPlayerManager*>& thePlayers, int chudHeight, NVGcontext *ctx) {
    auto view = gRenderer->viewParams;
    CAbstractPlayer *player = itsGame->GetLocalPlayer();
    CPlayerManager *playerManager = itsGame->FindPlayerManager(player);

    if(thePlayers.size() > 0 && (playerManager->GetShowScoreboard() || itsGame->gameStatus != kPlayingStatus)) {
        AvaraScoreRecord theScores = itsGame->scoreKeeper->localScores;
        CNetManager *net = itsGame->itsApp->GetNet();
        float colorBoxWidth = 30.0;
        int bufferWidth = view->viewPixelDimensions.h;
        //int bufferHeight = view->viewPixelDimensions.v;
        float boardWidth = 620;
        float boardHeight = 60 + (colorBoxWidth + 10) * thePlayers.size();
        float x = bufferWidth - boardWidth - 20;
        float y = 20; //bufferHeight-chudHeight-boardHeight - 20;
        float fontsz_m = 28.0, fontsz_s = 18.0;
        ARGBColor longTeamColor = 0;
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
            CPlayerManager *thisPlayer = net->playerTable[playerTableIndex].get();
            const std::string playerName((char *)thisPlayer->PlayerName() + 1, thisPlayer->PlayerName()[0]);
            std::string ping = "--";
            longTeamColor = *ColorManager::getTeamColor(net->teamColors[playerTableIndex] + 1);
            longTeamColor.ExportGLFloats(teamColorRGB, 3);
            NVGcolor textColor = aliveColor;

            if (std::any_of(thePlayers.begin(), thePlayers.end(), [&](CPlayerManager * pm) { return pm == thisPlayer; })) {
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

void CHUD::DrawEditingHud(CAbstractPlayer *player, NVGcontext *ctx) {
    if (!player) {
        return;
    }
    CPlayerManager *playerManager = itsGame->FindPlayerManager(player);
    if (!playerManager) {
        return;
    }
    if (playerManager->GetShowEditingHud()) {
        float boardWidth = 400;
        float boardHeight = 119;
        float x = 20;
        float y = 20;
        float fontsz = 18.0;
        float lineHeight = 20;

        nvgBeginPath(ctx);
        nvgFillColor(ctx, nvgRGBA(20, 20, 20, 160));
        nvgRoundedRect(ctx, x, y, boardWidth, boardHeight, 4.0);
        nvgFill(ctx);

        nvgFillColor(ctx, nvgRGBA(255, 255, 255, 255));
        nvgTextAlign(ctx, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
        nvgFontSize(ctx, fontsz);

        x += 14;
        y += 11;

        char ehudText[64];

        CPlayerActor *actor = static_cast<CPlayerActor*>(player);

        float playerY = ToFloat(actor->location[1]) + 0.1; // not sure why + 0.1 is needed

        snprintf(ehudText, sizeof(ehudText), "Player x/z/y: %.1f, %.1f, %.1f",
            ToFloat(actor->location[0]),
            ToFloat(actor->location[2]),
            playerY > 0.0 ? playerY : 0.0
        );
        nvgText(ctx, x, y, ehudText, NULL);

        y += lineHeight;

        // Heading is 32768 at an ALF angle of 0, and 0 at an ALF angle of 180.
        int heading = ceil(((player->heading - 32768) % 65536) / 65536.0 * 360);
        heading = heading <= 0 ? -heading : 360 - heading;
        snprintf(ehudText, sizeof(ehudText), "Heading (ALF angle): %d", heading);
        nvgText(ctx, x, y, ehudText, NULL);

        y += lineHeight;

        RayHitRecord theHit;
        Matrix *mt = &player->viewPortPart->itsTransform;
        Matrix m1, m2;
        OneMatrix(&m1);
        CombineTransforms(&m1, &m2, mt);
        theHit.direction[0] = (*mt)[2][0];
        theHit.direction[1] = (*mt)[2][1];
        theHit.direction[2] = (*mt)[2][2];

        snprintf(ehudText, sizeof(ehudText), "Sight sign vector x/z: %2d, %2d",
            theHit.direction[0] < 0 ? -1 : theHit.direction[0] == 0 ? 0 : 1,
            theHit.direction[2] < 0 ? -1 : theHit.direction[2] == 0 ? 0 : 1
        );
        nvgText(ctx, x, y, ehudText, NULL);

        y += lineHeight;

        theHit.origin[0] = m2[3][0];
        theHit.origin[1] = m2[3][1];
        theHit.origin[2] = m2[3][2];
        theHit.distance = MAXFIXED;
        theHit.closestHit = NULL;
        player->RayTestWithGround(&theHit, kSolidBit);

        float distance = ToFloat(theHit.distance);
        
        if (distance > EHUD_SIGHT_MAX_DISTANCE) {
            snprintf(ehudText, sizeof(ehudText), "Sight x/z/y: --");
        } else {
            NormalizeVector(3, theHit.direction);
            snprintf(ehudText, sizeof(ehudText), "Sight x/z/y: %.1f, %.1f, %.1f",
                ToFloat(theHit.origin[0] + theHit.direction[0] * distance),
                ToFloat(theHit.origin[2] + theHit.direction[2] * distance),
                ToFloat(theHit.origin[1] + theHit.direction[1] * distance)
            );
        }
        nvgText(ctx, x, y, ehudText, NULL);

        y += lineHeight;

        if (distance > EHUD_SIGHT_MAX_DISTANCE) {
            snprintf(ehudText, sizeof(ehudText), "Distance: --");
        } else {
            snprintf(ehudText, sizeof(ehudText), "Distance: %.1f", ToFloat(theHit.distance));
        }
        nvgText(ctx, x, y, ehudText, NULL);
    }
}

void CHUD::DrawLevelName(NVGcontext *ctx) {
    auto view = gRenderer->viewParams;
    std::string level = itsGame->loadedLevel;
    if(itsGame->gameStatus != kPlayingStatus && level.length() > 0) {
        int bufferWidth = view->viewPixelDimensions.h;
        int bufferHeight = view->viewPixelDimensions.v;
        float x = 0.0;
        float y = bufferHeight - 130.0;
        float bounds[4];

        nvgBeginPath(ctx);
        nvgFontFace(ctx, "mono");
        nvgFontSize(ctx, 24.0);
        nvgTextBounds(ctx, x,y, level.c_str(), NULL, bounds);
        x = (bufferWidth / 2) - (bounds[2]-bounds[0]) / 2;
        nvgFillColor(ctx, BACKGROUND_COLOR);
        nvgRect(ctx, x-5,y, (int)(bounds[2]-bounds[0])+10, (int)(bounds[3]-bounds[1])+4);
        nvgFill(ctx);

        nvgFillColor(ctx, nvgRGBA(255,255,255,220));
        nvgText(ctx, x,y+(bounds[3]-bounds[1])-3, level.c_str(), NULL);
    }
}

void CHUD::DrawPaused(NVGcontext *ctx) {
    auto view = gRenderer->viewParams;
    if (itsGame->gameStatus == kPauseStatus) {
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

void CHUD::DrawImage(NVGcontext* ctx, int image, float alpha,
		float sx, float sy, float sw, float sh, // sprite location on texture
		float x, float y, float w, float h) // position and size of the sprite rectangle on screen
{
	float ax, ay;
	int iw,ih;
	NVGpaint img;
	
	nvgImageSize(ctx, image, &iw, &ih);

	// Aspect ration of pixel in x an y dimensions. This allows us to scale
	// the sprite to fill the whole rectangle.
	ax = w / sw;
	ay = h / sh;

	img = nvgImagePattern(ctx, x - sx*ax, y - sy*ay, (float)iw*ax, (float)ih*ay,
				0, image, alpha);
	nvgBeginPath(ctx);
	nvgRect(ctx, x,y, w,h);
	nvgFillPaint(ctx, img);
	nvgFill(ctx);
}

void CHUD::Render(NVGcontext *ctx) {
    auto view = gRenderer->viewParams;
    CAbstractPlayer *player = itsGame->GetLocalPlayer();
    CAbstractPlayer *spectatePlayer = itsGame->GetSpectatePlayer();
    CNetManager *net = itsGame->itsApp->GetNet();

    DrawLevelName(ctx);
    DrawPaused(ctx);

    std::vector<CPlayerManager*> thePlayers = net->ActivePlayers(); // only players actively playing

    int lastPlayerSlot = 6;  // limits HUD height
    for (auto player: thePlayers) {
        lastPlayerSlot = std::max(lastPlayerSlot, int(player->Slot() + 1));
    }

    int bufferWidth = view->viewPixelDimensions.h, bufferHeight = view->viewPixelDimensions.v;
    int chudHeight = 13 * lastPlayerSlot;

    DrawScore(thePlayers, chudHeight, ctx);
    DrawEditingHud(player, ctx);

    nvgBeginFrame(ctx, bufferWidth, bufferHeight, view->viewPixelRatio);

    nvgBeginPath(ctx);
    nvgRect(ctx, 0, bufferHeight - chudHeight, bufferWidth, chudHeight);
    nvgFillColor(ctx, BACKGROUND_COLOR);
    nvgFill(ctx);

    float fontsz_l = 25.0, fontsz_m = 15.0, fontsz_s = 10.0;
    nvgFontFace(ctx, "mono");

    float mX = 20.0;
    float mY = (bufferHeight - chudHeight + 8);
    short textAlign = NVG_ALIGN_LEFT;
    int messageCount = 0, maxMessageCount = 5;
    for (auto iter = itsGame->itsApp->MessageLines().rbegin(); iter != itsGame->itsApp->MessageLines().rend(); iter++) {
        MsgLine msg = *iter;
        switch (msg.align) {
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
        nvgFillColor(ctx, ColorManager::getMessageColor(msg.category).IntoNVG());
        nvgText(ctx, mX, mY, msg.text.c_str(), NULL);
        if (messageCount >= maxMessageCount) {
            break;
        }
        mY += 11;
        messageCount++;
    }

    float pY;
    ARGBColor longTeamColor = 0;
    float teamColorRGB[3];
    float colorBoxAlpha = 1.0;
    for (int i = 0; i < kMaxAvaraPlayers; i++) {
        CPlayerManager *thisPlayer = net->playerTable[i].get();
        std::string playerName = thisPlayer->GetPlayerName();
        if (playerName.length() < 1) continue;

        // Get player RTT
        long rtt = 0;
        if (i != net->itsCommManager->myId && playerName.length() > 0) {
            rtt = net->itsCommManager->GetMaxRoundTrip(1 << i);
        }

        pY = (bufferHeight - chudHeight + 8) + (11 * i);
        longTeamColor = *ColorManager::getTeamColor(net->teamColors[i] + 1);
        longTeamColor.ExportGLFloats(teamColorRGB, 3);
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
        float colorBoxHeight = 10.0;
        float pingBarHeight = 0.0;
        nvgBeginPath(ctx);

        //highlight player if spectating
        if (spectatePlayer != NULL && thisPlayer->GetPlayer() == spectatePlayer) {
            textColor = (*ColorManager::getTeamTextColor(net->teamColors[i] + 1)).IntoNVG();
            colorBoxWidth = 150.0;
        }

        nvgRect(ctx, bufferWidth - 160, pY, colorBoxWidth, colorBoxHeight);
        nvgFillColor(ctx, nvgRGBAf(teamColorRGB[0], teamColorRGB[1], teamColorRGB[2], colorBoxAlpha));
        nvgFill(ctx);

        // draw something based on the loading status
        switch (thisPlayer->LoadingStatus()) {
          // Draw a dot
          case kLReady:
            nvgBeginPath(ctx);
            nvgCircle(ctx, bufferWidth - 155, pY + (colorBoxHeight / 2), 3.0);
            nvgFillColor(ctx, nvgRGBAf(0, 0, 0, 255));
            nvgFill(ctx);

            nvgBeginPath(ctx);
            nvgCircle(ctx, bufferWidth - 155, pY + (colorBoxHeight / 2), 2.0);
            nvgFillColor(ctx, nvgRGBAf(255, 255, 255, 255));
            nvgFill(ctx);
            break;

          case kLActive:
            // Ping Indicator
            NVGcolor pingColor;
            if (rtt != 0 && thisPlayer->Presence() != kzSpectating) { // Don't draw ping for yourself or spectators
              if (rtt < 100) {  // Green + Small
                pingColor = ColorManager::getPingColor(0).IntoNVG();
                pingBarHeight = 2;
              } else if (rtt <= 200) { // Yellow + Medium
                pingColor = ColorManager::getPingColor(1).IntoNVG();
                pingBarHeight = 6;
              } else if (rtt > 200) { // Red + Large
                pingColor = ColorManager::getPingColor(2).IntoNVG();
                pingBarHeight = 10;
              }
              nvgBeginPath(ctx);
              nvgRect(ctx, bufferWidth - 147, pY + (10 - pingBarHeight), 3, pingBarHeight);
              nvgFillColor(ctx, pingColor);
              nvgFill(ctx);
            }
          break;

          // Draw a slash
          case kLNetDelayed:
            nvgBeginPath(ctx);
            nvgMoveTo(ctx, bufferWidth - 150, pY);
            nvgLineTo(ctx, bufferWidth - 160, pY + colorBoxHeight);
            nvgStrokeColor(ctx, nvgRGBAf(0, 0, 0, 255));
            nvgStrokeWidth(ctx, 1);
            nvgStroke(ctx);
            nvgClosePath(ctx);
          break;

          default:
            break;
        }

        //player name
        nvgFillColor(ctx, textColor);
        nvgTextAlign(ctx, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
        nvgFontSize(ctx, fontsz_m);
        nvgText(ctx, bufferWidth - 141, pY - 2, playerName.c_str(), NULL);

        short status = thisPlayer->GetStatusChar();
        if (status >= 0) {
            std::string playerLives = std::to_string(status);
            if (status == 10) playerLives = "%";
            if (status == 12) playerLives = "C";
            if (thisPlayer->Presence() == kzSpectating) playerLives = "*";
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
        nvgText(ctx, bufferWidth - 168, pY - 2, playerChat.c_str(), NULL);

        //spectating onscreen name
        if(spectatePlayer != NULL && thisPlayer->GetPlayer() == spectatePlayer) {
            int x = (int)(bufferWidth / 2.0);
            int y = 50;
            float fontsz_m = 24.0;
            float fontsz_s = 18.0;
            float bounds[4], nextBounds[4], prevBounds[4];
            std::string specMessage("Spectating " + playerName);
            std::string nextMessage("Spectate Next: ]");
            std::string prevMessage("Spectate Previous: [");

            nvgBeginPath(ctx);
            nvgFontFace(ctx, "mono");
            nvgTextAlign(ctx, NVG_ALIGN_MIDDLE | NVG_ALIGN_BOTTOM);
            nvgFontSize(ctx, fontsz_m);
            nvgTextBounds(ctx, x,y, specMessage.c_str(), NULL, bounds);

            //draw box for text
            nvgBeginPath(ctx);
            nvgRoundedRect(ctx, x - 100, y, (bounds[2]-bounds[0])+10, 28.0, 3.0);
            nvgFillColor(ctx, BACKGROUND_COLOR);
            nvgFill(ctx);

            //draw text
            nvgFillColor(ctx, nvgRGBA(255, 255, 255, 255));
            nvgText(ctx, x - 100 + 5, y + 14, specMessage.c_str(), NULL);

            nvgBeginPath(ctx);
            nvgFontFace(ctx, "mono");
            nvgTextAlign(ctx, NVG_ALIGN_MIDDLE | NVG_ALIGN_BOTTOM);
            nvgFontSize(ctx, fontsz_s);
            nvgTextBounds(ctx, x, y, nextMessage.c_str(), NULL, nextBounds);
            nvgTextBounds(ctx, x, y, prevMessage.c_str(), NULL, prevBounds);

            //Spectate Next
            nvgBeginPath(ctx);
            nvgFontSize(ctx, fontsz_s);
            nvgRoundedRect(ctx, x + 25, y + 30, (nextBounds[2]-nextBounds[0])+10, 28.0, 3.0);
            nvgFillColor(ctx, BACKGROUND_COLOR);
            nvgFill(ctx);

            nvgFillColor(ctx, nvgRGBA(255, 255, 255, 255));
            nvgText(ctx, x + 25 + 5, y + 44, nextMessage.c_str(), NULL);
            
            //Spectate Previous
            nvgBeginPath(ctx);
            nvgRoundedRect(ctx, x - 220, y + 30, (prevBounds[2]-prevBounds[0])+10, 28.0, 3.0);
            nvgFillColor(ctx, BACKGROUND_COLOR);
            nvgFill(ctx);

            nvgFillColor(ctx, nvgRGBA(255, 255, 255, 255));
            nvgText(ctx, x - 220 + 5, y + 44, prevMessage.c_str(), NULL);
        }
    }

    DrawKillFeed(ctx, net, bufferWidth, fontsz_l);

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
        ColorManager::getEnergyGaugeColor().IntoNVG(),
        ColorManager::getPlasmaGauge1Color().IntoNVG(),
        ColorManager::getPlasmaGauge2Color().IntoNVG(),
        ColorManager::getShieldGaugeColor().IntoNVG()
    };
    NVGcolor g2c[] = {
        ColorManager::getPinwheel1Color().IntoNVG(),
        ColorManager::getPinwheel2Color().IntoNVG(),
        ColorManager::getPinwheel3Color().IntoNVG(),
        ColorManager::getPinwheel4Color().IntoNVG()
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
        nvgFillColor(ctx, ColorManager::getNetDelay2Color().IntoNVG());
        nvgFill(ctx);
    }
    if (itsGame->longWait) {
        nvgBeginPath(ctx);
        nvgRect(ctx, g1X + 4, gY + 4, 8.0, 8.0);
        nvgFillColor(ctx, ColorManager::getNetDelay1Color().IntoNVG());
        nvgFill(ctx);
    }
    nvgFillColor(ctx, nvgRGBA(255, 255, 255, 255));
    nvgFontSize(ctx, fontsz_m);
    nvgTextAlign(ctx, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
    if(player->itsManager != nullptr) {
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

void CHUD::DrawShadowBox(NVGcontext *ctx, int x, int y, int width, int height) {
    NVGpaint shadowPaint = nvgBoxGradient(ctx, x, y+2, width, height, 8, 10, BACKGROUND_COLOR, nvgRGBA(0,0,0,0));
    nvgBeginPath(ctx);
    nvgRect(ctx, x-10,y-10, width+20, height+30);
    nvgRoundedRect(ctx, x,y, width, height, 4.0);
    nvgPathWinding(ctx, NVG_HOLE);
    nvgFillPaint(ctx, shadowPaint);
    nvgFill(ctx);
}

void CHUD::DrawKillFeed(NVGcontext *ctx, CNetManager *net, int bufferWidth, float fontSize) {
    // Kill Events
    if (itsGame->itsApp->Get(kHUDShowKillFeed) && net->isPlaying && !itsGame->scoreEventList.empty()) {
        float killEventPosition[2] = {(float)bufferWidth - 50.0f, 200.0f};
        float killEventSize[2] = {0, 0};

        float killEventBounds[4] = {0,0,0,0};
        float killEventKillerNameBounds[4] = {0,0,0,0};

        float killEventIconXPosition = 0;

        int eventCount = 0;
        for (auto iter = itsGame->scoreEventList.begin(); iter != itsGame->scoreEventList.end(); iter++) {
            ScoreInterfaceEvent event = *iter;
            float imgWidth = 36.0;
            float eventPositionY;

            switch(event.scoreType) {
                case ksiKillBonus: {
                    // Event player names
                    std::string killerName = " " + event.player;
                    std::string killedName = " " + event.playerTarget;
                    float teamColorRGB[3], teamTargetColorRGB[3], iconLeftMargin = 23.0f;

                    // Get how wide the event rect needs to be
                    std::string eventText = killerName + "     " + killedName;
                    if (event.weaponUsed == ksiObjectCollision) {
                        eventText = "    " + killedName;
                        killerName = "";
                        iconLeftMargin = 11.0f;
                    }
                    nvgBeginPath(ctx);
                    nvgTextAlign(ctx, NVG_ALIGN_RIGHT);
                    nvgFontSize(ctx, fontSize);
                    nvgFillColor(ctx, nvgRGBA(255, 255, 255, 255));
                    nvgTextBounds(ctx, 0, 0, eventText.c_str(), NULL, killEventBounds);             // Size of the entire rendered text
                    nvgTextBounds(ctx, 0, 0, killerName.c_str(), NULL, killEventKillerNameBounds);  // Size of the killer name text (used to place the icon)

                    // Get Measurements based on text size for the total size of the event box
                    killEventSize[0] = killEventBounds[2] - killEventBounds[0] + 20.0f;
                    killEventSize[1] = killEventBounds[3] - killEventBounds[1] + 10.0f;
                    killEventIconXPosition = killEventKillerNameBounds[2] - killEventKillerNameBounds[0] + iconLeftMargin;   // Position the icon in the empty space
                    eventPositionY = killEventPosition[1] + ((float)eventCount * (killEventSize[1] + 10.0f));

                    // Background box
                    DrawShadowBox(ctx, killEventPosition[0] - killEventSize[0], eventPositionY, killEventSize[0], killEventSize[1]);
                    nvgBeginPath(ctx);
                    nvgRoundedRect(ctx, killEventPosition[0] - killEventSize[0], eventPositionY, killEventSize[0], killEventSize[1], 4.0);
                    nvgFillColor(ctx, BACKGROUND_COLOR);
                    nvgFill(ctx);

                    // Locations/Dimensions of the icons in the sprite atlas
                    float ix, iy, iWidth, iHeight;
                    switch(event.weaponUsed) {
                        case ksiPlasmaHit:
                            ix = 118.0;
                            iy = 0.0;
                            iWidth = 55.0;
                            iHeight = 51.0;
                            break;
                        case ksiGrenadeHit:
                            ix = 0.0;
                            iy = 0.0;
                            iWidth = 60.0;
                            iHeight = 51.0;
                            break;
                        case ksiMissileHit:
                            ix = 60.0;
                            iy = 0.0;
                            iWidth = 58.0;
                            iHeight = 51.0;
                            break;
                        case ksiObjectCollision:
                            ix = 173.0;
                            iy = 0.0;
                            iWidth = 58.0;
                            iHeight = 51.0;
                            break;
                        default:
                            ix = 0.0;
                            iy = 0.0;
                            iWidth = 0.0;
                            iHeight = 0.0;
                            break;
                    }
                    ARGBColor longTeamColor = *ColorManager::getTeamColor(event.team);
                    longTeamColor.ExportGLFloats(teamColorRGB, 3);
                    ARGBColor longTeamTargetColor = *ColorManager::getTeamColor(event.teamTarget);
                    longTeamTargetColor.ExportGLFloats(teamTargetColorRGB, 3);
                    float teamTargetColorPosition = killEventPosition[0] - killEventSize[0] + killEventIconXPosition + imgWidth + 15.0f;

                    // Team color of killer
                    if (event.weaponUsed != ksiObjectCollision) {
                        nvgBeginPath(ctx);
                        nvgRect(ctx, killEventPosition[0] - killEventSize[0] + 10.0f, eventPositionY + 10.0f, 9.0, killEventSize[1] - 20.0f);
                        nvgFillColor(ctx, nvgRGBAf(teamColorRGB[0], teamColorRGB[1], teamColorRGB[2], 1.0));
                        nvgFill(ctx);
                    }
                    
                    // Team color of killed
                    nvgBeginPath(ctx);
                    nvgRect(ctx, teamTargetColorPosition, eventPositionY + 10.0f, 9.0, killEventSize[1] - 20.0f);
                    nvgFillColor(ctx, nvgRGBAf(teamTargetColorRGB[0], teamTargetColorRGB[1], teamTargetColorRGB[2], 1.0));
                    nvgFill(ctx);

                    // Draw icon for death reason
                    DrawImage(ctx, images, 1.0, ix, iy, iWidth, iHeight, 
                        killEventPosition[0] - killEventSize[0] + killEventIconXPosition, eventPositionY + 5.0f, imgWidth, killEventSize[1] - 10.0f);

                    // Draw display text for event
                    nvgBeginPath(ctx);
                    nvgTextAlign(ctx, NVG_ALIGN_LEFT);
                    nvgFontSize(ctx, fontSize);
                    nvgFillColor(ctx, nvgRGBA(255, 255, 255, 255));
                    nvgText(ctx, killEventPosition[0] - killEventSize[0] + 10.0f, eventPositionY + 25.0f, eventText.c_str(), NULL);

                    eventCount++;
                    break;
                }

                case ksiScoreGoal:
                case ksiGrabBall: {
                    // Event player names
                    std::string playerName = " " + event.player;
                    float teamColorRGB[3];

                    std::string eventText;
                    if (event.scoreType == ksiScoreGoal) {
                        eventText = playerName + " scored!";
                    } else if (event.scoreType == ksiGrabBall) {
                        eventText = playerName + " has the ball!";
                    } else {
                        eventText = "";
                    }
                    // Get how wide the event rect needs to be
                    nvgBeginPath(ctx);
                    nvgTextAlign(ctx, NVG_ALIGN_RIGHT);
                    nvgFontSize(ctx, fontSize);
                    nvgFillColor(ctx, nvgRGBA(255, 255, 255, 255));
                    nvgTextBounds(ctx, 0, 0, eventText.c_str(), NULL, killEventBounds);             // Size of the entire rendered text

                    // Get Measurements based on text size for the total size of the event box
                    killEventSize[0] = killEventBounds[2] - killEventBounds[0] + 20.0f;
                    killEventSize[1] = killEventBounds[3] - killEventBounds[1] + 10.0f;
                    eventPositionY = killEventPosition[1] + ((float)eventCount * (killEventSize[1] + 10.0f));

                    // Background box
                    DrawShadowBox(ctx, killEventPosition[0] - killEventSize[0], eventPositionY, killEventSize[0], killEventSize[1]);
                    nvgBeginPath(ctx);
                    nvgRoundedRect(ctx, killEventPosition[0] - killEventSize[0], eventPositionY, killEventSize[0], killEventSize[1], 4.0);
                    nvgFillColor(ctx, BACKGROUND_COLOR);
                    nvgFill(ctx);

                    ARGBColor longTeamColor = *ColorManager::getTeamColor(event.team);
                    longTeamColor.ExportGLFloats(teamColorRGB, 3);

                    // Draw player's team color
                    nvgBeginPath(ctx);
                    nvgRect(ctx, killEventPosition[0] - killEventSize[0] + 10.0f, eventPositionY + 10.0f, 9.0, killEventSize[1] - 20.0f);
                    nvgFillColor(ctx, nvgRGBAf(teamColorRGB[0], teamColorRGB[1], teamColorRGB[2], 1.0));
                    nvgFill(ctx);

                    // Draw event text
                    nvgBeginPath(ctx);
                    nvgTextAlign(ctx, NVG_ALIGN_LEFT);
                    nvgFontSize(ctx, fontSize);
                    nvgFillColor(ctx, nvgRGBA(255, 255, 255, 255));
                    nvgText(ctx, killEventPosition[0] - killEventSize[0] + 10.0f, eventPositionY + 25.0f, eventText.c_str(), NULL);

                    eventCount++;
                    break;
                }

                /*case ksiHoldBall:
                    break;*/
                default:
                    break;
            }

        }
    }
}

void CHUD::RenderNewHUD(NVGcontext *ctx) {
    auto view = gRenderer->viewParams;
    CAbstractPlayer *player = itsGame->GetLocalPlayer();
    CAbstractPlayer *spectatePlayer = itsGame->GetSpectatePlayer();
    CNetManager *net = itsGame->itsApp->GetNet();

    DrawLevelName(ctx);
    DrawPaused(ctx);

    std::vector<CPlayerManager*> thePlayers = net->ActivePlayers(); // only players actively playing

    int playerCount = (int)net->PlayerCount();
    int highestUsedSlot = 0;
    // Find the highest numbered occupied slot
    // Empty slots will only show up if they are between occupied slots
    for (auto thisPlayer: net->AllPlayers()) {
        highestUsedSlot = std::max(highestUsedSlot, (int)(thisPlayer->Slot() + 1));
    }

    int bufferWidth = view->viewPixelDimensions.h, bufferHeight = view->viewPixelDimensions.v;
    int chudHeight = 13 * playerCount;

    DrawScore(thePlayers, chudHeight, ctx);
    DrawEditingHud(player, ctx);

    nvgBeginFrame(ctx, bufferWidth, bufferHeight, view->viewPixelRatio);

    float fontsz_l = 25.0, fontsz_m = 17.0, fontsz_s = 12.0;
    nvgFontFace(ctx, "mono");

    // Parameters for drawing UI elements on the screen
    // Units for position and size are set as a percentage of the screen size to make elements look the same after the screen resolution changes
    float levelMessagePosition[2] = {50.0f, (float)bufferHeight - 240.0f};
    float levelMessageSize[2] = {350.0f, 90.0f};
    int levelMessageSpacing = 16;
    int levelMessageMaxLines = 5;

    float systemMessagePosition[2] = {50.0f, (float)bufferHeight - 140.0f};
    float systemMessageSize[2] = {520.0f, 90.0f};
    int systemMessageSpacing = 16;
    int systemMessageMaxLines = 5;

    int playerLineHeight = 17;
    float playerListPosition[2] = {(float)bufferWidth - 570.0f, (float)bufferHeight - 60.0f - (highestUsedSlot * playerLineHeight)};
    float playerListSize[2] = {520.0f, (highestUsedSlot * playerLineHeight) + 10.0f};
    float timePosition[2] = {(float)bufferWidth - 350.0f, playerListPosition[1] - 37.0f};
    float timeSize[2] = {75.0f, 27.0f};
    float scorePosition[2] = {(float)bufferWidth - 500.0f, playerListPosition[1] - 37.0f};
    float scoreSize[2] = {75.0f, 27.0f};
    float spectatePlayerPosition[2] = {(float)bufferWidth / 2.0f, 50.0f};

    // System Message Backdrop
    if (itsGame->itsApp->Get(kHUDShowSystemMessages)) {
        DrawShadowBox(ctx, systemMessagePosition[0], systemMessagePosition[1], systemMessageSize[0], systemMessageSize[1]);
        nvgBeginPath(ctx);
        nvgRoundedRect(ctx, systemMessagePosition[0], systemMessagePosition[1], systemMessageSize[0], systemMessageSize[1], 4.0);
        nvgFillColor(ctx, BACKGROUND_COLOR);
        nvgFill(ctx);
    } else {
        systemMessageMaxLines = 0;
    }

    // Level Messages
    // Backdrop is not drawn until a level message is going to be displayed
    if (!itsGame->itsApp->Get(kHUDShowLevelMessages)) {
        levelMessageMaxLines = 0;
    }

    // Player List Backdrop
    if (itsGame->itsApp->Get(kHUDShowPlayerList)) {
        DrawShadowBox(ctx, playerListPosition[0], playerListPosition[1], playerListSize[0], playerListSize[1]);
        nvgBeginPath(ctx);
        nvgRoundedRect(ctx, playerListPosition[0], playerListPosition[1], playerListSize[0], playerListSize[1], 4.0);
        nvgFillColor(ctx, BACKGROUND_COLOR);
        nvgFill(ctx);
    }

    DrawKillFeed(ctx, net, bufferWidth, fontsz_l);

    // Read the message list starting at the end and reading backwards.
    // As newer messages are added, older messages travel up in the display
    // Messages will be displayed on the screen like this:
    //
    // Less Recent  ^  Read last
    //              |
    //              |
    // Most Recent  |  Read first
    float mX = 0, mY = 0, sX = 0;
    int levelMsgCount = 0, sysMsgCount = 0;
    short textAlign = NVG_ALIGN_LEFT;
    for (auto iter = itsGame->itsApp->MessageLines().rbegin(); iter != itsGame->itsApp->MessageLines().rend(); iter++) {
        MsgLine msg = *iter;

        // Filter messages that are older than the current game
        // Don't keep reading messages when a message from a previous game is found
        if (msg.gameId < itsGame->currentGameId) break; 

        // Set initial message params based on category
        // This determines which display panel the message gets added to
        // Check if we can still display messages of the given category
        switch (msg.category) {
            case MsgCategory::System:
            case MsgCategory::Error:
                mX = systemMessagePosition[0];
                sX = systemMessageSize[0];
                mY = (systemMessagePosition[1] - 20.0 + systemMessageSize[1]) - (float)(systemMessageSpacing * sysMsgCount);
                if (sysMsgCount < systemMessageMaxLines) {
                    sysMsgCount++;
                } else {
                    // This queue is full. Move on to the next message
                    continue;
                }
                break;
            case MsgCategory::Level:
                mX = levelMessagePosition[0];
                sX = levelMessageSize[0];
                mY = (levelMessagePosition[1] - 20.0 + levelMessageSize[1]) - (float)(levelMessageSpacing * levelMsgCount);
                if (levelMsgCount < levelMessageMaxLines) {
                    // Level Message Backdrop -- Only draw this once, the first time a level message is found
                    if (levelMsgCount == 0) {
                        DrawShadowBox(ctx, levelMessagePosition[0], levelMessagePosition[1], levelMessageSize[0], levelMessageSize[1]);
                        nvgBeginPath(ctx);
                        nvgRoundedRect(ctx, levelMessagePosition[0], levelMessagePosition[1], levelMessageSize[0], levelMessageSize[1], 4.0);
                        nvgFillColor(ctx, BACKGROUND_COLOR);
                        nvgFill(ctx);
                    }

                    levelMsgCount++;
                } else {
                    // This queue is full. Move on to the next message
                    continue;
                }
                break;
        }

        // Message alignment
        switch (msg.align) {
            case MsgAlignment::Left:
                mX += 10.0;
                textAlign = NVG_ALIGN_LEFT;
                break;
            case MsgAlignment::Center:
                if (msg.category == MsgCategory::Level) {
                    mX += 10.0;
                    textAlign = NVG_ALIGN_LEFT;
                    break;
                }
                mX += (sX / 2.0);
                textAlign = NVG_ALIGN_CENTER;
                break;
            case MsgAlignment::Right:
                mX += sX - 10;
                textAlign = NVG_ALIGN_RIGHT;
                break;
        }

        // Draw the message
        nvgBeginPath(ctx);
        nvgTextAlign(ctx, textAlign | NVG_ALIGN_TOP);
        nvgFontSize(ctx, fontsz_m);
        nvgFillColor(ctx, ColorManager::getMessageColor(msg.category).IntoNVG());
        nvgText(ctx, mX, mY, msg.text.c_str(), NULL);

        // If all display queues are full then we don't need to continue reading messages
        if (sysMsgCount >= systemMessageMaxLines && levelMsgCount >= levelMessageMaxLines) {
            break;
        }
    }


    float pY;
    ARGBColor longTeamColor = 0;
    float teamColorRGB[3];
    float colorBoxAlpha = 1.0;
    if (itsGame->itsApp->Get(kHUDShowPlayerList)) {
        for (int i = 0; i < kMaxAvaraPlayers; i++) {
            CPlayerManager *thisPlayer = net->playerTable[i].get();
            std::string playerName = thisPlayer->GetPlayerName();
            if (playerName.length() < 1) continue;

            // Get player RTT
            long rtt = 0;
            if (i != net->itsCommManager->myId && playerName.length() > 0) {
                rtt = net->itsCommManager->GetMaxRoundTrip(1 << i);
            }

            pY = (playerListPosition[1] + 8) + (playerLineHeight * i); // Base y position for the entire player line
            longTeamColor = *ColorManager::getTeamColor(net->teamColors[i] + 1);
            longTeamColor.ExportGLFloats(teamColorRGB, 3);
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
            float colorBoxWidth = 15.0;
            float colorBoxHeight = 15.0;
            float pingBarHeight = 0.0;
            nvgBeginPath(ctx);

            //highlight player if spectating
            if (spectatePlayer != NULL && thisPlayer->GetPlayer() == spectatePlayer) {
                textColor = (*ColorManager::getTeamTextColor(net->teamColors[i] + 1)).IntoNVG();
                colorBoxWidth = 142.0;
            }

            nvgRect(ctx, playerListPosition[0] + 370, pY - 2, colorBoxWidth, colorBoxHeight);
            nvgFillColor(ctx, nvgRGBAf(teamColorRGB[0], teamColorRGB[1], teamColorRGB[2], colorBoxAlpha));
            nvgFill(ctx);

            // draw something based on the loading status
            switch (thisPlayer->LoadingStatus()) {
            // Draw a dot
            case kLReady:
                nvgBeginPath(ctx);
                nvgCircle(ctx, playerListPosition[0] + 378, pY + (colorBoxHeight / 2) - 2, 5.0);
                nvgFillColor(ctx, nvgRGBAf(0, 0, 0, 255));
                nvgFill(ctx);

                nvgBeginPath(ctx);
                nvgCircle(ctx, playerListPosition[0] + 378, pY + (colorBoxHeight / 2) - 2, 4.0);
                nvgFillColor(ctx, nvgRGBAf(255, 255, 255, 255));
                nvgFill(ctx);
                break;

            case kLActive:
                // Ping Indicator
                NVGcolor pingColor;
                if (rtt != 0 && thisPlayer->Presence() != kzSpectating) { // Don't draw ping for yourself or spectators
                if (rtt < 100) {  // Green + Small
                    pingColor = ColorManager::getPingColor(0).IntoNVG();
                    pingBarHeight = 3;
                } else if (rtt <= 200) { // Yellow + Medium
                    pingColor = ColorManager::getPingColor(1).IntoNVG();
                    pingBarHeight = 9;
                } else if (rtt > 200) { // Red + Large
                    pingColor = ColorManager::getPingColor(2).IntoNVG();
                    pingBarHeight = 16;
                }
                nvgBeginPath(ctx);
                nvgRect(ctx, playerListPosition[0] + 389, pY + (10 - pingBarHeight) + 3, 5, pingBarHeight);
                nvgFillColor(ctx, pingColor);
                nvgFill(ctx);
                }
            break;

            // Draw a slash
            case kLNetDelayed:
                nvgBeginPath(ctx);
                nvgMoveTo(ctx, playerListPosition[0] + 371, pY + colorBoxHeight - 2);
                nvgLineTo(ctx, playerListPosition[0] + 385, pY - 2);
                nvgStrokeColor(ctx, nvgRGBAf(0, 0, 0, 255));
                nvgStrokeWidth(ctx, 1);
                nvgStroke(ctx);
                nvgClosePath(ctx);
            break;

            default:
                break;
            }

            //player name
            nvgFillColor(ctx, textColor);
            nvgTextAlign(ctx, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
            nvgFontSize(ctx, fontsz_m);
            nvgText(ctx, playerListPosition[0] + 396, pY - 2, playerName.c_str(), NULL);

            // Player status
            short status = thisPlayer->GetStatusChar();
            if (status >= 0) {
                std::string playerLives = std::to_string(status);
                if (status == 10) playerLives = "%";
                if (status == 12) playerLives = "C";
                if (thisPlayer->Presence() == kzSpectating) playerLives = "*";
                nvgTextAlign(ctx, NVG_ALIGN_RIGHT | NVG_ALIGN_TOP);
                nvgFontSize(ctx, fontsz_s);
                nvgFillColor(ctx, nvgRGBA(255, 255, 255, 255));
                nvgText(ctx, playerListPosition[0] + 369, pY - 3, playerLives.c_str(), NULL);
                if (thisPlayer->GetMessageIndicator() > 0) {
                    nvgText(ctx, playerListPosition[0] + 369, pY + 4, "<", NULL);
                }
            }

            // Player message string
            nvgTextAlign(ctx, NVG_ALIGN_RIGHT | NVG_ALIGN_TOP);
            nvgFontSize(ctx, fontsz_m);
            nvgFillColor(ctx, nvgRGBA(255, 255, 255, 255));
            nvgText(ctx, playerListPosition[0] + 360, pY - 2, playerChat.c_str(), NULL);

            //spectating onscreen name
            if(spectatePlayer != NULL && thisPlayer->GetPlayer() == spectatePlayer) {
                float fontsz_m = 24.0;
                float fontsz_s = 18.0;
                float bounds[4], nextBounds[4], prevBounds[4], toggleBounds[4];
                std::string on("On");
                std::string off("Off");

                std::string specMessage("Spectating " + playerName);
                std::string nextMessage("Spectate Next: ]");
                std::string prevMessage("Spectate Previous: [");
                std::string toggleCamMessage("Toggle Free Cam: Space Bar");
                std::string camStatusMessage("Free Cam: " + (player->freeView ? on : off));

                if (!player->freeView || (player->freeView && player->IsFreeCamAttached())) {
                    nvgBeginPath(ctx);
                    nvgFontFace(ctx, "mono");
                    nvgTextAlign(ctx, NVG_ALIGN_MIDDLE | NVG_ALIGN_BOTTOM);
                    nvgFontSize(ctx, fontsz_m);
                    nvgTextBounds(ctx, spectatePlayerPosition[0], spectatePlayerPosition[1], specMessage.c_str(), NULL, bounds);

                    //draw box for text
                    nvgBeginPath(ctx);
                    nvgRoundedRect(ctx, spectatePlayerPosition[0] - 100, spectatePlayerPosition[1], (bounds[2]-bounds[0])+10, 28.0, 3.0);
                    nvgFillColor(ctx, BACKGROUND_COLOR);
                    nvgFill(ctx);

                    //draw text
                    nvgFillColor(ctx, nvgRGBA(255, 255, 255, 255));
                    nvgText(ctx, spectatePlayerPosition[0] - 100 + 5, spectatePlayerPosition[1] + 14, specMessage.c_str(), NULL);
                }

                nvgBeginPath(ctx);
                nvgFontFace(ctx, "mono");
                nvgTextAlign(ctx, NVG_ALIGN_MIDDLE | NVG_ALIGN_BOTTOM);
                nvgFontSize(ctx, fontsz_s);
                nvgTextBounds(ctx, spectatePlayerPosition[0], spectatePlayerPosition[1], nextMessage.c_str(), NULL, nextBounds);
                nvgTextBounds(ctx, spectatePlayerPosition[0], spectatePlayerPosition[1], prevMessage.c_str(), NULL, prevBounds);
                nvgTextBounds(ctx, spectatePlayerPosition[0], spectatePlayerPosition[1], toggleCamMessage.c_str(), NULL, toggleBounds);

                //Spectate Next
                nvgBeginPath(ctx);
                nvgFontSize(ctx, fontsz_s);
                nvgRoundedRect(ctx, spectatePlayerPosition[0] + 25, spectatePlayerPosition[1] + 30, (nextBounds[2]-nextBounds[0])+10, 28.0, 3.0);
                nvgFillColor(ctx, BACKGROUND_COLOR);
                nvgFill(ctx);

                nvgFillColor(ctx, nvgRGBA(255, 255, 255, 255));
                nvgText(ctx, spectatePlayerPosition[0] + 25 + 5, spectatePlayerPosition[1] + 44, nextMessage.c_str(), NULL);
                
                //Spectate Previous
                nvgBeginPath(ctx);
                nvgRoundedRect(ctx, spectatePlayerPosition[0] - 220, spectatePlayerPosition[1] + 30, (prevBounds[2]-prevBounds[0])+10, 28.0, 3.0);
                nvgFillColor(ctx, BACKGROUND_COLOR);
                nvgFill(ctx);

                nvgFillColor(ctx, nvgRGBA(255, 255, 255, 255));
                nvgText(ctx, spectatePlayerPosition[0] - 220 + 5, spectatePlayerPosition[1] + 44, prevMessage.c_str(), NULL);

                // Toggle Free Cam Message
                nvgBeginPath(ctx);
                nvgRoundedRect(ctx, spectatePlayerPosition[0] - 270, spectatePlayerPosition[1] + 60, (toggleBounds[2]-toggleBounds[0])+10, 28.0, 3.0);
                nvgFillColor(ctx, BACKGROUND_COLOR);
                nvgFill(ctx);

                nvgFillColor(ctx, nvgRGBA(255, 255, 255, 255));
                nvgText(ctx, spectatePlayerPosition[0] - 270 + 5, spectatePlayerPosition[1] + 74, toggleCamMessage.c_str(), NULL);

                // Free Cam Status Message
                nvgBeginPath(ctx);
                nvgFontSize(ctx, fontsz_s);
                nvgRoundedRect(ctx, spectatePlayerPosition[0] + 25, spectatePlayerPosition[1] + 60, (nextBounds[2]-nextBounds[0])+10, 28.0, 3.0);
                if (player->freeView) {
                    nvgFillColor(ctx, nvgRGBA(0, 50, 0, 180));
                } else {
                    nvgFillColor(ctx, nvgRGBA(50, 0, 0, 180));
                }
                nvgFill(ctx);

                nvgFillColor(ctx, nvgRGBA(255, 255, 255, 255));
                nvgText(ctx, spectatePlayerPosition[0] + 25 + 5, spectatePlayerPosition[1] + 74, camStatusMessage.c_str(), NULL);
            }
        }
    }

    if (!player) {
        return;
    }

    if(spectatePlayer != NULL)
        player = spectatePlayer;

    char scoreText[20];
    char timeText[9];

    // Score Backdrop
    if (itsGame->itsApp->Get(kHUDShowScore)) {
        DrawShadowBox(ctx, scorePosition[0], scorePosition[1], scoreSize[0], scoreSize[1]);
        nvgBeginPath(ctx);
        nvgRoundedRect(ctx, scorePosition[0], scorePosition[1], scoreSize[0], scoreSize[1], 4.0);
        nvgFillColor(ctx, BACKGROUND_COLOR);
        nvgFill(ctx);
    }

    // Draw Score
    if (itsGame->itsApp->Get(kHUDShowScore)) {
        nvgBeginPath(ctx);
        nvgFillColor(ctx, nvgRGBA(255, 255, 255, 255));
        nvgFontSize(ctx, fontsz_l);
        nvgTextAlign(ctx, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
        if(player->itsManager != nullptr) {
            snprintf(scoreText, sizeof(scoreText), "%ld", itsGame->scores[player->itsManager->Slot()]);
            nvgText(ctx, scorePosition[0] + 5.0f, scorePosition[1] + 14.0f, scoreText, NULL);
        }
    }

    // Time Backdrop
    if (itsGame->itsApp->Get(kHUDShowTime)) {
        DrawShadowBox(ctx, timePosition[0], timePosition[1], timeSize[0], timeSize[1]);
        nvgBeginPath(ctx);
        nvgRoundedRect(ctx, timePosition[0], timePosition[1], timeSize[0], timeSize[1], 4.0);
        nvgFillColor(ctx, BACKGROUND_COLOR);
        nvgFill(ctx);
    }

    // Draw time elapsed
    if (itsGame->itsApp->Get(kHUDShowTime)) {
        long timeTemp = itsGame->timeInSeconds;
        int secs, mins;
        secs = timeTemp % 60;
        timeTemp /= 60;
        mins = timeTemp % 60;
        snprintf(timeText, sizeof(timeText), "%02d:%02d", mins, secs);
        nvgFillColor(ctx, nvgRGBA(255, 255, 255, 255));
        nvgTextAlign(ctx, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
        nvgText(ctx, timePosition[0] + 5.0f, timePosition[1] + 14.0f, timeText, NULL);
    }

    nvgEndFrame(ctx);
}
