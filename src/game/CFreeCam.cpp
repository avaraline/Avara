#include "CFreeCam.h"

#include "CViewParameters.h"
#include "AbstractRenderer.h"
#include "CSmartPart.h"
#include "FastMat.h"

#include "Debug.h"

CFreeCam::CFreeCam(CAbstractPlayer *thePlayer) {
    itsPlayer = thePlayer;
    camSpeed = 350;
    radius = FIX3(25000);
}

void CFreeCam::ToggleState(Boolean state) {
    CAbstractPlayer *spectatePlayer = itsPlayer->itsGame->GetSpectatePlayer();

    if (state && spectatePlayer != NULL) {
        isAttached = true;
    }
}

void CFreeCam::SetAttached(Boolean attach) {
    isAttached = attach;
}

Boolean CFreeCam::IsAttached() {
    return isAttached && itsPlayer->itsGame->GetSpectatePlayer() != NULL;
}

void CFreeCam::ViewControl(FunctionTable *ft) {    
    auto vp = gRenderer->viewParams;
    Vector direction;
    CAbstractPlayer *spectatePlayer = itsPlayer->itsGame->GetSpectatePlayer();

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

    itsPlayer->WriteDBG(0, ToFloat(vp->atPoint[0])); // Cam target X
    itsPlayer->WriteDBG(1, ToFloat(vp->atPoint[1])); // Cam target Y
    itsPlayer->WriteDBG(2, ToFloat(vp->atPoint[2])); // Cam target Z
    itsPlayer->WriteDBG(3, ToFloat(vp->fromPoint[0])); // Cam position X
    itsPlayer->WriteDBG(4, ToFloat(vp->fromPoint[1])); // Cam position Y
    itsPlayer->WriteDBG(5, ToFloat(vp->fromPoint[2])); // Cam position Z
    itsPlayer->WriteDBG(6, ToFloat(direction[0])); // Cam facing X (Normalized)
    itsPlayer->WriteDBG(7, ToFloat(direction[1])); // Cam facing Y (Normalized)
    itsPlayer->WriteDBG(8, ToFloat(direction[2])); // Cam facing Z (Normalized)
    itsPlayer->WriteDBG(9, ToFloat(len)); // Dist between Cam and target
    itsPlayer->WriteDBG(10, ToFloat(heading)); // Direction in radians?
    itsPlayer->WriteDBG(11, ToFloat(pitch)); // Vertical angle in radians?
    itsPlayer->WriteDBG(12, (float)ft->mouseDelta.h); // Horizontal Mouse direction integer mouseRight: positive, mouseLeft: negative
    itsPlayer->WriteDBG(13, (float)ft->mouseDelta.v); // Vertical Mouse direction integer   mouseUp: negative,    mouseDown: positive
    itsPlayer->WriteDBG(14, ToFloat(xRot)); // horizontal camera angle
    itsPlayer->WriteDBG(15, ToFloat(zRot)); // vertical camera angle
    itsPlayer->WriteDBG(16, isAttached); // is the camera attached to a hector
    if (spectatePlayer != NULL) {
        itsPlayer->WriteDBG(17, spectatePlayer->GetPlayerPosition()); // which hector are we looking at
    } else {
        itsPlayer->WriteDBG(17, 0); // which hector are we looking at
    }


    // Calc movement distance for each axis
    Fixed finalXSpeed = FMulDivNZ(direction[0], ToFixed(camSpeed), ToFixed(1000));
    Fixed finalYSpeed = FIX3(camSpeed);
    Fixed finalZSpeed = FMulDivNZ(direction[2], ToFixed(camSpeed), ToFixed(1000));

    // Camera rotation
    vp->fromPoint[0] = vp->atPoint[0] - FMul(direction[0], radius);
    vp->fromPoint[2] = vp->atPoint[2] - FMul(direction[2], radius);

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
    if (TESTFUNC(kfuLoadMissile, ft->held)) {
        vp->fromPoint[1] += finalYSpeed;
        vp->atPoint[1] += finalYSpeed;
    }
    if (TESTFUNC(kfuFireWeapon, ft->held)) {
        vp->fromPoint[1] -= finalYSpeed;
        vp->atPoint[1] -= finalYSpeed;
    }

    ControlViewPoint();
}

void CFreeCam::FrameAction() {

}

void CFreeCam::ControlViewPoint() {
    auto vp = gRenderer->viewParams;

    vp->LookFrom(vp->fromPoint[0], vp->fromPoint[1], vp->fromPoint[2]);
    vp->LookAt(vp->atPoint[0], vp->atPoint[1], vp->atPoint[2]);
    vp->PointCamera();
}