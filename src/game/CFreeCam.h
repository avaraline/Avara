#pragma once
#include "CRealMovers.h"
#include "CAbstractPlayer.h"

class CFreeCam final : public CRealMovers {
public:
    CAbstractPlayer *itsPlayer;
    CViewParameters *freeParams;

    Fixed radius;
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