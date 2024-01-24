#import <Foundation/Foundation.h>
#import <MetalKit/MetalKit.h>
#import <QuartzCore/CAMetalLayer.h>
#import "MetalRenderInterface.h"

@interface MetalRendererImpl : NSObject

- (nonnull instancetype)initWithMetalLayer:(nonnull CAMetalLayer *)layer nvgContext:(nullable NVGcontext *)ctx;

@end
