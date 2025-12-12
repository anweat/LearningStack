#version 330 core
in  vec3 FragPos;
in  vec3 Normal;
out vec4 OutColor;

uniform vec3 lightPos = vec3(2,2,2);
uniform vec3 viewPos  = vec3(0,0,3);
uniform vec3 objColor = vec3(0.8,0.3,0.3);

void main()
{
    vec3 N = normalize(Normal);
    vec3 L = normalize(lightPos - FragPos);
    vec3 V = normalize(viewPos  - FragPos);
    vec3 R = reflect(-L, N);

    float diff = max(dot(N, L), 0.0);
    float spec = pow(max(dot(V, R), 0.0), 32);

    vec3 ambient  = 0.1 * objColor;
    vec3 diffuse  = diff * objColor;
    vec3 specular = spec * vec3(0.5);

    OutColor = vec4(ambient + diffuse + specular, 1.0);
}
