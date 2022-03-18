#include "CGUIWidget.h"

class CGUIButton : public CGUIWidget {
public:
    CGUIButton(NVGcolor color, 
               char *str, 
               char *font,
               int action, 
               Rect rect);
    void attach(CGUI *gui);
    void update(CGUI *gui);
    void activated() {}
    //void Render(NVGcontext *ctx);
    ~CGUIButton();

protected:
    NVGcolor _color;
    char *_str;
    int _action;
    float rot = 0;
    CSmartBox *_part;
};
