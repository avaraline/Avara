#version 330 core

in vec2 texCoords;

uniform sampler2D screenTexture;
uniform int fxaa;
uniform vec2 texelStep;
uniform float lumaThreshold;
uniform float mulReduce;
uniform float minReduce;
uniform float maxSpan;

out vec4 color;

void main()
{
    vec3 rgbM = texture(screenTexture, texCoords).rgb;

    // Possibility to toggle FXAA on and off.
    if (fxaa == 0)
    {
        color = vec4(rgbM, 1.0);

        return;
    }

    // Sampling neighbour texels. Offsets are adapted to OpenGL texture coordinates.
    vec3 rgbNW = textureOffset(screenTexture, texCoords, ivec2(-1, 1)).rgb;
    vec3 rgbNE = textureOffset(screenTexture, texCoords, ivec2(1, 1)).rgb;
    vec3 rgbSW = textureOffset(screenTexture, texCoords, ivec2(-1, -1)).rgb;
    vec3 rgbSE = textureOffset(screenTexture, texCoords, ivec2(1, -1)).rgb;

    // see http://en.wikipedia.org/wiki/Grayscale
    const vec3 toLuma = vec3(0.299, 0.587, 0.114);

    // Convert from RGB to luma.
    float lumaNW = dot(rgbNW, toLuma);
    float lumaNE = dot(rgbNE, toLuma);
    float lumaSW = dot(rgbSW, toLuma);
    float lumaSE = dot(rgbSE, toLuma);
    float lumaM = dot(rgbM, toLuma);

    // Gather minimum and maximum luma.
    float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
    float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));

    // If contrast is lower than a maximum threshold ...
    if (lumaMax - lumaMin <= lumaMax * lumaThreshold)
    {
        // ... do no AA and return.
        color = vec4(rgbM, 1.0);

        return;
    }

    // Sampling is done along the gradient.
    vec2 samplingDirection;
    samplingDirection.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
    samplingDirection.y =  ((lumaNW + lumaSW) - (lumaNE + lumaSE));

    // Sampling step distance depends on the luma: The brighter the sampled texels, the smaller the final sampling step direction.
    // This results, that brighter areas are less blurred/more sharper than dark areas.
    float samplingDirectionReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) * 0.25 * mulReduce, minReduce);

    // Factor for norming the sampling direction plus adding the brightness influence.
    float minSamplingDirectionFactor = 1.0 / (min(abs(samplingDirection.x), abs(samplingDirection.y)) + samplingDirectionReduce);

    // Calculate final sampling direction vector by reducing, clamping to a range and finally adapting to the texture size.
    samplingDirection = clamp(samplingDirection * minSamplingDirectionFactor, vec2(-maxSpan), vec2(maxSpan)) * texelStep;

    // Inner samples on the tab.
    vec3 rgbSampleNeg = texture(screenTexture, texCoords + samplingDirection * (1.0/3.0 - 0.5)).rgb;
    vec3 rgbSamplePos = texture(screenTexture, texCoords + samplingDirection * (2.0/3.0 - 0.5)).rgb;

    vec3 rgbTwoTab = (rgbSamplePos + rgbSampleNeg) * 0.5;

    // Outer samples on the tab.
    vec3 rgbSampleNegOuter = texture(screenTexture, texCoords + samplingDirection * (0.0/3.0 - 0.5)).rgb;
    vec3 rgbSamplePosOuter = texture(screenTexture, texCoords + samplingDirection * (3.0/3.0 - 0.5)).rgb;

    vec3 rgbFourTab = (rgbSamplePosOuter + rgbSampleNegOuter) * 0.25 + rgbTwoTab * 0.5;

    // Calculate luma for checking against the minimum and maximum value.
    float lumaFourTab = dot(rgbFourTab, toLuma);

    // Are outer samples of the tab beyond the edge ...
    if (lumaFourTab < lumaMin || lumaFourTab > lumaMax)
    {
        // ... yes, so use only two samples.
        color = vec4(rgbTwoTab, 1.0);
    }
    else
    {
        // ... no, so use four samples.
        color = vec4(rgbFourTab, 1.0);
    }
}
