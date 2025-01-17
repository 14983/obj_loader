#version 330 core
in vec3 FragPos;
in vec3 Normal;
out vec4 FragColor;
uniform vec3 viewPos;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 objectAmbientColor;
uniform vec3 objectDiffuseColor;
uniform vec3 objectSpecularColor;
uniform float objectShininess;
void main() {
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    // ambient
    vec3 ambient = objectAmbientColor;
    // diffuse
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * objectDiffuseColor;
    // specular
    vec3 viewDir = normalize(viewPos-FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), objectShininess);
    vec3 specular = objectSpecularColor * spec;
    // combine results
    vec3 result = (ambient + diffuse + specular) * lightColor;
    FragColor = vec4(result, 1.0);
}
