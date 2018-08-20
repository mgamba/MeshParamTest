#version 150

uniform sampler2D uTexture;

in VertexData    {
    vec3 baricentric;
    vec4 color;
    vec2 texcoord;
} vVertexIn;

out vec4                oColor;

float edgeFactor()
{
    vec3 d = fwidth( vVertexIn.baricentric );
    vec3 a3 = smoothstep( vec3(0.5), d * 1.5, vVertexIn.baricentric );
    return min(min(a3.x, a3.y), a3.z);
}

void main(void) {
    float smallest = min(min(vVertexIn.baricentric.x, vVertexIn.baricentric.y), vVertexIn.baricentric.z);
    float brightness;
    if (smallest < 0.02) {
        brightness = 1.000;
    } else {
        brightness = smoothstep(0.500, 0.000, smallest+0.15);
    }
    
    oColor = vec4(0.0, brightness, 0.0, 1.000);
}


