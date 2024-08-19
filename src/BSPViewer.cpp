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
#include "ARGBColor.h"
#include "csscolorparser.hpp"
#include "Resource.h"
#include <SDL2/SDL.h>
#include <iostream>
#include <string>
#include <sstream>

#define STB_IMAGE_WRITE_IMPLEMENTATION
extern "C" {
    #include "stb_image_write.h"
}

class BSPViewer : public CApplication {
public:
    CBSPPart *itsPart;
    CBSPWorldImpl *itsWorld;
    CViewParameters *itsView;
    CWorldShader *worldShader;

    Vector location, orientation;
    int current_id;
    bool update = true;
    bool skyToggle = true;
    bool helpToggle = true;
    ARGBColor marker1 = (*ColorManager::getMarkerColor(0)).WithA(0xff);
    ARGBColor marker2 = (*ColorManager::getMarkerColor(1)).WithA(0xff);
    ARGBColor marker3 = (*ColorManager::getMarkerColor(2)).WithA(0xff);
    ARGBColor marker4 = (*ColorManager::getMarkerColor(3)).WithA(0xff);
    std::string help_text;

    BSPViewer(int id) : CApplication("BSP Viewer") {
        AvaraGLInitContext();
        glClearColor(1, 0, 1, 1);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        glBlendFunc(GL_ONE, GL_ZERO);

        std::stringstream help;
        help << "AWSD: rotate model to cardinal directions\n";
        help << "Left click: move model on z axis\n";
        help << "Right click: rotate model\n";
        help << "R/T: previous/next shape\n";
        help << "U: sky toggle\n";
        help << "0: capture image\n";
        help << "F1: show/hide this text";
        help_text = help.str();

        AvaraGLSetFOV(50);

        marker1 = ColorManager::getTeamColor(3).value();
        marker2 = ColorManager::getTeamColor(2).value(); 
        current_id = id;
        itsWorld = new CBSPWorldImpl(1);
        newPart(current_id);

        itsView = new CViewParameters;

        itsView->yonBound = FIX(100);
        itsView->dirtyLook = true;

        itsView->LookFrom(0, 0, FIX(1.5));
        itsView->LookAtPart(itsPart);
        itsView->PointCamera();

        worldShader = new CWorldShader;
        worldShader->IWorldShader();
        worldShader->skyShadeCount = 12;

        AvaraGLSetLight(0, 1.0, 85, 165, 0xffffffff);
        AvaraGLSetLight(1, 0, 45, 0, 0xffffffff);
        AvaraGLSetAmbient(0.31, 0xffffffff);

        skyToggle = true;
        helpToggle = true;


        location[0] = FIX(0);
        location[1] = FIX(0);
        location[2] = FIX(0);

        orientation[0] = FIX(0);
        orientation[1] = FIX(0);
        orientation[2] = FIX(0);
    }

    void updateClearColor(CSSColorParser::Color c) {
        glClearColor(c.r, c.g, c.b, c.a);
    }

    void helpText() {
        if (!helpToggle) return;

        std::stringstream info;
        info << help_text << "\n";
        info << "Model: " << current_id << "\n";
        info << "Translation: " << ToFloat(location[0]) << "," << ToFloat(location[1]) << "," << ToFloat(location[2]) << "\n";
        info << "Rotation: " << ToFloat(orientation[0]) << "," << ToFloat(orientation[1]) << "," << ToFloat(orientation[2]);
        nvgBeginFrame(nvg_context, fb_size_x, fb_size_y, 1.0);
        nvgFontSize(nvg_context, 25);
        nvgFontFace(nvg_context, "mono");
        nvgTextBox(nvg_context, 10, 30, 500, info.str().c_str(), NULL);
        nvgEndFrame(nvg_context);
    }
    bool newPart(int id) {
            if (itsWorld->GetPartCount() > 0) {
                itsWorld->RemovePart(itsPart);
            }
            itsPart = CBSPPart::Create(id);
            if (itsPart->polyCount > 0) {
                itsPart->ReplaceColor(
                    *ColorManager::getMarkerColor(0),
                    marker1
                );
                itsPart->ReplaceColor(
                    *ColorManager::getMarkerColor(1),
                    marker2
                );
                itsPart->ReplaceColor(
                    *ColorManager::getMarkerColor(2),
                    marker3
                );
                itsPart->ReplaceColor(
                    *ColorManager::getMarkerColor(3),
                    marker4
                );
                AvaraGLUpdateData(itsPart);
                itsWorld->AddPart(itsPart);
                SDL_Log("Loaded BSP %d", id);
                return true;
            }
            else return false;

    }


    bool HandleEvent(SDL_Event &event) {
        Vector prevLocation;
        memcpy(prevLocation, location, sizeof(Vector));
        Vector prevOrientation;
        memcpy(prevOrientation, orientation, sizeof(Vector));
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
                            if (current_id > INT32_MAX) {
                                current_id = 1;
                            }
                        } while (newPart(current_id) != true);
                        return true;
                    case SDLK_t:
                        do{
                            current_id--;
                            if (current_id < 1) {
                                current_id = INT32_MAX;
                            }
                        } while(newPart(current_id) != true);
                        return true;
                    case SDLK_y:
                        newPart(current_id);
                        return true;
                    case SDLK_u:
                        skyToggle = !skyToggle;
                        return true;
                    case SDLK_F1:
                        helpToggle = !helpToggle;
                        return true;
                    case SDLK_0:
                        std::stringstream ss;
                        ss << current_id << '-' << SDL_GetTicks() << ".png";
                        capture(ss.str().c_str());
                        return true;
                }
                break;
        }
        
        if (update) {
            if (prevLocation[0] != location[0] ||
                prevLocation[1] != location[1] ||
                prevLocation[2] != location[2] ||
                prevOrientation[0] != orientation[0] ||
                prevOrientation[1] != orientation[1] ||
                prevOrientation[2] != orientation[2])
            SDL_Log("-s %d --location %f %f %f --orientation %f %f %f", current_id, ToFloat(location[0]), ToFloat(location[1]), ToFloat(location[2]), ToFloat(orientation[0]), ToFloat(orientation[1]), ToFloat(orientation[2]));
        }
        return update;
    }

    int capture(const char* fn) {
        drawContents();
        //SDL_GL_SwapWindow(app->window);
        int w = fb_size_x;
        int h = fb_size_y;
        int comp = 3;
        std::vector<GLchar> pic(w * h * comp);

        glReadPixels(0, 0, w, h, comp == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, pic.data());

        for (int line = 0; line != h / 2; ++line) {
            std::swap_ranges(
                pic.begin() + comp * w * line,
                pic.begin() + comp * w * (line + 1),
                pic.begin() + comp * w * (h - line - 1));
        }
        SDL_Log("Writing %s", fn);
        return stbi_write_png(fn, w, h, comp, pic.data(), w * comp);

    }

    void drawContents() {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        AvaraGLViewport(fb_size_x, fb_size_y);
        itsView->SetViewRect(fb_size_x, fb_size_y, fb_size_x / 2, fb_size_y / 2);
        // Maybe put this at the CApplication level?
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        glBlendFunc(GL_ONE, GL_ZERO);
        itsView->viewPixelRatio = FIX(4.0/3.0);
        itsView->CalculateViewPyramidCorners();
        itsView->PointCamera();

        itsPart->Reset();
        itsPart->RotateZ(orientation[2]);
        itsPart->RotateY(orientation[1]);
        itsPart->RotateX(orientation[0]);
        TranslatePart(itsPart, location[0], location[1], location[2]);
        itsPart->MoveDone();

        if (skyToggle)
        worldShader->ShadeWorld(itsView);

        itsWorld->Render(itsView);

        helpText();
    }

};

int main(int argc, char *argv[]) {

    // Init Avara stuff.
    InitMatrix();

    // The BSPViewer application itself.
    BSPViewer *app = new BSPViewer(215);
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-s" || arg == "--shape") {
            u_int16_t id = atoi(argv[++i]);
            app->current_id = id;
            app->newPart(id);
        }

        if (arg == "--orientation") {
            app->newPart(app->current_id);
            app->orientation[0] = ToFixed(atof(argv[++i]));
            app->orientation[1] = ToFixed(atof(argv[++i]));
            app->orientation[2] = ToFixed(atof(argv[++i]));
        }
        if (arg == "--location") {
            app->newPart(app->current_id);
            app->location[0] = ToFixed(atof(argv[++i]));
            app->location[1] = ToFixed(atof(argv[++i]));
            app->location[2] = ToFixed(atof(argv[++i]));
        }
        else if (arg == "--marker-color1") {
            std::string s = std::string(argv[++i]);
            std::optional<ARGBColor> c = ARGBColor::Parse(s);
            if (c.has_value()) {
                app->marker1 = c.value();
                app->newPart(app->current_id);
            } 
        }
        else if (arg == "--marker-color2") {
            std::string s = std::string(argv[++i]);
            std::optional<ARGBColor> c = ARGBColor::Parse(s);
            if (c.has_value()) {
                app->marker2 = c.value();
                app->newPart(app->current_id);
            } 
        }
        else if (arg == "--marker-color3") {
            std::string s = std::string(argv[++i]);
            std::optional<ARGBColor> c = ARGBColor::Parse(s);
            if (c.has_value()) {
                app->marker3 = c.value();
                app->newPart(app->current_id);
            } 
        }
        else if (arg == "--marker-color4") {
            std::string s = std::string(argv[++i]);
            std::optional<ARGBColor> c = ARGBColor::Parse(s);
            if (c.has_value()) {
                app->marker4 = c.value();
                app->newPart(app->current_id);
            } 
        }
        else if (arg == "--background-color") {
            
            app->skyToggle = false;
            std::string s = std::string(argv[++i]);
            std::optional<CSSColorParser::Color> c = CSSColorParser::parse(s);
            if (c.has_value()) app->updateClearColor(c.value());
        }

        else if (arg == "--output-file") {
            exit(app->capture(argv[++i]));
        }
        else if (arg == "--ambient") {
            AvaraGLSetAmbient(atof(argv[++i]), 0xffffffff);
        }
        else if (arg == "--hull-emotes") {

            int hulls[3] = {215, 216, 217};
            std::string hull_names[] = {"Light", "Medium", "Heavy"};
            app->orientation[0] = ToFixed(14);
            app->orientation[1] = ToFixed(25.2);
            app->location[2] = ToFixed(-0.44007);
            
            app->skyToggle = false;
            std::stringstream ss;
            int error = 0;
            for (size_t i = 0; i < 3; i++) {
                for (size_t j = 0; j < kMaxTeamColors; j++) {
                    
                    app->marker1 = ColorManager::getTeamColor(j).value();
                    app->newPart(hulls[i]);
                    app->drawContents();
                    ss.str("");
                    ss << hull_names[i] << "-" << ColorManager::getTeamColorName(j).value() << ".png";
                    error += app->capture(ss.str().c_str());
                }
                app->marker1 = ColorManager::getTeamColor(7).value();
                app->newPart(hulls[i]);
                app->drawContents();
                ss.str("");
                ss << hull_names[i] << "-Black.png";
                error += app->capture(ss.str().c_str());

                app->marker1 = ColorManager::getTeamColor(8).value();
                app->newPart(hulls[i]);
                app->drawContents();
                ss.str("");
                ss << hull_names[i] << "-White.png";
                error += app->capture(ss.str().c_str());
            }
            SDL_Quit();
            exit(error);
        }
    }



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

        //glClearColor(1, 0, 1, 1);
        // Update everything and draw
        app->drawContents();
        app->helpText();
        SDL_GL_SwapWindow(app->window);
    }
    app->Done();

    return 0;
}
