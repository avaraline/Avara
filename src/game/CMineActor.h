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
    MaskType watchBits;
    MessageRecord activator;

    long phase;
    long fuseTime;

    long lookTime;
    long lookNextTime;
    long shapeNextTime;
    long shapeTimes[2][2];

    Fixed shieldEnergy;
    Fixed radius;
    Fixed activateEnergy;

    short activateSound;
    Fixed activateVolume;

    short curShape;

    MessageRecord startMsg;
    MessageRecord stopMsg;
    Boolean enabled;
    Boolean activated;

    virtual void BeginScript();
    virtual CAbstractActor *EndScript();
    virtual void Activate();
    virtual ~CMineActor();
    virtual void FrameAction();
    virtual void WasHit(RayHitRecord *theHit, Fixed hitEnergy);
};
