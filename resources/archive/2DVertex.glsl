#version 430

layout(location = 0) in vec2 l_position;
layout(location = 1) in vec2 l_texCoord;

out vec2 v_texCoord;

uniform mat4 u_projMatrix;
uniform float u_z;

void main()
{
    gl_Position = u_projMatrix * vec4(l_position, u_z, 1.0);
    v_texCoord = l_texCoord;
}