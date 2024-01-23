#version 430 core

struct Light {
    vec3 position;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Light light;

//uniform vec4 diffuse_material;
//uniform vec4 ambient_material;
uniform vec4 specular_material;
uniform float shininess;

uniform vec3 viewPos;

out vec4 FragColor;

in vec2 texcoord;
in vec3 normal;
in vec3 fragPos;
in vec3 lightPos;

uniform sampler2D ourTexture;
//uniform vec3 uLightColor;

void main()
{
    // ambient
//    vec3 ambient = vec3(ambient_material) * light.ambient;
    vec3 ambient = vec3(texture(ourTexture, texcoord)) * light.ambient;
 
    // diffuse 
    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(lightPos - fragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    //vec3 diffuse = diff * vec3(diffuse_material) * light.diffuse;
    vec3 diffuse = diff * vec3(texture(ourTexture, texcoord)) * light.diffuse;

    // specular
    // je potreba? pozice kamery je asi uz zapocitana v normale. Ve Vertex shaderu se pocita z viewMatrix.
    vec3 viewDir = normalize(viewPos-fragPos);  // od pozice kamery se odecita frag position
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = (vec3(specular_material) * spec) * light.specular; 
    
//    FragColor = texture(ourTexture, texcoord) * vec4(ambient + diffuse + specular, 1.0);
    FragColor = vec4(ambient + diffuse + specular, 1.0);
}
