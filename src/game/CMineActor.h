/*
    Copyright ©1995-1996, Juri Munkki
    All rights reserved.

    File: CMineActor.h
    Created: Tuesday, September 5, 1995, 08:25
    Modified: Sunday, September 8, 1996, 22:33
*/

#include "CGlowActors.h"

class CMineActor : public CGlowActors {
public:
    MaskType watchBits = 0;
    MessageRecord activator = {0};

    long phase = 0;
    long fuseTime = 0;

    long lookTime = 0;
    long lookNextTime = 0;
    long shapeNextTime = 0;
    long shapeTimes[2][2] = {0};

    Fixed shieldEnergy = 0;
    Fixed radius = 0;
    Fixed activateEnergy = 0;

    short activateSound = 0;
    Fixed activateVolume = 0;

    short curShape = 0;

    MessageRecord startMsg = {0};
    MessageRecord stopMsg = {0};
    Boolean enabled = 0;
    Boolean activated = 0;

    virtual void BeginScript();
    virtual CAbstractActor *EndScript();
    virtual void Activate();
    virtual ~CMineActor();
    virtual void FrameAction();
    virtual void WasHit(RayHitRecord *theHit, Fixed hitEnergy);
};
