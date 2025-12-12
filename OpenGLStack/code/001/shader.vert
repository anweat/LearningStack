#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNorm;

uniform mat4 MVP;   // 模型-视图-投影
uniform mat4 MV;    // 模型-视图（用于法线）
uniform mat3 N;     // 法线矩阵

out vec3 FragPos;   // 世界坐标
out vec3 Normal;    // 世界法线

void main()
{
    FragPos   = vec3(MV * vec4(aPos, 1.0));
    Normal    = N * aNorm;
    gl_Position = MVP * vec4(aPos, 1.0);
}
