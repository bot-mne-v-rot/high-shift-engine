#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

struct Material {
    sampler2D texture_diffuse0;
    sampler2D texture_specular0;
};

uniform Material material;

void main() {
    vec4 diffuse_color = texture(material.texture_diffuse0, TexCoord);
    vec4 specular_color = texture(material.texture_specular0, TexCoord);
    FragColor = mix(diffuse_color, specular_color, 0.0);
}