#version 150

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in VertexData    {
    vec4 color;
    vec2 texcoord;
} vVertexIn[];

out VertexData    {
    vec3 baricentric;
    vec4 color;
    vec2 texcoord;
} vVertexOut;

void main(void)
{
    vVertexOut.baricentric = vec3(1, 0, 0);
    vVertexOut.color = vVertexIn[0].color;
    vVertexOut.texcoord = vVertexIn[0].texcoord;
    gl_Position = gl_in[0].gl_Position;
    EmitVertex();
    
    vVertexOut.baricentric = vec3(0, 1, 0);
    vVertexOut.color = vVertexIn[1].color;
    vVertexOut.texcoord = vVertexIn[1].texcoord;
    gl_Position = gl_in[1].gl_Position;
    EmitVertex();
    
    vVertexOut.baricentric = vec3(0, 0, 1);
    vVertexOut.color = vVertexIn[2].color;
    vVertexOut.texcoord = vVertexIn[2].texcoord;
    gl_Position = gl_in[2].gl_Position;
    EmitVertex();
    
    EndPrimitive();
}

