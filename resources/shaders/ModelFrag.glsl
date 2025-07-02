#version 430

layout(location = 0) out vec4 l_color;


in vec2 v_texCoord;
in vec3 v_normal;
in vec3 v_viewSpaceCoord;
in vec3 v_lightPos;


//NOTE: DO NOT UPDATE THESE without changing the code in mesh class
uniform sampler2D u_textureDiffuse0;
uniform sampler2D u_textureSpecular0;

uniform bool u_enableLighting;
uniform bool u_haveSpecularTexture;


float LightFallOff(float distance)
{
    if (distance > 50) {
        float divideFactor = abs(1.25 / (55 - distance));

        if (divideFactor < 1)
            divideFactor = 1;

        return (min(500.0 / pow(1.12, distance), 1.5)) / divideFactor;
    }
    return min(500.0 / pow(1.12, distance), 1.5);
}

void main()
{
    if (!u_enableLighting)
    {
        l_color = texture(u_textureDiffuse0, v_texCoord);
        return;
    }

    const float ambientStrength = 0.15;
    const float specularStrength = 0.2;

    float distance = abs(length(v_lightPos - v_viewSpaceCoord));
    l_color = texture(u_textureDiffuse0, v_texCoord);

    if (distance > 55) {
        l_color.rgb = l_color.rgb / 8;
        return; //If the distance is greater than 55, then we can just return immedietly as light will have next to no effect
    }

    float lightIntensityDiffuse = LightFallOff(distance);

    vec3 normal = normalize(v_normal);
    vec3 lightDir = normalize(v_lightPos - v_viewSpaceCoord);
    vec3 viewDir = normalize(-v_viewSpaceCoord);

    //Abient
    vec3 ambient = ambientStrength * vec3(1.0, 1.0, 1.0);//TODO: replace vec3 with u_lightColor

    //Diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * vec3(1.0, 1.0, 1.0);//TODO: replace vec3 with u_lightColor

    //specular
    vec3 specular;
    if (u_haveSpecularTexture)
    {
        specular = texture(u_textureSpecular0, v_texCoord).rgb * vec3(1.0, 1.0, 1.0) * vec3(specularStrength, specularStrength, specularStrength);//TODO: replace first vec3 with u_lightColor
    }
    else
    {
        vec3 reflectDir = reflect(-lightDir, normal);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 2);
        specular = (specularStrength * spec) * vec3(1.0, 1.0, 1.0); //TODO: replace vec3 with u_lightColor
    }

    vec3 lightingResult = ambient + (diffuse * lightIntensityDiffuse) + specular;

    l_color = vec4(lightingResult, 1.0) * l_color;
}
