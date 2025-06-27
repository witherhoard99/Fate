#version 430

in vec2 v_texCoord;

layout(location = 0) out vec4 l_color;

uniform sampler2D u_diffuseTexture;

void main()
{
    l_color = texture(u_diffuseTexture, v_texCoord);
}