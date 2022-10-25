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
#include "CViewParameters.h"
#include "CWorldShader.h"

#include <SDL2/SDL.h>
#include <iostream>
#include <string>
#include <sstream>

// These are defined all over the place right now...
#define kMarkerColor 0x00fefefe
#define kOtherMarkerColor 0x00fe0000

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
        AvaraGLInitContext();

        AvaraGLSetFOV(50);
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
                itsPart->ReplaceColor(kMarkerColor, 13421568); // yellow
                itsPart->ReplaceColor(kOtherMarkerColor, 16646144); // red
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
                    update = true;
                }
                if (event.motion.state & SDL_BUTTON(SDL_BUTTON_RIGHT)) {
                    orientation[0] += FIX((float)event.motion.yrel / 5.0);
                    orientation[1] += FIX((float)event.motion.xrel / 5.0);
                    update = true;
                }
                break;
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_LEFT:
                        orientation[1] -= FIX(5);
                        update = true;
                        break;
                    case SDLK_RIGHT:
                        orientation[1] += FIX(5);
                        update = true;
                        break;
                    case SDLK_DOWN:
                        orientation[0] -= FIX(5);
                        update = true;
                        break;
                    case SDLK_UP:
                        orientation[0] += FIX(5);
                        update = true;
                        break;
                    case SDLK_w:
                        orientation[0] = FIX(90);
                        orientation[1] = FIX(0);
                        update = true;
                        break;
                    case SDLK_s:
                        orientation[0] = FIX(-90);
                        orientation[1] = FIX(0);
                        update = true;
                        break;
                    case SDLK_d:
                        orientation[0] = FIX(0);
                        orientation[1] = FIX(90);
                        update = true;
                        break;
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
        return update;
    }

    void drawContents() {
        // Maybe put this at the CApplication level?
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        glBlendFunc(GL_ONE, GL_ZERO);
        AvaraGLViewport(fb_size_x, fb_size_y);
        itsView->SetViewRect(fb_size_x, fb_size_y, fb_size_x / 2, fb_size_y / 2);
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

};

int main(int argc, char *argv[]) {


    // Init Avara stuff.
    InitMatrix();

    // The BSPViewer application itself.
    BSPViewer *app = new BSPViewer(argc > 1 ? atoi(argv[1]) : 215);


    bool main_loop_active = true;
    SDL_Event event;

    while (main_loop_active) {

        //if (!app->visible()) {
        //    continue;
        //}
        if (SDL_WaitEventTimeout(&event, 500)) {
            if (event.type == SDL_QUIT) {
                main_loop_active = false;
            }
            app->HandleEvent(event);
        }

        glClearColor(.3, .5, .3, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        // Update everything and draw
        app->drawContents();
        SDL_GL_SwapWindow(app->window);
    }
    app->Done();

    return 0;
}
