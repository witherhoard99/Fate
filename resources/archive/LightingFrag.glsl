#version 430

layout(location = 0) out vec4 l_color;


in vec2 v_texCoord;
in vec3 v_normal;
in vec3 v_viewSpaceCoord;
in vec3 v_lightPos;


uniform sampler2D u_texture;
uniform vec4 u_color;
uniform bool u_enableLighting;


float LightFallOff(float value)
{
    return min(15.0 / pow(value, 1.6), 1);
}


void main()
{
    if (!u_enableLighting)
    {
        l_color = texture(u_texture, v_texCoord);
        if (l_color.a <= 0.05)
            discard;
        return;
    }
    else
    {
        l_color = texture(u_texture, v_texCoord);
    }

    float ambientStrength = 0.15;
    float specularStrength = 0;
    float lightIntensityDiffuse = LightFallOff(length(v_lightPos - v_viewSpaceCoord));

    vec3 normal = normalize(v_normal);
    vec3 lightDir = normalize(v_lightPos - v_viewSpaceCoord);
    vec3 viewDir = normalize(-v_viewSpaceCoord);

    //Abient
    vec3 ambient = ambientStrength * vec3(1.0, 1.0, 1.0); //TODO: replace vec3 with u_lightColor later

    //Diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * vec3(1.0, 1.0, 1.0); //TODO: replace vec3 with u_lightColor later

    //Specular
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 2);
    vec3 specular = (specularStrength * spec) * vec3(1.0, 1.0, 1.0); //TODO: replace vec3 with u_lightColor later

    vec3 lightingResult = ambient + (diffuse * lightIntensityDiffuse) + specular;
    vec4 objectColor = vec4(0.0, 0.5, 0.6, 1.0);

    l_color = vec4(lightingResult, 1.0) * l_color;

    if (l_color.a <= 0.05)
        discard;
}