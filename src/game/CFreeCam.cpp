#include "CFreeCam.h"

#include "CViewParameters.h"
#include "AbstractRenderer.h"
#include "CSmartPart.h"
#include "FastMat.h"

#include "Debug.h"

CFreeCam::CFreeCam(CAbstractPlayer *thePlayer) {
    itsPlayer = thePlayer;
    itsGame = thePlayer->itsGame;
    itsSoundLink = gHub->GetSoundLink();

    camSpeed = 350;
    radius = FIX3(25000);
    pitch = FIX3(125); // 45 degrees
    heading = 0;

    isAttached = false;
    action = camInactive;
    isActive = kIsActive;
    partCount = 0;
    glow = 0;
}

void CFreeCam::ToggleState(Boolean state) {
    CAbstractPlayer *spectatePlayer = itsGame->GetSpectatePlayer();

    if (state) {
        if (action == camInactive) {
            itsGame->AddActor(this);
            itsPlayer->freeCamIdent = ident;
            action = camAnimating;
        }
        if (spectatePlayer != NULL) {
            isAttached = true;
        }
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

    // This sets up the ability for the camera to follow a player
    if (isAttached && spectatePlayer != NULL) {
        vp->LookAtPart(spectatePlayer->viewPortPart);
    }

    // Mouse movements adjust polar coordinates
    heading += FIX3(ft->mouseDelta.h * .2);
    pitch -= FIX3(ft->mouseDelta.v * .2);
    if (pitch > FIX3(250)) pitch = FIX3(250);
    if (pitch < FIX3(10)) pitch = FIX3(10);

    // Convert from polar to cartesian
    // Find normalized camera position
    Fixed xRot=0, yRot=0, zRot=0;
    xRot = FMul(FOneCos(heading), FOneSin(pitch));
    yRot = FOneCos(pitch);
    zRot = FMul(FOneSin(heading), FOneSin(pitch));
    direction[0] = xRot;
    direction[1] = yRot;
    direction[2] = zRot;

    // Calc potential velocity for each axis
    Fixed finalXSpeed = FMulDivNZ(direction[0], ToFixed(camSpeed), ToFixed(1000));
    Fixed finalYSpeed = FIX3(camSpeed);
    Fixed finalZSpeed = FMulDivNZ(direction[2], ToFixed(camSpeed), ToFixed(1000));

    // Set actual camera position
    vp->fromPoint[0] = vp->atPoint[0] - FMul(direction[0], radius);
    vp->fromPoint[1] = vp->atPoint[1] + FMul(direction[1], radius);
    vp->fromPoint[2] = vp->atPoint[2] - FMul(direction[2], radius);

    // Zoom out
    if (TESTFUNC(kfuZoomOut, ft->held)) {
        radius += FIX3(camSpeed);
        if (radius > FIX3(50000)) {
            radius = FIX3(50000);
        }
    }
    // Zoom In
    if (TESTFUNC(kfuZoomIn, ft->held)) {
        radius -= FIX3(camSpeed);
        if (radius < FIX3(10000)) {
            radius = FIX3(10000);
        }
    }

    // Orthogonal camera movement
    if (TESTFUNC(kfuForward, ft->held)) {
        isAttached = false;
        vp->fromPoint[0] += finalXSpeed;
        vp->fromPoint[2] += finalZSpeed;
        vp->atPoint[0] += finalXSpeed;
        vp->atPoint[2] += finalZSpeed;
    }
    if (TESTFUNC(kfuReverse, ft->held)) {
        isAttached = false;
        vp->fromPoint[0] -= finalXSpeed;
        vp->fromPoint[2] -= finalZSpeed;
        vp->atPoint[0] -= finalXSpeed;
        vp->atPoint[2] -= finalZSpeed;
    }
    if (TESTFUNC(kfuLeft, ft->held)) {
        isAttached = false;
        vp->fromPoint[0] += finalZSpeed;
        vp->fromPoint[2] -= finalXSpeed;
        vp->atPoint[0] += finalZSpeed;
        vp->atPoint[2] -= finalXSpeed;
    }
    if (TESTFUNC(kfuRight, ft->held)) {
        isAttached = false;
        vp->fromPoint[0] -= finalZSpeed;
        vp->fromPoint[2] += finalXSpeed;
        vp->atPoint[0] -= finalZSpeed;
        vp->atPoint[2] += finalXSpeed;
    }

    // Up
    if (TESTFUNC(kfuFreeCamUp, ft->held)) {
        isAttached = false;
        vp->atPoint[1] += finalYSpeed;
        vp->fromPoint[1] += finalYSpeed;
    }
    // Down
    if (TESTFUNC(kfuFreeCamDown, ft->held)) {
        // Leave camera attached if already at y=0
        if (vp->atPoint[1] > 0) {
            isAttached = false;
            // Focal point should not go below 0
            // If focal point does go below zero, move camera by the same amount to keep it at a fixed distance
            if (vp->atPoint[1] - finalYSpeed < 0) {
                vp->fromPoint[1] -= vp->atPoint[1];
                vp->atPoint[1] = 0;
            } else {
                vp->atPoint[1] -= finalYSpeed;
                vp->fromPoint[1] -= finalYSpeed;
            }
        }
    }
}

void CFreeCam::FrameAction() {
}

void CFreeCam::ControlSoundPoint(CViewParameters *vp) {
    Fixed theRight[] = {FIX(-1), 0, 0};

    // This hard-coded data matches the vector data used for ControlSoundPoint() in AbstractPlayer.cpp
    // The matrix data from viewParams differs from the hard-coded data in a way that makes the sound not play in the correct channels
    theRight[0] = FIX3(707);
    theRight[1] = 0;
    theRight[2] = FIX3(707);

    gHub->SetMixerLink(itsSoundLink);
    gHub->UpdateRightVector(theRight);
}

void CFreeCam::ControlViewPoint() {
    auto vp = gRenderer->viewParams;

    vp->LookFrom(vp->fromPoint[0], vp->fromPoint[1], vp->fromPoint[2]);
    vp->LookAt(vp->atPoint[0], vp->atPoint[1], vp->atPoint[2]);
    vp->PointCamera();

    UpdateSoundLink(itsSoundLink, vp->fromPoint, speed, itsGame->soundTime);
    ControlSoundPoint(vp);
}