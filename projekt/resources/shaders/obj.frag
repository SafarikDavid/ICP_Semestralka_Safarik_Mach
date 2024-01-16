#version 430 core

uniform vec4 diffuse_material;

out vec4 FragColor;

void main()
{
    FragColor = diffuse_material;
}
