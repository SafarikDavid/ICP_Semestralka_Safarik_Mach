#version 430 core

uniform vec4 diffuse_material;

out vec4 FragColor;

in vec2 texcoord;

uniform sampler2D ourTexture;

void main()
{

    FragColor = texture(ourTexture, texcoord) * diffuse_material;
}
