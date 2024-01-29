#version 430 core

struct AmbientLight {  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform AmbientLight ambientLight;

struct PointLight {
    vec3 position;
  
    float constant;
    float linear;
    float quadratic; 

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform PointLight pointLight;

struct SpotLight {
    vec3  position;
    vec3  direction;
    float cutOff;
    float outerCutOff;
  
    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform SpotLight spotLight;

//uniform vec4 diffuse_material;
//uniform vec4 ambient_material;
uniform vec4 specular_material;
uniform float shininess;

uniform float alpha = 1.0f;

uniform sampler2D ourTexture;

uniform vec3 viewPos;

out vec4 FragColor;

in vec2 texcoord;
in vec3 normal;
in vec3 fragPos;
//in vec3 lightPos;

vec3 calculateAmbientLighting(AmbientLight light){
    // ambient
    vec3 ambient = vec3(texture(ourTexture, texcoord)) * light.ambient;
 
    // diffuse
    vec3 diffuse = vec3(texture(ourTexture, texcoord)) * light.diffuse;

    // specular
    vec3 specular = vec3(specular_material) * light.specular; 

    return ambient + diffuse + specular;
}

vec3 calculatePointLighting(PointLight light, vec3 norm, vec3 fragPos, vec3 viewDir){
    vec3 lightDir = normalize(light.position - fragPos);

    // ambient
    vec3 ambient = vec3(texture(ourTexture, texcoord)) * light.ambient;
 
    // diffuse 
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * vec3(texture(ourTexture, texcoord)) * light.diffuse;

    // specular
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = vec3(specular_material) * spec * light.specular; 
    
    // attenuation
    float distance    = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return ambient + diffuse + specular;
}

vec3 calculateSpotLighting(SpotLight light, vec3 norm, vec3 fragPos, vec3 viewDir){
    vec3 lightDir = normalize(light.position - fragPos);

    // ambient
    vec3 ambient = vec3(texture(ourTexture, texcoord)) * light.ambient;
 
    // diffuse 
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * vec3(texture(ourTexture, texcoord)) * light.diffuse;

    // specular
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = vec3(specular_material) * spec * light.specular; 
    
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));   
    
    // spotlight intensity
    float theta = dot(lightDir, normalize(-light.direction)); 
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;

    return (ambient + diffuse + specular);
}

void main()
{
    // properties
    vec3 norm = normalize(normal);
    vec3 viewDir = normalize(viewPos - fragPos);

    vec3 outputColor = vec3(0.0);

    outputColor += calculateAmbientLighting(ambientLight);
    outputColor += calculatePointLighting(pointLight, norm, fragPos, viewDir);
    outputColor += calculateSpotLighting(spotLight, norm, fragPos, viewDir);
    
    FragColor = vec4(outputColor, alpha);
}
