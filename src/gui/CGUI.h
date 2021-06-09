#include "CDirectObject.h"
#include "CApplication.h"
#include "nanovg.h"
#include <SDL2/SDL.h>

class CGUI : public CDirectObject {
public:
	CApplication *itsApp;
	CGUI(CApplication *app);
	virtual ~CGUI() {}

	void Render(NVGcontext *ctx);
};