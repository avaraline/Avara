#include "CWindow.h"

#include "CApplication.h"

CWindow::CWindow(CApplication *app, const std::string &title) :
  nanogui::Window((nanogui::Widget *)app, title), mApplication(app) {
    app->Register(this);
}

CWindow::~CWindow() {
    mApplication->Unregister(this);
}

void CWindow::restoreState() {
    long x = mApplication->Number(title() + "X", 50);
    long y = mApplication->Number(title() + "Y", 50);
    setPosition(nanogui::Vector2i(x, y));
}

void CWindow::saveState() {
    nanogui::Vector2i position = this->position();
    mApplication->Set(title() + "X", position.x);
    mApplication->Set(title() + "Y", position.y);
}
