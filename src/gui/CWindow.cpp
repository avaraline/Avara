#include "CWindow.h"

#include "CApplication.h"

CWindow::CWindow(CApplication *app, const std::string &title) :
  nanogui::Window((nanogui::Widget *)app, title), mApplication(app) {
    app->Register(this);
}

CWindow::~CWindow() {
    mApplication->Unregister(this);
}
