#pragma once
#include "nanovg.h"
#include "CDirectObject.h"
#include <functional>
#include "Types.h"



typedef std::function<void(NVGcontext *c)> NVGCall;
NVGCall flat_map(NVGCall a, NVGCall b);
const Point pt(short x, short y);
const Point midpoint(Rect &r);
const NVGCall text(const char *s, const char *font, NVGcolor color, Point pos);
const NVGCall rect(Rect &r, NVGcolor &c);
const NVGCall quick_text(const char *s, Point pos);