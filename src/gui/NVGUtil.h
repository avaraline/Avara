//
//  NVGUtil.h
//  Avara
//
//  Created by Andy Halstead on 8/21/26.
//

#ifndef NVGUtil_h
#define NVGUtil_h

#include <nanogui/nanogui.h>
#include "ARGBColor.h"

#define phi 1.618
#define phicube (phi * phi * phi)

enum GUIState {
    title = 0,
    tracker,
    joinedServer,
    hostSettings,
    hostServer,
    singlePlayer,
    settings,
    about
};

inline static void DrawImage(NVGcontext* ctx, int image, float alpha,
        float sx, float sy, float sw, float sh, // sprite location on texture
        float x, float y, float w, float h) // position and size of the sprite rectangle on screen
{
    float ax, ay;
    int iw,ih;
    NVGpaint img;

    nvgImageSize(ctx, image, &iw, &ih);

    // Aspect ration of pixel in x an y dimensions. This allows us to scale
    // the sprite to fill the whole rectangle.
    ax = w / sw;
    ay = h / sh;

    img = nvgImagePattern(ctx, x - sx*ax, y - sy*ay, (float)iw*ax, (float)ih*ay,
                0, image, alpha);
    nvgBeginPath(ctx);
    nvgRect(ctx, x,y, w,h);
    nvgFillPaint(ctx, img);
    nvgFill(ctx);
}

inline static nanogui::Color ToNanoguiColor(const ARGBColor &argbColor) {
    return nanogui::Color(argbColor.GetR(),
                          argbColor.GetG(),
                          argbColor.GetB(),
                          argbColor.GetA());
}

class SpriteWidget : nanogui::Widget {
public:
    SpriteWidget(Widget* parent) : Widget(parent), mSize(48), mSourceSize(48),
    mOffset(nanogui::Vector2i(0, 0)), mImageSize(nanogui::Vector2i(0, 0)) {

    };
    nanogui::Vector2i preferredSize(NVGcontext *ctx) const {
        return nanogui::Vector2i(mSize, mSize);
    };
    void setImage(NVGcontext *ctx, int p) {
        mImageDataHandle = p;
        nvgImageSize(ctx, mImageDataHandle, &mImageSize.x, &mImageSize.y);
    };
    void setSpriteSize(int size) {
        mSourceSize = size;
    }
    void setDisplaySize(int size) {
        mSize = size;
    }
    void setOffset(int x, int y) {
        mOffset.x = x;
        mOffset.y = y;
    }
    void draw(NVGcontext *ctx) {
        DrawImage(ctx, mImageDataHandle, 1.0, mOffset.x, mOffset.y, mSourceSize, mSourceSize, mPos.x, mPos.y, mSize, mSize);
    };
private:
    int mSize = 48;
    int mSourceSize = 48;
    int mImageDataHandle;
    nanogui::Vector2i mOffset;
    nanogui::Vector2i mImageSize;
};

#endif /* NVGUtil_h */
