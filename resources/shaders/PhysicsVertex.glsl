#version 430

layout(location = 0) in vec3 l_position;
layout(location = 1) in vec4 l_color;

uniform mat4 u_MVP;

out vec4 v_color;

void main()
{
    vec4 position = vec4(l_position, 1.0);
    gl_Position = u_MVP * position;

    v_color = l_color;
}
