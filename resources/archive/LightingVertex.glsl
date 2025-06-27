#version 430


layout(location = 0) in vec4 l_position;
layout(location = 1) in vec2 l_texCoord;
layout(location = 2) in vec3 l_normal;


out vec2 v_texCoord;
out vec3 v_normal;
out vec3 v_viewSpaceCoord;
out vec3 v_lightPos;


uniform mat4 u_MVP;
uniform mat4 u_viewMatrix;
uniform mat4 u_modelMatrix;

uniform vec3 u_lightPos;
uniform float u_time;


void main()
{
    gl_Position = u_MVP * l_position;

    v_texCoord = l_texCoord;
    v_lightPos = vec3(u_viewMatrix * vec4(u_lightPos, 1.0));

    v_normal = mat3(transpose(inverse(u_viewMatrix * u_modelMatrix))) * l_normal;
    v_viewSpaceCoord = vec3(u_viewMatrix * u_modelMatrix * l_position);
}
