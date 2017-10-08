#version 450 core

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inTexCoord;

uniform Uniforms {
    mat4 model;
    mat4 view;
    mat4 projection;
} u;

out VertexData {
    vec4 normal;
    vec2 texCoord;
    vec3 viewPos;
} o;

void main()
{
    o.viewPos = (u.view * u.model * vec4(inPosition, 1.0f)).xyz;
    gl_Position = u.projection * vec4(o.viewPos, 1.0f);
    o.normal = u.model * vec4(inNormal, 0); // TODO: Do inverse transpose
    o.texCoord = inTexCoord;
}
