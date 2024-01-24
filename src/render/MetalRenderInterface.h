#ifndef MetalRenderInterface_h
#define MetalRenderInterface_h

#include "nanovg.h"
#include "MetalShaderTypes.h"

void * MetalRenderCreate(void *, NVGcontext *);
void   MetalRenderSky(void *, SkyUniforms *);
void   MetalRenderFrame(void *);

#endif /* MetalRenderInterface_h */
