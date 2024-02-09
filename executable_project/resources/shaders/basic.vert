#version 430 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

out vec3 color;

uniform mat4 uMm = mat4(1.0);
uniform mat4 uVm = mat4(1.0);
uniform mat4 uPm = mat4(1.0);

void main()
{
    // Outputs the positions/coordinates of all vertices
    gl_Position = uPm * uVm * uMm * vec4(aPos, 1.0f);
    
    color = aColor;
}
