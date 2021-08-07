#version 330 core
in vec3 Normal;
out vec4 FragColor;

uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPos;

void main()
{
    float ambientStrength = 0.4;
    vec3 ambient = ambientStrength * lightColor;

    FragColor = vec4(lightColor * objectColor, 1.0);
    //FragColor = vec4(1.0);
}