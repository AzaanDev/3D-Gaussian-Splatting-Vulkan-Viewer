
#version 430 core

layout(location = 0) in vec3 inColor;
layout(location = 1) in vec3 inConic;
layout(location = 2) in float inAlpha;
layout(location = 3) in vec2 inCoordxy;
 
layout(location = 0) out vec4 fragColor;


void main()
{
    float power = -0.5f * (inConic.x * inCoordxy.x * inCoordxy.x + inConic.z * inCoordxy.y * inCoordxy.y) - inConic.y * inCoordxy.x * inCoordxy.y;
    if (power > 0.f)
        discard;
    float opacity = min(0.99f, inAlpha * exp(power));
    if (opacity < 1.f / 255.f)
        discard;
    fragColor = vec4(inColor, opacity);
 }