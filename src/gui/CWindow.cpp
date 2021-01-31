#include "CWindow.h"

#include "CApplication.h"
#include "limits.h"

CWindow::CWindow(CApplication *app, const std::string &title) : mApplication(app) {
    app->Register(this);
}

CWindow::~CWindow() {
    mApplication->Unregister(this);
}

void CWindow::restoreState() {
    long x = mApplication->Number(title() + "X", LONG_MIN);
    long y = mApplication->Number(title() + "Y", LONG_MIN);
    
    if(x != LONG_MIN && y != LONG_MIN)
        SDL_SetWindowPosition(mApplication->window, x, y);
}

void CWindow::saveState() {
    int posx, posy;
    SDL_GetWindowPosition(mApplication->window, &posx, &posy)
    mApplication->Set(title() + "X", posx);
    mApplication->Set(title() + "Y", posy);
}
