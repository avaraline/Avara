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
    CAbstractPlayer *itsPlayer = 0;
    CViewParameters *freeParams = 0;

    short action = 0;
    Fixed radius = 0, heading = 0, pitch = 0;
    short camSpeed = 0;
    Boolean isAttached = 0;

    CFreeCam(CAbstractPlayer *thePlayer);
    virtual void FrameAction();
    virtual void ViewControl(FunctionTable *ft);
    virtual void ToggleState(Boolean state);
    virtual Boolean IsAttached();
    virtual void SetAttached(Boolean attach);
    virtual void ControlViewPoint();
};