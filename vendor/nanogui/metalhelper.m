//
//  metalhelper.m
//  Avara
//
//  Created by Daniel Watson on 1/22/24.
//

#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#include <SDL2/SDL.h>

void setupMetalView(SDL_MetalView view) {
    CAMetalLayer *layer = (__bridge CAMetalLayer *)SDL_Metal_GetLayer(view);
    layer.device = MTLCreateSystemDefaultDevice();
    //layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
}
