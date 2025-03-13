#pragma once

#include "CDirectObject.h"
#include <vector>
#include "CViewParameters.h"
#include "CPlayerManager.h"
#include "nanovg.h"

class CAvaraGame;

class CHUD : CDirectObject {
public:
    CAvaraGame *itsGame;
    int images;

    CHUD(CAvaraGame *game);
    virtual ~CHUD() {}

    void Render(NVGcontext *ctx);
    void RenderNewHUD(NVGcontext *ctx);
    void DrawLevelName(NVGcontext *ctx);
    void DrawPaused(NVGcontext *ctx);
    void DrawScore(std::vector<CPlayerManager*>& thePlayers, int chudHeight, NVGcontext *ctx);
    void DrawEditingHud(CAbstractPlayer *player, NVGcontext *ctx);
    void DrawShadowBox(NVGcontext *ctx, int x, int y, int height, int width);
    void DrawKillFeed(NVGcontext *ctx, CNetManager *net, int bufferWidth, float fontSize);
    void LoadImages(NVGcontext *ctx);
    void DrawImage(NVGcontext* ctx, int image, float alpha, float sx, float sy, float sw, float sh, float x, float y, float w, float h);

    char ehudText[4][128];
};
