//
//  MetalRendererImpl.m
//  Avara
//
//  Created by Daniel Watson on 1/22/24.
//

#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#import "MetalRendererImpl.h"

#include <SDL2/SDL.h>
#include "MetalShaderTypes.h"
#include "nanovg_mtl.h"
#include "Skybox.h"

@implementation MetalRendererImpl {
    NVGcontext *_nvg;
    CAMetalLayer *_layer;
    
    id <MTLDevice> _device;
    id <MTLCommandQueue> _commandQueue;
    
    // Pipelines
    id <MTLRenderPipelineState> _sky;
    
    // Skybox vertex and uniform buffers
    id <MTLBuffer> _svb;
    id <MTLBuffer> _sub;
}

- (nonnull instancetype)initWithMetalLayer:(CAMetalLayer *)layer nvgContext:(NVGcontext *)ctx
{
    self = [super init];
    if(self)
    {
        _nvg = ctx;
        _layer = layer;
        _device = (__bridge id<MTLDevice>)(mnvgDevice(ctx));
        _commandQueue = (__bridge id<MTLCommandQueue>)(mnvgCommandQueue(ctx));
        [self setupMetal:layer];
    }
    return self;
}

- (void)setupMetal:(nonnull CAMetalLayer *)layer {
    NSError *err;

    id <MTLLibrary> lib = [_device newDefaultLibrary];
    id <MTLFunction> skyVtx = [lib newFunctionWithName:@"vtxSky"];
    id <MTLFunction> skyFrg = [lib newFunctionWithName:@"frgSky"];
    
    // Create the Skybox render pipeline
    MTLVertexDescriptor *skyVtxDesc = [MTLVertexDescriptor new];
    skyVtxDesc.attributes[AttribPosition].format = MTLVertexFormatFloat3;
    skyVtxDesc.attributes[AttribPosition].offset = 0;
    skyVtxDesc.attributes[AttribPosition].bufferIndex = 0;
    skyVtxDesc.layouts[0].stride = 12;
    skyVtxDesc.layouts[0].stepRate = 1;
    skyVtxDesc.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;

    MTLRenderPipelineDescriptor *skyDesc = [MTLRenderPipelineDescriptor new];
    skyDesc.label = @"SkyBox";
    skyDesc.rasterSampleCount = 1;
    skyDesc.vertexFunction = skyVtx;
    skyDesc.fragmentFunction = skyFrg;
    skyDesc.vertexDescriptor = skyVtxDesc;
    skyDesc.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
    skyDesc.colorAttachments[0].blendingEnabled = false;
    
    _sky = [_device newRenderPipelineStateWithDescriptor:skyDesc error:&err];
    
    // Allocate a buffer to hold the skybox vertex data.
    // Usage mode set to Private since we will never need to access the data with the CPU again,
    // allowing the driver to optimize the data (although it doesn't matter with data this small)
    _svb = [_device newBufferWithBytes:skyboxVertices length:sizeof skyboxVertices options:MTLResourceStorageModePrivate];

    // A buffer to hold the skybox vertex + fragment uniforms.
    _sub = [_device newBufferWithLength:sizeof(SkyUniforms) options:MTLResourceStorageModeShared];

}

- (void)renderSky:(nonnull SkyUniforms *)su {
    SkyUniforms *ubuf = (SkyUniforms *)[_sub contents];
    *ubuf = *su;

    id<CAMetalDrawable> drawable = (__bridge id<CAMetalDrawable>)mnvgDrawable(_nvg);

    id<MTLCommandBuffer> buffer = [_commandQueue commandBuffer];
    MTLRenderPassDescriptor *pass = [MTLRenderPassDescriptor renderPassDescriptor];
    //pass.colorAttachments[0].clearColor = color;
    pass.colorAttachments[0].loadAction  = MTLLoadActionClear;
    pass.colorAttachments[0].storeAction = MTLStoreActionStore;
    pass.colorAttachments[0].texture = drawable.texture;

    id<MTLRenderCommandEncoder> encoder = [buffer renderCommandEncoderWithDescriptor:pass];
    
    assert(drawable != nil);
    assert(encoder != nil);
    
    [encoder setCullMode:MTLCullModeNone];
    [encoder setVertexBuffer:_svb offset:0 atIndex:0];
    [encoder setVertexBuffer:_sub offset:0 atIndex:1];
    [encoder setFragmentBuffer:_sub offset:0 atIndex:1];
    [encoder setRenderPipelineState:_sky];
    [encoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:36];
    [encoder endEncoding];

    [buffer presentDrawable:drawable];
    [buffer commit];
}

void * MetalRenderCreate(void *layer, NVGcontext *nvg) {
    return (void *)CFBridgingRetain([[MetalRendererImpl alloc] initWithMetalLayer:(__bridge CAMetalLayer *)layer nvgContext:nvg]);
}

void MetalRenderSky(void *impl, SkyUniforms *su) {
    MetalRendererImpl *render = (__bridge MetalRendererImpl *)impl;
    [render renderSky:su];
}

void MetalRenderFrame(void *impl) {
    MetalRendererImpl *render = (__bridge MetalRendererImpl *)impl;
    
    /*
    @autoreleasepool {
        MTLClearColor color = MTLClearColorMake(1, 0, 0, 1);
        id<CAMetalDrawable> surface = [render->_layer nextDrawable];

        MTLRenderPassDescriptor *pass = [MTLRenderPassDescriptor renderPassDescriptor];
        pass.colorAttachments[0].clearColor = color;
        pass.colorAttachments[0].loadAction  = MTLLoadActionClear;
        pass.colorAttachments[0].storeAction = MTLStoreActionStore;
        pass.colorAttachments[0].texture = surface.texture;

        id<MTLCommandBuffer> buffer = [render->_commandQueue commandBuffer];
        [buffer enqueue];
        id<MTLRenderCommandEncoder> encoder = [buffer renderCommandEncoderWithDescriptor:pass];
        [encoder endEncoding];
        [buffer presentDrawable:surface];
        [buffer commit];
    }
    */
}

@end
