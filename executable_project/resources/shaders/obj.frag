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

struct DirectionalLight{
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform DirectionalLight directionalLight;

uniform vec4 specular_material;
uniform float shininess;

uniform sampler2D ourTexture;

uniform vec3 viewPos;

out vec4 FragColor;

in vec2 texcoord;
in vec3 normal;
in vec3 fragPos;

vec4 calculateAmbientLighting(AmbientLight light){
    // ambient
    vec4 ambient = texture(ourTexture, texcoord) * vec4(light.ambient, 1.0);
 
    // diffuse
    vec4 diffuse = texture(ourTexture, texcoord) * vec4(light.diffuse, 1.0);

    // specular
    vec4 specular = specular_material * vec4(light.specular, 1.0); 

    return ambient + diffuse + specular;
}

vec4 calculatePointLighting(PointLight light, vec3 norm, vec3 fragPos, vec3 viewDir){
    vec3 lightDir = normalize(light.position - fragPos);

    // ambient
    vec4 ambient = texture(ourTexture, texcoord) * vec4(light.ambient, 1.0);
 
    // diffuse 
    float diff = max(dot(norm, lightDir), 0.0);
    vec4 diffuse = diff * texture(ourTexture, texcoord) * vec4(light.diffuse, 1.0);

    // specular
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec4 specular = specular_material * spec * vec4(light.specular, 1.0); 
    
    // attenuation
    float distance    = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return ambient + diffuse + specular;
}

vec4 calculateSpotLighting(SpotLight light, vec3 norm, vec3 fragPos, vec3 viewDir){
    vec3 lightDir = normalize(light.position - fragPos);

    // ambient
    vec4 ambient = texture(ourTexture, texcoord) * vec4(light.ambient, 1.0);
 
    // diffuse 
    float diff = max(dot(norm, lightDir), 0.0);
    vec4 diffuse = diff * texture(ourTexture, texcoord) * vec4(light.diffuse, 1.0);

    // specular
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec4 specular = specular_material * spec * vec4(light.specular, 1.0); 
    
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

vec4 calculateDirectionalLighting(DirectionalLight light, vec3 norm, vec3 fragPos, vec3 viewDir){
    vec3 lightDir = normalize(-light.direction);

    // ambient
    vec4 ambient = texture(ourTexture, texcoord) * vec4(light.ambient, 1.0);
 
    // diffuse 
    float diff = max(dot(norm, lightDir), 0.0);
    vec4 diffuse = diff * texture(ourTexture, texcoord) * vec4(light.diffuse, 1.0);

    // specular
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec4 specular = specular_material * spec * vec4(light.specular, 1.0); 

    return (ambient + diffuse + specular);
}

void main()
{
    // properties
    vec3 norm = normalize(normal);
    vec3 viewDir = normalize(viewPos - fragPos);

    // init output color
    vec4 outputColor = vec4(0.0);

    // calculate all lighting
    outputColor += calculateAmbientLighting(ambientLight);
    outputColor += calculatePointLighting(pointLight, norm, fragPos, viewDir);
    outputColor += calculateSpotLighting(spotLight, norm, fragPos, viewDir);
    outputColor += calculateDirectionalLighting(directionalLight, norm, fragPos, viewDir);    

    // Set the alpha channel from the texture
    outputColor.a = texture(ourTexture, texcoord).a;

    FragColor = outputColor;
}
