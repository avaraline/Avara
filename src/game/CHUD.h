#include "CDirectObject.h"
#include "CViewParameters.h"
#include "nanovg.h"

#include <string>
#include <vector>

class CAvaraGame;

class CHUD : CDirectObject {
public:
    CAvaraGame *itsGame;

    CHUD(CAvaraGame *game);
    virtual ~CHUD() {}

    void Render(CViewParameters *view, NVGcontext *ctx);
    void DrawLevelName(CViewParameters *view, NVGcontext *ctx);
    void DrawPaused(CViewParameters *view, NVGcontext *ctx);
    void DrawScore(int playingCount, int chudHeight, CViewParameters *view, NVGcontext *ctx);
    void DrawRoster(int playingCount, int chudHeight, CViewParameters *view, NVGcontext *ctx);
    void DrawLevelInfo(int chudHeight, CViewParameters *view, NVGcontext *ctx);
    void DrawTable(std::string title, float x, float y, std::vector<std::pair<std::string, std::string>> data, CViewParameters *view, NVGcontext *ctx);

};
