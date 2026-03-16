//
//  GUIItem.cpp
//  Avara
//
//  Created by Andy Halstead on 2/24/26.
//

#include "GUIItem.h"

void GUIItem::Draw(NVGcontext *ctx) {
    //SDL_Log("Draw %d", itemType);
    switch (itemType) {
        case GUIItemType::JustLine:
            nvgBeginPath(ctx);
            nvgStrokeColor(ctx, color);
            nvgMoveTo(ctx, rect.x, rect.y);
            nvgLineTo(ctx, rect.x + rect.w, rect.y);
            nvgStrokeWidth(ctx, rect.h);
            nvgStroke(ctx);
            nvgClosePath(ctx);
            break;
        case GUIItemType::JustText:
            NVGRect(ctx);
            NVGText(ctx);
            break;
        case GUIItemType::JustRect:
            NVGRect(ctx);
            break; 
        case GUIItemType::Pane:
            NVGPane(ctx);
            if (text.length()) {
                NVGText(ctx);
            }
            break;
        case GUIItemType::Button:
            NVGBtn(ctx);
            break;
        case GUIItemType::TextInput:
            NVGRectPath(ctx);
            NVGText(ctx);
            break;
        case GUIItemType::Checkbox:
            NVGRectPath(ctx);
            break;
        case GUIItemType::Dropdown:
            NVGRect(ctx);
            break;
    }
}

void GUIItem::NVGText(NVGcontext *ctx) {
    nvgFontFace(ctx, "mono");
    if (textSize > 0) nvgFontSize(ctx, textSize);
    else nvgFontSize(ctx, std::round(rect.h * .5f));
    nvgFillColor(ctx, textColor);
    if (textAlign > 0) {
        nvgTextAlign(ctx, textAlign);
        nvgText(ctx, rect.x, rect.y + (rect.h / 2), text.c_str(), NULL);
    }
    else {
        nvgTextAlign(ctx, NVG_ALIGN_CENTER|NVG_ALIGN_MIDDLE);
        nvgText(ctx, rect.x + (rect.w * .5f), rect.y + (rect.h * .5f), text.c_str(), NULL);
    }
}

void GUIItem::NVGPane(NVGcontext *ctx) {
    nvgBeginPath(ctx);
    nvgFillColor(ctx, nvgRGBA(20, 20, 20, 160));
    nvgRoundedRect(ctx, rect.x, rect.y, rect.w, rect.h, 4.0);
    nvgFill(ctx);
    /*
    if (header_height > 0) {
        nvgBeginPath(ctx);
        nvgFillColor(ctx, nvgRGBA(0, 0, 0, 160));
        nvgRoundedRect(ctx, rect.x, rect.y, rect.w, header_height, 4.0);
        nvgFill(ctx);
    }
    */
    //drop shadow
    NVGpaint shadowPaint = nvgBoxGradient(ctx, rect.x, rect.y + 2, rect.w, rect.h, 8, 10, nvgRGBA(0,0,0,128), nvgRGBA(0,0,0,0));
    nvgBeginPath(ctx);
    nvgRect(ctx, rect.x - 10, rect.y - 10, rect.w + 20, rect.h + 30);
    nvgRoundedRect(ctx, rect.x, rect.y, rect.w, rect.h, 4.0);
    nvgPathWinding(ctx, NVG_HOLE);
    nvgFillPaint(ctx, shadowPaint);
    nvgFill(ctx);
}

void GUIItem::NVGBtn(NVGcontext *ctx) {
    if (focus) {
        outline = 10;
    }
    else outline = 0;
    NVGRect(ctx);
    NVGText(ctx);
}

void GUIItem::NVGRect(NVGcontext *ctx) {
    nvgBeginPath(ctx);
    if (outline > 0) {
        NVGcolor outliner;
        outliner.r = color.r * .5;
        outliner.g = color.g * .5;
        outliner.b = color.b * .5;
        outliner.a = color.a;
        NVGrect outlinerect;
        outlinerect.x = rect.x - 10;
        outlinerect.y = rect.y - 10;
        outlinerect.w = rect.w + 20;
        outlinerect.h = rect.h + 20;
        nvgRect(ctx, outlinerect.x, outlinerect.y, outlinerect.w, outlinerect.h);
        nvgFillColor(ctx, outliner);
        nvgFill(ctx);
    }
    nvgRect(ctx, rect.x, rect.y, rect.w, rect.h);
    nvgFillColor(ctx, color);
    nvgFill(ctx);
}

void GUIItem::NVGRectPath(NVGcontext *ctx) {
    nvgBeginPath(ctx);
    nvgRect(ctx, rect.x, rect.y, rect.w, rect.h);
    nvgStrokeColor(ctx, textColor);
    nvgStroke(ctx);
    nvgClosePath(ctx);
}
