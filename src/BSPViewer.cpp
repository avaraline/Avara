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

#include <SDL.h>

// These are defined all over the place right now...
#define kMarkerColor 0x00fefefe
#define kOtherMarkerColor 0x00fe0000

class BSPViewer : public CApplication {
public:
    CBSPPart *itsPart;
    CBSPWorld *itsWorld;
    CViewParameters *itsView;

    Vector location, orientation;

    BSPViewer() : CApplication("BSP Viewer", 800, 600) {
        itsWorld = new CBSPWorld;
        itsWorld->IBSPWorld(1);

        itsPart = new CBSPPart;
        itsPart->IBSPPart(215);
        itsPart->ReplaceColor(kMarkerColor, 13421568); // yellow
        itsPart->ReplaceColor(kOtherMarkerColor, 13421568); // yellow
        itsWorld->AddPart(itsPart);

        itsView = new CViewParameters;
        itsView->IViewParameters();
        itsView->SetLight(1, FIX(30), FIX(-20), FIX3(200), kLightGlobalCoordinates);
        itsView->yonBound = FIX(10000);

        location[0] = FIX(0);
        location[0] = FIX(0);
        location[2] = FIX(25);

        orientation[0] = FIX(0);
        orientation[1] = FIX(0);
        orientation[2] = FIX(0);
    }

    bool HandleEvent(SDL_Event &event) {
        switch (event.type) {
            case SDL_MOUSEMOTION:
                if (event.motion.state & SDL_BUTTON(SDL_BUTTON_LEFT)) {
                    location[2] += FIX((float)event.motion.yrel / 5.0);
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
                }
                break;
        }
        return false;
    }

    void DrawContents() {
        // Maybe put this at the CApplication level?
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        glBlendFunc(GL_ONE, GL_ZERO);

        itsView->SetViewRect(mFBSize.x, mFBSize.y, mFBSize.x / 2, mFBSize.y / 2);

        itsPart->Reset();
        itsPart->RotateZ(orientation[2]);
        itsPart->RotateY(orientation[1]);
        itsPart->RotateX(orientation[0]);
        TranslatePart(itsPart, location[0], location[1], location[2]);
        itsPart->MoveDone();
        itsWorld->Render(itsView);
    }
};

int main(int argc, char *argv[]) {
    // Init SDL and nanogui.
    nanogui::init();

    // Init Avara stuff.
    InitMatrix();

    // The BSPViewer application itself.
    BSPViewer *app = new BSPViewer();

    // Wait at most 10ms for any event.
    nanogui::mainloop(10);

    // Shut it down!!
    nanogui::shutdown();

    return 0;
}
