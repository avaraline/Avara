#include "CGUIScreen.h"
#include "CGUIWidget.h"
#include "CGUIButton.h"

CGUIScreen::CGUIScreen(int screen, CBSPWorld *world, CGUI *gui) {
    _screen = screen;
    _world = world;
    _gui = gui;

    switch(screen){
        case MainMenu:
        {
            Rect r;
            r.top = 50;
            r.bottom = 100;
            r.left = 100;
            r.right = 300;
            CGUIButton *b = new CGUIButton(
                nvgRGB(25, 255, 255),
                "CLICK???",
                "mono",
                0,
                r
            );
            _children.push_back(b);
            b->attach(_gui);
            break;
        }
        default:
            break;
    }
}