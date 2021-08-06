#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D texture_diffuse0;
uniform sampler2D texture_diffuse1;

void main()
{
    FragColor = mix(texture(texture_diffuse0, TexCoord), texture(texture_diffuse1, TexCoord), 0.0);
}