# 阶段 4: 着色器编程进阶

## 🎯 学习目标

- 深入理解 GLSL 着色器语言
- 实现各种后处理效果
- 掌握 PBR（基于物理的渲染）
- 理解天空盒和环境映射
- 实现几何着色器和细分着色器
- 掌握计算着色器基础

## 📋 框架性问题指引

### 核心问题 1: GLSL 的内存模型是什么？

**问题分解:**

- Uniform、Attribute、Varying 的区别？
- Uniform Buffer Object 的优势？
- Shader Storage Buffer Object 用于什么场景？

### 核心问题 2: PBR 为什么更真实？

**问题分解:**

- 传统光照模型的局限性？
- PBR 的物理基础是什么？
- 金属度和粗糙度如何影响渲染？

**PBR 核心要素:**

```
BRDF = 漫反射项 + 镜面反射项（菲涅尔F + 几何G + 分布D）
```

### 核心问题 3: 几何着色器能做什么？

**问题分解:**

- 几何着色器在管线中的位置？
- 如何生成新的图元？
- 典型应用场景有哪些？

## 💻 核心概念详解

### 1. GLSL 高级特性

#### Uniform Buffer Object (UBO)

多个着色器共享 uniform 数据，减少 API 调用。

```cpp
// C++端设置UBO
unsigned int uniformBlockIndex = glGetUniformBlockIndex(shaderProgram, "Matrices");
glUniformBlockBinding(shaderProgram, uniformBlockIndex, 0);

unsigned int uboMatrices;
glGenBuffers(1, &uboMatrices);
glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
glBindBufferRange(GL_UNIFORM_BUFFER, 0, uboMatrices, 0, 2 * sizeof(glm::mat4));

// 更新数据
glm::mat4 projection = glm::perspective(glm::radians(45.0f), ratio, 0.1f, 100.0f);
glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
glBindBuffer(GL_UNIFORM_BUFFER, 0);
```

```glsl
// GLSL端使用UBO
#version 460 core
layout (std140) uniform Matrices {
    mat4 projection;
    mat4 view;
};

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
```

#### Shader Storage Buffer Object (SSBO)

可读写的大容量缓冲区，用于 GPU 计算。

```cpp
// C++端
struct Particle {
    glm::vec4 position;
    glm::vec4 velocity;
};

std::vector<Particle> particles(1000);

unsigned int ssbo;
glGenBuffers(1, &ssbo);
glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
glBufferData(GL_SHADER_STORAGE_BUFFER, particles.size() * sizeof(Particle),
             particles.data(), GL_DYNAMIC_DRAW);
glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);
```

```glsl
// GLSL端
#version 460 core
layout (std430, binding = 0) buffer ParticleBuffer {
    vec4 positions[];
    vec4 velocities[];
};

void main() {
    positions[gl_VertexID] += velocities[gl_VertexID] * deltaTime;
}
```

### 2. 后处理效果

#### 帧缓冲设置

```cpp
class Framebuffer {
public:
    unsigned int FBO, texture, RBO;

    Framebuffer(int width, int height) {
        // 创建帧缓冲
        glGenFramebuffers(1, &FBO);
        glBindFramebuffer(GL_FRAMEBUFFER, FBO);

        // 创建纹理附件
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

        // 创建渲染缓冲对象（深度+模板）
        glGenRenderbuffers(1, &RBO);
        glBindRenderbuffer(GL_RENDERBUFFER, RBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);

        // 检查完整性
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void Bind() {
        glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    }

    void Unbind() {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
};

// 使用
Framebuffer fb(800, 600);

// 第一遍：渲染到帧缓冲
fb.Bind();
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
// ... 正常渲染场景
fb.Unbind();

// 第二遍：后处理
glClear(GL_COLOR_BUFFER_BIT);
postProcessShader.use();
glBindTexture(GL_TEXTURE_2D, fb.texture);
// 渲染全屏四边形
renderQuad();
```

#### 后处理着色器示例

```glsl
// 反色效果
#version 460 core
out vec4 FragColor;
in vec2 TexCoords;
uniform sampler2D screenTexture;

void main() {
    FragColor = vec4(vec3(1.0 - texture(screenTexture, TexCoords)), 1.0);
}

// 灰度效果
void main() {
    vec3 color = texture(screenTexture, TexCoords).rgb;
    float average = (color.r + color.g + color.b) / 3.0;
    FragColor = vec4(vec3(average), 1.0);
}

// 核效果（模糊、锐化等）
const float offset = 1.0 / 300.0;
vec2 offsets[9] = vec2[](
    vec2(-offset,  offset), vec2( 0.0f,    offset), vec2( offset,  offset),
    vec2(-offset,  0.0f),   vec2( 0.0f,    0.0f),   vec2( offset,  0.0f),
    vec2(-offset, -offset), vec2( 0.0f,   -offset), vec2( offset, -offset)
);

// 锐化核
float kernel[9] = float[](
    -1, -1, -1,
    -1,  9, -1,
    -1, -1, -1
);

// 模糊核
float kernel[9] = float[](
    1.0 / 16, 2.0 / 16, 1.0 / 16,
    2.0 / 16, 4.0 / 16, 2.0 / 16,
    1.0 / 16, 2.0 / 16, 1.0 / 16
);

void main() {
    vec3 sampleTex[9];
    for(int i = 0; i < 9; i++)
        sampleTex[i] = vec3(texture(screenTexture, TexCoords.st + offsets[i]));

    vec3 col = vec3(0.0);
    for(int i = 0; i < 9; i++)
        col += sampleTex[i] * kernel[i];

    FragColor = vec4(col, 1.0);
}

// 边缘检测
float kernel[9] = float[](
    1,  1, 1,
    1, -8, 1,
    1,  1, 1
);
```

#### HDR 和泛光效果

```glsl
// HDR色调映射
#version 460 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D hdrBuffer;
uniform float exposure;

void main() {
    vec3 hdrColor = texture(hdrBuffer, TexCoords).rgb;

    // Reinhard色调映射
    vec3 mapped = hdrColor / (hdrColor + vec3(1.0));

    // 曝光色调映射
    // vec3 mapped = vec3(1.0) - exp(-hdrColor * exposure);

    // Gamma校正
    mapped = pow(mapped, vec3(1.0 / 2.2));

    FragColor = vec4(mapped, 1.0);
}

// 泛光效果（高斯模糊）
#version 460 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D image;
uniform bool horizontal;
uniform float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main() {
    vec2 tex_offset = 1.0 / textureSize(image, 0);
    vec3 result = texture(image, TexCoords).rgb * weight[0];

    if(horizontal) {
        for(int i = 1; i < 5; ++i) {
            result += texture(image, TexCoords + vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
            result += texture(image, TexCoords - vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
        }
    } else {
        for(int i = 1; i < 5; ++i) {
            result += texture(image, TexCoords + vec2(0.0, tex_offset.y * i)).rgb * weight[i];
            result += texture(image, TexCoords - vec2(0.0, tex_offset.y * i)).rgb * weight[i];
        }
    }
    FragColor = vec4(result, 1.0);
}
```

### 3. PBR（基于物理的渲染）

#### PBR 理论基础

**Cook-Torrance BRDF:**

```glsl
vec3 BRDF(vec3 L, vec3 V, vec3 N, vec3 albedo, float metallic, float roughness) {
    vec3 H = normalize(V + L);

    // 菲涅尔项（Fresnel）
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

    // 法线分布函数（Normal Distribution Function）
    float NDF = DistributionGGX(N, H, roughness);

    // 几何遮蔽函数（Geometry Function）
    float G = GeometrySmith(N, V, L, roughness);

    // Cook-Torrance BRDF
    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;

    // 能量守恒
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    float NdotL = max(dot(N, L), 0.0);
    return (kD * albedo / PI + specular) * NdotL;
}
```

#### PBR 核心函数

```glsl
const float PI = 3.14159265359;

// Trowbridge-Reitz GGX法线分布函数
float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

// Smith几何遮蔽函数
float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

// Fresnel-Schlick近似
vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// 带粗糙度的Fresnel
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
```

#### 完整 PBR 着色器

```glsl
#version 460 core
out vec4 FragColor;

in vec3 WorldPos;
in vec3 Normal;
in vec2 TexCoords;

// 材质参数
uniform sampler2D albedoMap;
uniform sampler2D normalMap;
uniform sampler2D metallicMap;
uniform sampler2D roughnessMap;
uniform sampler2D aoMap;

// 光源
uniform vec3 lightPositions[4];
uniform vec3 lightColors[4];
uniform vec3 camPos;

void main() {
    // 材质属性
    vec3 albedo = pow(texture(albedoMap, TexCoords).rgb, vec3(2.2));  // sRGB -> 线性
    float metallic = texture(metallicMap, TexCoords).r;
    float roughness = texture(roughnessMap, TexCoords).r;
    float ao = texture(aoMap, TexCoords).r;

    vec3 N = getNormalFromMap();  // 从法线贴图获取法线
    vec3 V = normalize(camPos - WorldPos);

    // 计算反射率
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    // 光照累加
    vec3 Lo = vec3(0.0);
    for(int i = 0; i < 4; ++i) {
        // 逐光源计算辐射度
        vec3 L = normalize(lightPositions[i] - WorldPos);
        vec3 H = normalize(V + L);
        float distance = length(lightPositions[i] - WorldPos);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = lightColors[i] * attenuation;

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
        vec3 specular = numerator / denominator;

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;

        float NdotL = max(dot(N, L), 0.0);
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }

    // 环境光
    vec3 ambient = vec3(0.03) * albedo * ao;
    vec3 color = ambient + Lo;

    // HDR色调映射
    color = color / (color + vec3(1.0));
    // Gamma校正
    color = pow(color, vec3(1.0/2.2));

    FragColor = vec4(color, 1.0);
}
```

### 4. 天空盒与环境映射

#### 立方体贴图加载

```cpp
unsigned int loadCubemap(std::vector<std::string> faces) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++) {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB,
                         width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        } else {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

// 使用
std::vector<std::string> faces {
    "right.jpg", "left.jpg",
    "top.jpg", "bottom.jpg",
    "front.jpg", "back.jpg"
};
unsigned int cubemapTexture = loadCubemap(faces);
```

#### 天空盒着色器

```glsl
// vertex shader
#version 460 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 projection;
uniform mat4 view;

void main() {
    TexCoords = aPos;
    vec4 pos = projection * view * vec4(aPos, 1.0);
    gl_Position = pos.xyww;  // 确保深度始终为1.0
}

// fragment shader
#version 460 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube skybox;

void main() {
    FragColor = texture(skybox, TexCoords);
}
```

#### 环境映射（反射/折射）

```glsl
// 反射
#version 460 core
out vec4 FragColor;

in vec3 Normal;
in vec3 Position;

uniform vec3 cameraPos;
uniform samplerCube skybox;

void main() {
    vec3 I = normalize(Position - cameraPos);
    vec3 R = reflect(I, normalize(Normal));
    FragColor = vec4(texture(skybox, R).rgb, 1.0);
}

// 折射
void main() {
    float ratio = 1.00 / 1.52;  // 空气/玻璃折射率
    vec3 I = normalize(Position - cameraPos);
    vec3 R = refract(I, normalize(Normal), ratio);
    FragColor = vec4(texture(skybox, R).rgb, 1.0);
}
```

### 5. 几何着色器

#### 法线可视化

```glsl
// vertex shader
#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out VS_OUT {
    vec3 normal;
} vs_out;

uniform mat4 view;
uniform mat4 model;

void main() {
    gl_Position = view * model * vec4(aPos, 1.0);
    mat3 normalMatrix = mat3(transpose(inverse(view * model)));
    vs_out.normal = normalize(vec3(vec4(normalMatrix * aNormal, 0.0)));
}

// geometry shader
#version 460 core
layout (triangles) in;
layout (line_strip, max_vertices = 6) out;

in VS_OUT {
    vec3 normal;
} gs_in[];

const float MAGNITUDE = 0.4;
uniform mat4 projection;

void GenerateLine(int index) {
    gl_Position = projection * gl_in[index].gl_Position;
    EmitVertex();
    gl_Position = projection * (gl_in[index].gl_Position +
                                vec4(gs_in[index].normal, 0.0) * MAGNITUDE);
    EmitVertex();
    EndPrimitive();
}

void main() {
    GenerateLine(0);  // 第一个顶点法线
    GenerateLine(1);  // 第二个顶点法线
    GenerateLine(2);  // 第三个顶点法线
}
```

#### 粒子生成

```glsl
// geometry shader - 爆炸效果
#version 460 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in VS_OUT {
    vec2 texCoords;
} gs_in[];

out vec2 TexCoords;

uniform float time;

vec4 explode(vec4 position, vec3 normal) {
    float magnitude = 2.0;
    vec3 direction = normal * ((sin(time) + 1.0) / 2.0) * magnitude;
    return position + vec4(direction, 0.0);
}

vec3 GetNormal() {
    vec3 a = vec3(gl_in[0].gl_Position) - vec3(gl_in[1].gl_Position);
    vec3 b = vec3(gl_in[2].gl_Position) - vec3(gl_in[1].gl_Position);
    return normalize(cross(a, b));
}

void main() {
    vec3 normal = GetNormal();

    gl_Position = explode(gl_in[0].gl_Position, normal);
    TexCoords = gs_in[0].texCoords;
    EmitVertex();

    gl_Position = explode(gl_in[1].gl_Position, normal);
    TexCoords = gs_in[1].texCoords;
    EmitVertex();

    gl_Position = explode(gl_in[2].gl_Position, normal);
    TexCoords = gs_in[2].texCoords;
    EmitVertex();

    EndPrimitive();
}
```

### 6. 计算着色器入门

#### 粒子系统

```glsl
#version 460 core
layout (local_size_x = 256) in;

struct Particle {
    vec4 position;
    vec4 velocity;
    vec4 color;
    float life;
};

layout (std430, binding = 0) buffer ParticleBuffer {
    Particle particles[];
};

uniform float deltaTime;
uniform vec3 gravity;

void main() {
    uint id = gl_GlobalInvocationID.x;

    if (particles[id].life > 0.0) {
        // 更新速度
        particles[id].velocity.xyz += gravity * deltaTime;

        // 更新位置
        particles[id].position.xyz += particles[id].velocity.xyz * deltaTime;

        // 更新生命值
        particles[id].life -= deltaTime;

        // 淡出效果
        particles[id].color.a = particles[id].life;
    }
}
```

```cpp
// C++端分派
shader.use();
shader.setFloat("deltaTime", deltaTime);
shader.setVec3("gravity", glm::vec3(0.0f, -9.8f, 0.0f));

glDispatchCompute(numParticles / 256, 1, 1);
glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
```

## 💡 常见问题解答

### Q1: PBR 和传统光照的主要区别？

- **PBR**: 基于物理，遵循能量守恒，在不同光照条件下表现一致
- **传统**: 经验模型，缺乏物理基础，需要大量调参

### Q2: 何时使用几何着色器？

- 动态生成几何体（草地、毛发）
- 粒子效果
- 法线可视化
- 注意：性能开销较大，谨慎使用

### Q3: 计算着色器 vs 传统管线？

- **计算着色器**: 通用 GPU 计算，不限于图形渲染
- **传统管线**: 专门用于图形渲染

## 📝 学习检查清单

- [ ] 掌握 UBO 和 SSBO 的使用
- [ ] 实现多种后处理效果
- [ ] 理解 PBR 理论和实现
- [ ] 实现天空盒和环境映射
- [ ] 编写几何着色器
- [ ] 了解计算着色器基础

## 🚀 下一步

进入 [阶段 5: 渲染引擎架构](./Stage05_Engine_Architecture.md)
