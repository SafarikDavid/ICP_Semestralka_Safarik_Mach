#version 430 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexcoord;
layout (location = 2) in vec3 aNormal;

uniform mat4 uMm = mat4(1.0);
uniform mat4 uVm = mat4(1.0);
uniform mat4 uPm = mat4(1.0);

out vec2 texcoord;
out vec3 normal;

void main()
{
    // Outputs the positions/coordinates of all vertices
    gl_Position = uPm * uVm * uMm * vec4(aPos, 1.0f);
    
    texcoord = aTexcoord;
    
    normal = aNormal; //incorrect, mus must be recomputed for light model
}
