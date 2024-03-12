#include "CFreeCam.h"

#include "CViewParameters.h"
#include "AbstractRenderer.h"
#include "CSmartPart.h"
#include "FastMat.h"

#include "Debug.h"

CFreeCam::CFreeCam(CAbstractPlayer *thePlayer) {
    itsPlayer = thePlayer;
    itsGame = thePlayer->itsGame;

    camSpeed = 350;
    radius = FIX3(25000);

    action = camInactive;
    isActive = kIsActive;
    partCount = 0;
    glow = 0;
}

void CFreeCam::ToggleState(Boolean state) {
    CAbstractPlayer *spectatePlayer = itsGame->GetSpectatePlayer();

    if (state && spectatePlayer != NULL) {
        if (action == camInactive) {
            itsGame->AddActor(this);
            itsPlayer->freeCamIdent = ident;
            action = camAnimating;
        }
        isAttached = true;
    }
}

void CFreeCam::SetAttached(Boolean attach) {
    isAttached = attach;
}

Boolean CFreeCam::IsAttached() {
    return isAttached && itsGame->GetSpectatePlayer() != NULL;
}

void CFreeCam::ViewControl(FunctionTable *ft) {    
    auto vp = gRenderer->viewParams;
    Vector direction;
    CAbstractPlayer *spectatePlayer = itsGame->GetSpectatePlayer();

    if (isAttached && spectatePlayer != NULL) {
        vp->LookAtPart(spectatePlayer->viewPortPart);
    }

    // Find direction vector for the camera
    direction[0] = vp->atPoint[0] - vp->fromPoint[0];
    direction[1] = vp->atPoint[1] - vp->fromPoint[1];
    direction[2] = vp->atPoint[2] - vp->fromPoint[2];

    // Find length of direction vector
    Fixed len = FSqrt(FMul(direction[0], direction[0]) + FMul(direction[2], direction[2]));

    // Normalize Direction vector
    direction[0] = FDivNZ(direction[0], len);
    //direction[1] = FDivNZ(direction[1], len);
    direction[2] = FDivNZ(direction[2], len);
    
    // Calc the horizontal viewing angle from the direction vector
    Fixed heading, pitch;
    heading = FOneArcTan2(direction[0], direction[2]);
    
    // Find new direction vector if mouse moved
    Fixed xRot=0, zRot=0;
    if (ft->mouseDelta.h > 0) {
        heading -= FIX3(5);
    } else if (ft->mouseDelta.h < 0) {
        heading += FIX3(5);
    }
    xRot = FOneCos(heading);
    zRot = FOneSin(heading);
    direction[0] = xRot;
    direction[2] = zRot;

    // Calc movement distance for each axis
    Fixed finalXSpeed = FMulDivNZ(direction[0], ToFixed(camSpeed), ToFixed(1000));
    Fixed finalYSpeed = FIX3(camSpeed);
    Fixed finalZSpeed = FMulDivNZ(direction[2], ToFixed(camSpeed), ToFixed(1000));

    // Camera rotation
    vp->fromPoint[0] = vp->atPoint[0] - FMul(direction[0], radius);
    vp->fromPoint[2] = vp->atPoint[2] - FMul(direction[2], radius);

    // Orthogonal camera movement
    if (TESTFUNC(kfuFreeCamForward, ft->held)) {
        isAttached = false;
        vp->fromPoint[0] += finalXSpeed;
        vp->fromPoint[2] += finalZSpeed;
        vp->atPoint[0] += finalXSpeed;
        vp->atPoint[2] += finalZSpeed;
    }
    if (TESTFUNC(kfuFreeCamBackward, ft->held)) {
        isAttached = false;
        vp->fromPoint[0] -= finalXSpeed;
        vp->fromPoint[2] -= finalZSpeed;
        vp->atPoint[0] -= finalXSpeed;
        vp->atPoint[2] -= finalZSpeed;
    }
    if (TESTFUNC(kfuFreeCamLeft, ft->held)) {
        isAttached = false;
        vp->fromPoint[0] += finalZSpeed;
        vp->fromPoint[2] -= finalXSpeed;
        vp->atPoint[0] += finalZSpeed;
        vp->atPoint[2] -= finalXSpeed;
    }
    if (TESTFUNC(kfuFreeCamRight, ft->held)) {
        isAttached = false;
        vp->fromPoint[0] -= finalZSpeed;
        vp->fromPoint[2] += finalXSpeed;
        vp->atPoint[0] -= finalZSpeed;
        vp->atPoint[2] += finalXSpeed;
    }

    // Handle y-axis movement differently depending on if the camera is attached to a player or not
    // Up
    if (TESTFUNC(kfuFreeCamUp, ft->held)) {
        if (isAttached) {
            // Cam is attached so don't move focal point
            vp->fromPoint[1] += finalYSpeed;
        } else {
            vp->fromPoint[1] += finalYSpeed;
            // Don't move focal point until threshold is passed
            if (vp->fromPoint[1] > yFromThreshold) {
                vp->atPoint[1] += finalYSpeed;
            }
        }
    }
    // Down
    if (TESTFUNC(kfuFreeCamDown, ft->held)) {
        if (isAttached) {
            // Cam is attached so don't move focal point
            // Cam can't go below focal point
            vp->fromPoint[1] -= finalYSpeed;
            if (vp->fromPoint[1] < vp->atPoint[1]) {
                vp->fromPoint[1] = vp->atPoint[1];
            }
        } else {
            // Don't let camera or focal point go below y=0
            // When zero is reached, save the height of the camera
            // The focal point will not move on the y-axis until the camera is at least as far away (on y-axis) as the saved threshold
            if (vp->atPoint[1] != 0)
                vp->atPoint[1] -= finalYSpeed;

            if (vp->atPoint[1] < 0) {
                yFromThreshold = vp->fromPoint[1];
                vp->atPoint[1] = 0;
            }
            vp->fromPoint[1] -= finalYSpeed;
            if (vp->fromPoint[1] < 0) {
                vp->fromPoint[1] = 0;
            }
        }
    }
}

void CFreeCam::FrameAction() {
}

void CFreeCam::ControlViewPoint() {
    auto vp = gRenderer->viewParams;

    vp->LookFrom(vp->fromPoint[0], vp->fromPoint[1], vp->fromPoint[2]);
    vp->LookAt(vp->atPoint[0], vp->atPoint[1], vp->atPoint[2]);
    vp->PointCamera();
}