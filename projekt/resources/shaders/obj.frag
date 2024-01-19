#version 430 core

uniform vec4 diffuse_material;
uniform vec4 ambient_material;
uniform vec4 specular_material;
uniform float shininess;

out vec4 FragColor;

in vec2 texcoord;
in vec3 normal;
in vec3 fragPos;
in vec3 lightPos;

uniform sampler2D ourTexture;
uniform vec3 uLightColor;

void main()
{
    // ambient
    vec3 ambient = vec3(ambient_material) * uLightColor;
 
    // diffuse 
    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(lightPos - fragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = (diff * vec3(diffuse_material)) * uLightColor;

    // specular
    // vec3 viewDir = normalize(viewPos - fragPos); predelat na tohle. viewPos asi bude pozice kamery
    vec3 viewDir = normalize(-fragPos); // the viewer is always at (0,0,0) in view-space, so viewDir is (0,0,0) - Position => -Position
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = (vec3(specular_material) * spec) * uLightColor; 
    
    //vec3 result = (ambient + diffuse + specular) * diffuse_material;
    //FragColor = vec4(result, 1.0);
    FragColor = texture(ourTexture, texcoord) * vec4(ambient + diffuse + specular, 1.0);
//FragColor = texture(ourTexture, texcoord) * diffuse_material * vec4(ambient + diffuse + specular, 1.0);
}
