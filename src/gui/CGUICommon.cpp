#include "CGUICommon.h"
#include "nanovg.h"

NVGcolor __white = nvgRGBA(255,255,255,255);
NVGcolor __red = nvgRGBA(255,0,0,0);


NVGCall flat_map(NVGCall a, NVGCall b) {
    return [a, b] (NVGcontext *c) {
        a(c);
        b(c);
    };
};

const Point pt(short x, short y) {
    Point p;
    p.h = x;
    p.v = y;
    return p;
}

const Point midpoint(Rect &r) {
    return pt((short)(r.left + ((r.right - r.left) / 2)), 
              (short)(r.top + ((r.bottom - r.top) / 2)));
};

const NVGCall text(const char *s, const char *font, NVGcolor color, Point pos) {
    return [s, font, color, pos](NVGcontext *ctx) {
        nvgFontFace(ctx, font);
        nvgFillColor(ctx, color);
        nvgTextAlign(ctx, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
        nvgFontSize(ctx, 25);
        nvgText(ctx, pos.h, pos.v, s, NULL);
    };
};

const NVGCall rect(Rect &r, NVGcolor &c) {
    return [r, c](NVGcontext *ctx) {
        nvgBeginPath(ctx);
        short w = std::abs(r.right - r.left);
        short h = std::abs(r.top - r.bottom);
        nvgRect(ctx, r.left , r.top, w, h);
        nvgStrokeColor(ctx, c);
        nvgStroke(ctx);
        nvgClosePath(ctx);
    };
};

const NVGCall quick_text(const char *s, Point pos) {
    return text(s, "mono", __white, pos);
};