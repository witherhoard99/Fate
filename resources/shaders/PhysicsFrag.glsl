#version 430

layout(location = 0) out vec4 l_color;

in vec4 v_color;

void main()
{
    l_color = v_color;
}