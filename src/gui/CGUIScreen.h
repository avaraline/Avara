
#include "CBSPWorld.h"
#include "CGUIWidget.h"

class CGUI;
class CGUIScreen {
public: 
    CGUIScreen(int screen, CBSPWorld *world, CGUI *gui);
    virtual ~CGUIScreen(){};
    virtual bool handleSDLEvent(SDL_Event &event){ return false; };
    virtual void Render(NVGcontext *ctx) {
        for(auto i = _children.begin(); i != _children.end(); i++) {
            (*i)->render(ctx);
        }
    };
    virtual void Update() {
        for(auto i = _children.begin(); i != _children.end(); i++) {
            (*i)->update(_gui);
        }
    }
protected:
    int _screen;
    CBSPWorld *_world;
    CGUI *_gui;
    std::vector<CGUIWidget*> _children;
};