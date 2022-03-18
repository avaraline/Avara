#pragma once
#include "CSmartBox.h"
#include "CGUI.h"


class CGUIWidget {
public:

    Rect _rect;
    std::vector<CGUIWidget> children;
    CSmartBox *_part;
    Vector _partLoc;
    NVGCall _nvgactions;

    CGUIWidget() {
        _partLoc[0] = 0;
        _partLoc[1] = 0;
        _partLoc[2] = 0;
    }
    virtual void attach(CGUI *gui) {
        gui->itsWorld->AddPart(_part);
    }
    virtual void update(CGUI *gui) = 0;
    virtual void activated() = 0;
    void render(NVGcontext *ctx) {
        _nvgactions(ctx);
    };
    virtual ~CGUIWidget() {
        //_gui->itsWorld->RemovePart(_part);
        //_part->Dispose();
    }
};
