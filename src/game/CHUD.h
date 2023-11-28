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

    void Render(CViewParameters *view, NVGcontext *ctx);
    void RenderNewHUD(CViewParameters *view, NVGcontext *ctx);
    void DrawLevelName(CViewParameters *view, NVGcontext *ctx);
    void DrawPaused(CViewParameters *view, NVGcontext *ctx);
    void DrawScore(std::vector<CPlayerManager*>& thePlayers, int chudHeight, CViewParameters *view, NVGcontext *ctx);
    void DrawShadowBox(NVGcontext *ctx, int x, int y, int height, int width);
    void LoadImages(NVGcontext *ctx);
    void DrawImage(NVGcontext* ctx, int image, float alpha, float sx, float sy, float sw, float sh, float x, float y, float w, float h);

};
