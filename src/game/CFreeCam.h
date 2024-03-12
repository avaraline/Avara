#pragma once
#include "CRealMovers.h"
#include "CAbstractPlayer.h"

//class AbstractPlayer;

enum {
    camInactive,
    camAnimating
};

class CFreeCam final : public CRealMovers {
//class CFreeCam final {
public:
    CAbstractPlayer *itsPlayer;
    CViewParameters *freeParams;

    short action;
    Fixed radius;
    Fixed yFromThreshold;
    short camSpeed;
    Boolean isAttached;

    CFreeCam(CAbstractPlayer *thePlayer);
    virtual void FrameAction();
    virtual void ViewControl(FunctionTable *ft);
    virtual void ToggleState(Boolean state);
    virtual Boolean IsAttached();
    virtual void SetAttached(Boolean attach);
    virtual void ControlViewPoint();
};