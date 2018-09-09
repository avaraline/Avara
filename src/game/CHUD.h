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
};
