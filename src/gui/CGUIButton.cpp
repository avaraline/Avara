#include "CGUIButton.h"
#include "CGUI.h"

#include "CAvaraGame.h"
#include "CAvaraApp.h"
#include "CNetManager.h"

CGUIButton::CGUIButton(NVGcolor color, 
               char *str, 
               char *font,
               int action, 
               Rect r){
        _color = color;
        _str = str;
        _action = action;
        _rect = r;
        Point mid = midpoint(r);
        _nvgactions = text(_str, font, color, mid);
};

CGUIButton::~CGUIButton() {

}

void CGUIButton::attach(CGUI *gui) {
    Point s_topleft = pt(_rect.left, _rect.top);
    Point s_bottomright = pt(_rect.right, _rect.bottom);
    glm::vec3 ws_topleft = gui->screenToWorld(&s_topleft);
    glm::vec3 ws_bottomright = gui->screenToWorld(&s_bottomright);
    Vector dims;
    dims[0] = FIX(std::abs(ws_bottomright.x - ws_topleft.x) / 2.0);
    dims[1] = FIX(std::abs(ws_bottomright.y - ws_topleft.y) / 2.0);
    dims[2] = FIX3(1);
    _part = new CSmartBox;
    _part->ISmartBox(kBlockBSP,
                     dims, 
                     RGBAToLong(_color), 
                     RGBAToLong(_color), 
                     0, 0);

    _part->ReplaceColor(0x00fefefe, 0x00ffffff);
    Point mid = midpoint(_rect);
    glm::vec3 worldpos = gui->screenToWorld(&mid);
    _partLoc[0] = ToFixed(worldpos.x);
    _partLoc[1] = ToFixed(worldpos.y);
    _partLoc[2] = 0;
    _part->UpdateOpenGLData();
    gui->itsWorld->AddPart(_part);
}

void CGUIButton::update(CGUI *gui) {
        if(_rect.left < gui->cursor_x
        && gui->cursor_x < _rect.right
        && _rect.top < gui->cursor_y 
        && gui->cursor_y < _rect.bottom) {
            rot += ((float)gui->dt / 1000.0) / 2.0f;
            if (gui->cursor_buttons & SDL_BUTTON_LMASK) {
                _nvgactions = quick_text("CLICKED!", midpoint(_rect));
                gui->itsGame->itsApp->GetNet()->SendLoadLevel("single-player", "training-grounds.alf");
            }
        }
        else {
            rot = 0;
        }

        
        _part->Reset();
        InitialRotatePartY(_part, FIX(rot));
        TranslatePart(_part, _partLoc[0], _partLoc[1], _partLoc[2]);
        _part->MoveDone();
    }