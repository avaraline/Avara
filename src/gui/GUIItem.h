//
//  GUIItem.h
//  Avara
//
//  Created by Andy Halstead on 2/24/26.
//

#ifndef GUIItem_h
#define GUIItem_h
#include "nanovg.h"
#include <glm/glm.hpp>
#include <functional>

enum GUIItemType { Pane, JustText, JustLine, JustRect, Button, TextInput, Checkbox, Dropdown };
typedef unsigned short Dim;
struct NVGrect {
    union {
        Dim rect[4];
    };
    struct {
        Dim x, y, w, h;
    };
};
typedef struct NVGrect NVGrect;

static inline NVGrect nvgRectData(Dim x, Dim y, Dim w, Dim h) {
    NVGrect r;
    r.x = x;
    r.y = y;
    r.w = w;
    r.h = h;
    return r;
}

static inline bool isInRect(Dim x, Dim y, NVGrect r) {
    return (x > r.x  && x < r.x + r.w &&
            y > r.y && y < r.y + r.h);
};

static inline NVGcolor invertColor(NVGcolor c) {
    NVGcolor inverse;
    inverse.r = std::max<unsigned char>(255 - c.r, 0);
    inverse.g = std::max<unsigned char>(255 - c.g, 0);
    inverse.b = std::max<unsigned char>(255 - c.b, 0);
    inverse.a = c.a;
    return inverse;
}

class GUIItem {
public:
    GUIItem() {};
    GUIItemType itemType;
    NVGrect rect;
    NVGcolor color;
    NVGcolor textColor;
    Dim textSize = 0;
    Dim textAlign = 0;
    Dim outline;
    short ord_x = -1;
    short ord_y = -1;
    std::string text;
    bool interactable;
    bool focus;
    std::function<void()> action;
    
    
    void Draw(NVGcontext *ctx);
    void NVGRect(NVGcontext *ctx);
    void NVGRectPath(NVGcontext *ctx);
    void NVGBtn(NVGcontext *ctx);
    void NVGPane(NVGcontext *ctx);
    void NVGText(NVGcontext *ctx);
};

#endif /* GUIItem_h */
