#include "CDirectObject.h"
#include "CViewParameters.h"
#include "nanovg.h"

class CAvaraGame;

class CHUD : CDirectObject {
public:
    CAvaraGame *itsGame;

    CHUD(CAvaraGame *game);
    virtual ~CHUD() {}

    void Render(CViewParameters *view, NVGcontext *ctx);
    void DrawLevelName(CViewParameters *view, NVGcontext *ctx);
    void DrawPaused(CViewParameters *view, NVGcontext *ctx);
    void DrawScore(int playerCount, int chudHeight, CViewParameters *view, NVGcontext *ctx);

};
