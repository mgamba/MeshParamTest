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
    vec3 a3 = smoothstep( vec3(0.0), d * 1.5, vVertexIn.baricentric );
    return min(min(a3.x, a3.y), a3.z);
}

void main(void) {
    // determine frag distance to closest edge
    float fEdgeIntensity = 1.0 - edgeFactor();
    
    // blend between edge color and face color
    vec4 vFaceColor = texture( uTexture, vVertexIn.texcoord ) * vVertexIn.color; vFaceColor.a = 0.85;
    vec4 vEdgeColor = vec4(0.0, 1.0, 0.0, 0.85);
    oColor = mix(vFaceColor, vEdgeColor, fEdgeIntensity);
}

