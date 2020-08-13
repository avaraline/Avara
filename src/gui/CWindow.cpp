#include "CWindow.h"

#include "CApplication.h"
#include "limits.h"

CWindow::CWindow(CApplication *app, const std::string &title) :
  nanogui::Window((nanogui::Widget *)app, title), mApplication(app) {
    app->Register(this);
}

CWindow::~CWindow() {
    mApplication->Unregister(this);
}

void CWindow::restoreState() {
    long x = mApplication->Number(title() + "X", LONG_MIN);
    long y = mApplication->Number(title() + "Y", LONG_MIN);
    
    if(x != LONG_MIN && y != LONG_MIN)
        setPosition(nanogui::Vector2i(x, y));
}

void CWindow::saveState() {
    nanogui::Vector2i position = this->position();
    mApplication->Set(title() + "X", position.x);
    mApplication->Set(title() + "Y", position.y);
}
