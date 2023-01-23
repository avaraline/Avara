/*
    Copyright ©1995-1996, Juri Munkki
    All rights reserved.

    File: BSPViewer.c
    Created: Thursday, July 27, 1995, 23:10
    Modified: Tuesday, February 20, 1996, 08:02
*/

#include "AvaraGL.h"
#include "CApplication.h"
#include "CBSPPart.h"
#include "CBSPWorld.h"
#include "ColorManager.h"
#include "CViewParameters.h"
#include "CWorldShader.h"

#include <SDL.h>
#include <iostream>
#include <string>
#include <sstream>

class BSPViewer : public CApplication {
public:
    CBSPPart *itsPart;
    CBSPWorldImpl *itsWorld;
    CViewParameters *itsView;
    CWorldShader *worldShader;

    Vector location, orientation;
    int current_id;
    bool update = true;

    BSPViewer(int id) : CApplication("BSP Viewer") {
        current_id = id;
        itsWorld = new CBSPWorldImpl;
        itsWorld->IBSPWorld(1);
        newPart(current_id);

        itsView = new CViewParameters;
        itsView->IViewParameters();

        itsView->yonBound = FIX(100);
        itsView->dirtyLook = true;

        itsView->LookFrom(0, 0, FIX(1.5));
        itsView->LookAtPart(itsPart);
        itsView->PointCamera();

        worldShader = new CWorldShader;
        worldShader->IWorldShader();
        worldShader->skyShadeCount = 12;


        location[0] = FIX(0);
        location[1] = FIX(0);
        location[2] = FIX(0);

        orientation[0] = FIX(0);
        orientation[1] = FIX(0);
        orientation[2] = FIX(0);
    }


    bool newPart(int id) {
            if (itsWorld->GetPartCount() > 0) {
                itsWorld->RemovePart(itsPart);
            }
            itsPart = new CBSPPart;
            itsPart->IBSPPart(id);
            if (itsPart->polyCount > 0) {
                itsPart->ReplaceColor(*ColorManager::getMarkerColor(0), 0xffcccc00); // yellow
                itsPart->ReplaceColor(*ColorManager::getMarkerColor(1), 0xfffe0000); // red
                AvaraGLUpdateData(itsPart);
                itsWorld->AddPart(itsPart);
                SDL_Log("Loaded BSP %d", id);
                return true;
            }
            else return false;

    }


    bool HandleEvent(SDL_Event &event) {
        switch (event.type) {
            case SDL_MOUSEMOTION:
                if (event.motion.state & SDL_BUTTON(SDL_BUTTON_LEFT)) {
                    location[2] += FIX((float)event.motion.yrel / 5.0);
                    return true;
                }
                if (event.motion.state & SDL_BUTTON(SDL_BUTTON_RIGHT)) {
                    orientation[0] += FIX((float)event.motion.yrel / 5.0);
                    orientation[1] += FIX((float)event.motion.xrel / 5.0);
                    return true;
                }
                break;
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_LEFT:
                        orientation[1] -= FIX(5);
                        return true;
                    case SDLK_RIGHT:
                        orientation[1] += FIX(5);
                        return true;
                    case SDLK_DOWN:
                        orientation[0] -= FIX(5);
                        return true;
                    case SDLK_UP:
                        orientation[0] += FIX(5);
                        return true;
                    case SDLK_w:
                        orientation[0] = FIX(90);
                        orientation[1] = FIX(0);
                        return true;
                    case SDLK_s:
                        orientation[0] = FIX(-90);
                        orientation[1] = FIX(0);
                        return true;
                    case SDLK_d:
                        orientation[0] = FIX(0);
                        orientation[1] = FIX(90);
                        return true;
                    case SDLK_a:
                        orientation[0] = FIX(0);
                        orientation[1] = FIX(-90);
                        return true;
                    case SDLK_r:
                        do {
                            current_id++;
                            if (current_id > 1500) {
                                current_id = 1;
                            }
                        } while (newPart(current_id) != true);
                        return true;
                    case SDLK_t:
                        do{
                            current_id--;
                            if (current_id < 1) {
                                current_id = 1500;
                            }
                        } while(newPart(current_id) != true);
                        return true;
                    case SDLK_y:
                        newPart(current_id);
                        return true;
                }
                break;
        }
        return false;
    }

    void drawContents() {
        // Maybe put this at the CApplication level?
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        glBlendFunc(GL_ONE, GL_ZERO);

        AvaraGLViewport(mFBSize.x, mFBSize.y);
        itsView->SetViewRect(mFBSize.x, mFBSize.y, mFBSize.x / 2, mFBSize.y / 2);
        itsView->viewPixelRatio = FIX(4.0/3.0);
        itsView->CalculateViewPyramidCorners();
        itsView->PointCamera();

        itsPart->Reset();
        itsPart->RotateZ(orientation[2]);
        itsPart->RotateY(orientation[1]);
        itsPart->RotateX(orientation[0]);
        TranslatePart(itsPart, location[0], location[1], location[2]);
        itsPart->MoveDone();

        worldShader->ShadeWorld(itsView);

        itsWorld->Render(itsView);
    }

    void idle() {
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        drawContents();
        SDL_GL_SwapWindow(mSDLWindow);
    }
};

int main(int argc, char *argv[]) {


    // Init SDL and nanogui.
    nanogui::init();

    // Init Avara stuff.
    InitMatrix();

    // The BSPViewer application itself.
    if (argc > 1)
        BSPViewer *app = new BSPViewer(atoi(argv[1]));
    else
        BSPViewer *app = new BSPViewer(215);

    // Wait at most 10ms for any event.
    nanogui::mainloop(10);

    // Shut it down!!
    nanogui::shutdown();

    return 0;
}
