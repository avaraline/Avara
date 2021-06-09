#include "CGUI.h"
#include "CApplication.h"

CGUI::CGUI(CApplication *app) {
	itsApp = app;
}

void CGUI::Render(NVGcontext *ctx) {
	nvgBeginFrame(ctx, itsApp->win_size_x, itsApp->win_size_y, itsApp->pixel_ratio);
    nvgBeginPath(ctx);
    nvgFontFace(ctx, "mono");
	nvgFillColor(ctx, nvgRGBA(255,255,255,255));
	nvgTextAlign(ctx, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
	nvgFontSize(ctx, 25);
	nvgText(ctx, 100, 100, "hello from CGUI.cpp", NULL);
	nvgEndFrame(ctx);
}