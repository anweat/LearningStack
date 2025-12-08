# 阶段 6: 引擎优化与高级特性

## 🎯 学习目标

- 实现延迟渲染（Deferred Rendering）
- 掌握各种阴影技术
- 实现屏幕空间反射（SSR）
- 优化渲染性能（剔除、LOD）
- 实现抗锯齿技术（MSAA、FXAA、TAA）
- 掌握性能分析和调试工具

## 📋 框架性问题指引

### 核心问题 1: 延迟渲染 vs 前向渲染？

**问题分解:**

- 前向渲染的瓶颈在哪？
- 延迟渲染如何解决多光源问题？
- 延迟渲染的缺点是什么？

**对比:**

```
前向渲染: 光源数量 × 物体数量
延迟渲染: 光源数量 × 屏幕像素
```

### 核心问题 2: 阴影技术有哪些？

**问题分解:**

- Shadow Mapping 的原理？
- CSM（级联阴影）解决什么问题？
- PCSS 和 VSM 的区别？

### 核心问题 3: 如何优化渲染性能？

**问题分解:**

- GPU 瓶颈还是 CPU 瓶颈？
- 视锥剔除和遮挡剔除？
- LOD 系统如何实现？

## 💻 核心概念详解

### 1. 延迟渲染

#### G-Buffer 设置

```cpp
class GBuffer {
public:
    GBuffer(int width, int height) {
        glGenFramebuffers(1, &m_FBO);
        glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);

        // 位置缓冲（RGB = 世界空间位置）
        glGenTextures(1, &m_PositionTexture);
        glBindTexture(GL_TEXTURE_2D, m_PositionTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_PositionTexture, 0);

        // 法线缓冲（RGB = 法线）
        glGenTextures(1, &m_NormalTexture);
        glBindTexture(GL_TEXTURE_2D, m_NormalTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_NormalTexture, 0);

        // 颜色缓冲（RGB = 反照率, A = 镜面强度）
        glGenTextures(1, &m_AlbedoSpecTexture);
        glBindTexture(GL_TEXTURE_2D, m_AlbedoSpecTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, m_AlbedoSpecTexture, 0);

        // 告诉OpenGL使用哪些颜色附件
        unsigned int attachments[3] = {
            GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2
        };
        glDrawBuffers(3, attachments);

        // 深度缓冲
        glGenRenderbuffers(1, &m_DepthRBO);
        glBindRenderbuffer(GL_RENDERBUFFER, m_DepthRBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_DepthRBO);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "GBuffer not complete!" << std::endl;

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void BindForWriting() {
        glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
    }

    void BindForReading() {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_PositionTexture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, m_NormalTexture);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, m_AlbedoSpecTexture);
    }

private:
    unsigned int m_FBO;
    unsigned int m_PositionTexture, m_NormalTexture, m_AlbedoSpecTexture;
    unsigned int m_DepthRBO;
};
```

#### 几何 Pass 着色器

```glsl
// vertex shader
#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    vec4 worldPos = model * vec4(aPos, 1.0);
    FragPos = worldPos.xyz;
    TexCoords = aTexCoords;

    mat3 normalMatrix = transpose(inverse(mat3(model)));
    Normal = normalMatrix * aNormal;

    gl_Position = projection * view * worldPos;
}

// fragment shader
#version 460 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;

void main() {
    // 存储世界空间位置
    gPosition = FragPos;

    // 存储法线
    gNormal = normalize(Normal);

    // 存储反照率和镜面反射强度
    gAlbedoSpec.rgb = texture(texture_diffuse1, TexCoords).rgb;
    gAlbedoSpec.a = texture(texture_specular1, TexCoords).r;
}
```

#### 光照 Pass 着色器

```glsl
#version 460 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;

struct Light {
    vec3 Position;
    vec3 Color;

    float Linear;
    float Quadratic;
};

const int NR_LIGHTS = 32;
uniform Light lights[NR_LIGHTS];
uniform vec3 viewPos;

void main() {
    // 从G-Buffer获取数据
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = texture(gNormal, TexCoords).rgb;
    vec3 Albedo = texture(gAlbedoSpec, TexCoords).rgb;
    float Specular = texture(gAlbedoSpec, TexCoords).a;

    // 计算光照
    vec3 lighting = Albedo * 0.1; // 环境光
    vec3 viewDir = normalize(viewPos - FragPos);

    for(int i = 0; i < NR_LIGHTS; ++i) {
        // 漫反射
        vec3 lightDir = normalize(lights[i].Position - FragPos);
        vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Albedo * lights[i].Color;

        // 镜面反射
        vec3 halfwayDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(Normal, halfwayDir), 0.0), 16.0);
        vec3 specular = lights[i].Color * spec * Specular;

        // 衰减
        float distance = length(lights[i].Position - FragPos);
        float attenuation = 1.0 / (1.0 + lights[i].Linear * distance +
                                    lights[i].Quadratic * distance * distance);

        diffuse *= attenuation;
        specular *= attenuation;

        lighting += diffuse + specular;
    }

    FragColor = vec4(lighting, 1.0);
}
```

#### 延迟渲染流程

```cpp
// 几何Pass
gBuffer.BindForWriting();
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

geometryShader.use();
// 渲染所有几何体
for (auto& object : scene.objects) {
    geometryShader.setMat4("model", object.transform);
    object.mesh->Draw();
}

// 光照Pass
glBindFramebuffer(GL_FRAMEBUFFER, 0);
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

lightingShader.use();
gBuffer.BindForReading();
lightingShader.setInt("gPosition", 0);
lightingShader.setInt("gNormal", 1);
lightingShader.setInt("gAlbedoSpec", 2);

// 设置光源
for (int i = 0; i < lights.size(); i++) {
    lightingShader.setVec3("lights[" + std::to_string(i) + "].Position", lights[i].position);
    lightingShader.setVec3("lights[" + std::to_string(i) + "].Color", lights[i].color);
    lightingShader.setFloat("lights[" + std::to_string(i) + "].Linear", 0.7f);
    lightingShader.setFloat("lights[" + std::to_string(i) + "].Quadratic", 1.8f);
}

renderQuad();

// 前向渲染透明物体
// ...
```

### 2. 阴影技术

#### Shadow Mapping

```cpp
class ShadowMap {
public:
    ShadowMap(int width, int height) : m_Width(width), m_Height(height) {
        glGenFramebuffers(1, &m_FBO);

        // 创建深度纹理
        glGenTextures(1, &m_DepthMap);
        glBindTexture(GL_TEXTURE_2D, m_DepthMap);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0,
                     GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

        // 绑定到帧缓冲
        glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_DepthMap, 0);
        glDrawBuffer(GL_NONE);  // 不需要颜色缓冲
        glReadBuffer(GL_NONE);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void BindForWriting() {
        glViewport(0, 0, m_Width, m_Height);
        glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
        glClear(GL_DEPTH_BUFFER_BIT);
    }

    void BindForReading(unsigned int textureUnit) {
        glActiveTexture(GL_TEXTURE0 + textureUnit);
        glBindTexture(GL_TEXTURE_2D, m_DepthMap);
    }

private:
    unsigned int m_FBO, m_DepthMap;
    int m_Width, m_Height;
};
```

#### 阴影 Pass 着色器

```glsl
// Depth shader (vertex)
#version 460 core
layout (location = 0) in vec3 aPos;

uniform mat4 lightSpaceMatrix;
uniform mat4 model;

void main() {
    gl_Position = lightSpaceMatrix * model * vec4(aPos, 1.0);
}

// Depth shader (fragment) - 空的，只需要深度
#version 460 core
void main() {}
```

#### 阴影接收着色器

```glsl
#version 460 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
} fs_in;

uniform sampler2D diffuseTexture;
uniform sampler2D shadowMap;
uniform vec3 lightPos;
uniform vec3 viewPos;

float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir) {
    // 透视除法
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

    // [- 1, 1] -> [0, 1]
    projCoords = projCoords * 0.5 + 0.5;

    // 获取最近深度值
    float closestDepth = texture(shadowMap, projCoords.xy).r;

    // 当前片段深度
    float currentDepth = projCoords.z;

    // 偏移消除阴影失真
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);

    // PCF软阴影
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x) {
        for(int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;

    // 超出阴影贴图范围的不在阴影中
    if(projCoords.z > 1.0)
        shadow = 0.0;

    return shadow;
}

void main() {
    vec3 color = texture(diffuseTexture, fs_in.TexCoords).rgb;
    vec3 normal = normalize(fs_in.Normal);
    vec3 lightColor = vec3(1.0);

    // 环境光
    vec3 ambient = 0.15 * color;

    // 漫反射
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * lightColor;

    // 镜面反射
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
    vec3 specular = spec * lightColor;

    // 计算阴影
    float shadow = ShadowCalculation(fs_in.FragPosLightSpace, normal, lightDir);
    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;

    FragColor = vec4(lighting, 1.0);
}
```

#### 级联阴影贴图（CSM）

```cpp
class CascadedShadowMap {
public:
    static const int CASCADE_COUNT = 4;

    struct Cascade {
        float splitDepth;
        glm::mat4 viewProjMatrix;
        unsigned int depthMap;
    };

    CascadedShadowMap(int shadowMapSize) {
        m_ShadowMapSize = shadowMapSize;

        // 创建帧缓冲
        glGenFramebuffers(1, &m_FBO);

        // 为每个级联创建深度纹理
        for (int i = 0; i < CASCADE_COUNT; i++) {
            glGenTextures(1, &m_Cascades[i].depthMap);
            glBindTexture(GL_TEXTURE_2D, m_Cascades[i].depthMap);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, shadowMapSize, shadowMapSize,
                         0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
            float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
            glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
        }
    }

    void UpdateCascades(const Camera& camera, const glm::vec3& lightDir) {
        float cascadeSplits[CASCADE_COUNT] = { 0.1f, 0.25f, 0.5f, 1.0f };

        float nearClip = camera.nearPlane;
        float farClip = camera.farPlane;
        float clipRange = farClip - nearClip;

        for (int i = 0; i < CASCADE_COUNT; i++) {
            float splitNear = (i == 0) ? nearClip : nearClip + cascadeSplits[i-1] * clipRange;
            float splitFar = nearClip + cascadeSplits[i] * clipRange;

            m_Cascades[i].splitDepth = splitFar;

            // 计算视锥体的8个角点
            glm::mat4 proj = glm::perspective(camera.fov, camera.aspect, splitNear, splitFar);
            glm::mat4 invCam = glm::inverse(proj * camera.GetViewMatrix());

            std::vector<glm::vec4> frustumCorners;
            for (int x = 0; x < 2; x++) {
                for (int y = 0; y < 2; y++) {
                    for (int z = 0; z < 2; z++) {
                        glm::vec4 pt = invCam * glm::vec4(
                            2.0f * x - 1.0f,
                            2.0f * y - 1.0f,
                            2.0f * z - 1.0f,
                            1.0f
                        );
                        frustumCorners.push_back(pt / pt.w);
                    }
                }
            }

            // 计算视锥体中心
            glm::vec3 center = glm::vec3(0.0f);
            for (const auto& v : frustumCorners) {
                center += glm::vec3(v);
            }
            center /= frustumCorners.size();

            // 构建光源视图矩阵
            glm::mat4 lightView = glm::lookAt(center - lightDir, center, glm::vec3(0.0f, 1.0f, 0.0f));

            // 计算光源空间中的AABB
            float minX = std::numeric_limits<float>::max();
            float maxX = std::numeric_limits<float>::lowest();
            float minY = std::numeric_limits<float>::max();
            float maxY = std::numeric_limits<float>::lowest();
            float minZ = std::numeric_limits<float>::max();
            float maxZ = std::numeric_limits<float>::lowest();

            for (const auto& v : frustumCorners) {
                glm::vec4 trf = lightView * v;
                minX = std::min(minX, trf.x);
                maxX = std::max(maxX, trf.x);
                minY = std::min(minY, trf.y);
                maxY = std::max(maxY, trf.y);
                minZ = std::min(minZ, trf.z);
                maxZ = std::max(maxZ, trf.z);
            }

            // 构建正交投影矩阵
            glm::mat4 lightProjection = glm::ortho(minX, maxX, minY, maxY, minZ, maxZ);

            m_Cascades[i].viewProjMatrix = lightProjection * lightView;
        }
    }

    Cascade m_Cascades[CASCADE_COUNT];

private:
    unsigned int m_FBO;
    int m_ShadowMapSize;
};
```

### 3. 屏幕空间反射（SSR）

```glsl
#version 460 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;
uniform sampler2D depthMap;

uniform mat4 projection;
uniform mat4 view;
uniform vec3 cameraPos;

vec3 SSR(vec3 position, vec3 reflection, float maxDistance, int maxSteps) {
    vec3 step = reflection * (maxDistance / float(maxSteps));
    vec3 marchingPos = position;

    for(int i = 0; i < maxSteps; i++) {
        marchingPos += step;

        // 投影到屏幕空间
        vec4 projectedCoord = projection * view * vec4(marchingPos, 1.0);
        projectedCoord.xy /= projectedCoord.w;
        projectedCoord.xy = projectedCoord.xy * 0.5 + 0.5;

        // 采样深度
        float depth = texture(gPosition, projectedCoord.xy).z;

        // 检查碰撞
        if(abs(marchingPos.z - depth) < 0.01) {
            return texture(gAlbedoSpec, projectedCoord.xy).rgb;
        }
    }

    return vec3(0.0);  // 未找到反射
}

void main() {
    vec3 position = texture(gPosition, TexCoords).xyz;
    vec3 normal = texture(gNormal, TexCoords).xyz;
    vec3 albedo = texture(gAlbedoSpec, TexCoords).rgb;
    float specular = texture(gAlbedoSpec, TexCoords).a;

    vec3 viewDir = normalize(cameraPos - position);
    vec3 reflection = reflect(-viewDir, normal);

    vec3 ssrColor = SSR(position, reflection, 10.0, 50);

    FragColor = vec4(mix(albedo, ssrColor, specular), 1.0);
}
```

### 4. 性能优化技术

#### 视锥剔除

```cpp
class Frustum {
public:
    void Update(const glm::mat4& viewProj) {
        // 提取视锥体的6个平面
        for (int i = 0; i < 4; i++)
            m_Planes[0][i] = viewProj[i][3] + viewProj[i][0]; // Left
        for (int i = 0; i < 4; i++)
            m_Planes[1][i] = viewProj[i][3] - viewProj[i][0]; // Right
        for (int i = 0; i < 4; i++)
            m_Planes[2][i] = viewProj[i][3] + viewProj[i][1]; // Bottom
        for (int i = 0; i < 4; i++)
            m_Planes[3][i] = viewProj[i][3] - viewProj[i][1]; // Top
        for (int i = 0; i < 4; i++)
            m_Planes[4][i] = viewProj[i][3] + viewProj[i][2]; // Near
        for (int i = 0; i < 4; i++)
            m_Planes[5][i] = viewProj[i][3] - viewProj[i][2]; // Far

        // 归一化
        for (int i = 0; i < 6; i++) {
            float length = glm::length(glm::vec3(m_Planes[i]));
            m_Planes[i] /= length;
        }
    }

    bool IsBoxVisible(const glm::vec3& minp, const glm::vec3& maxp) const {
        for (int i = 0; i < 6; i++) {
            glm::vec3 vmin = minp;
            glm::vec3 vmax = maxp;

            if (m_Planes[i].x > 0) {
                vmin.x = maxp.x;
                vmax.x = minp.x;
            }
            if (m_Planes[i].y > 0) {
                vmin.y = maxp.y;
                vmax.y = minp.y;
            }
            if (m_Planes[i].z > 0) {
                vmin.z = maxp.z;
                vmax.z = minp.z;
            }

            if (glm::dot(glm::vec3(m_Planes[i]), vmin) + m_Planes[i].w < 0)
                return false;
        }
        return true;
    }

private:
    glm::vec4 m_Planes[6];
};

// 使用
Frustum frustum;
frustum.Update(projection * view);

for (auto& object : objects) {
    if (frustum.IsBoxVisible(object.aabb.min, object.aabb.max)) {
        // 渲染对象
    }
}
```

#### LOD 系统

```cpp
class LODMesh {
public:
    struct LODLevel {
        std::shared_ptr<Mesh> mesh;
        float distance;  // 切换距离
    };

    void AddLOD(std::shared_ptr<Mesh> mesh, float distance) {
        m_LODs.push_back({mesh, distance});

        // 按距离排序
        std::sort(m_LODs.begin(), m_LODs.end(),
            [](const LODLevel& a, const LODLevel& b) {
                return a.distance < b.distance;
            });
    }

    Mesh* GetLOD(float distanceToCamera) const {
        for (const auto& lod : m_LODs) {
            if (distanceToCamera < lod.distance) {
                return lod.mesh.get();
            }
        }
        // 返回最低LOD
        return m_LODs.empty() ? nullptr : m_LODs.back().mesh.get();
    }

private:
    std::vector<LODLevel> m_LODs;
};

// 使用
LODMesh lodMesh;
lodMesh.AddLOD(highDetailMesh, 10.0f);
lodMesh.AddLOD(mediumDetailMesh, 50.0f);
lodMesh.AddLOD(lowDetailMesh, 200.0f);

float distance = glm::distance(object.position, camera.position);
Mesh* mesh = lodMesh.GetLOD(distance);
mesh->Draw();
```

#### 遮挡剔除（硬件查询）

```cpp
class OcclusionQuery {
public:
    OcclusionQuery() {
        glGenQueries(1, &m_QueryID);
    }

    ~OcclusionQuery() {
        glDeleteQueries(1, &m_QueryID);
    }

    void Begin() {
        glBeginQuery(GL_ANY_SAMPLES_PASSED, m_QueryID);
    }

    void End() {
        glEndQuery(GL_ANY_SAMPLES_PASSED);
    }

    bool IsVisible() {
        GLuint result;
        glGetQueryObjectuiv(m_QueryID, GL_QUERY_RESULT, &result);
        return result > 0;
    }

private:
    unsigned int m_QueryID;
};

// 使用（两帧延迟）
for (auto& object : objects) {
    // 渲染边界框进行查询
    object.query.Begin();
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glDepthMask(GL_FALSE);
    RenderBoundingBox(object.aabb);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthMask(GL_TRUE);
    object.query.End();

    // 使用上一帧的查询结果
    if (object.wasVisible) {
        object.mesh->Draw();
    }

    object.wasVisible = object.query.IsVisible();
}
```

### 5. 抗锯齿技术

#### MSAA（多重采样）

```cpp
// 创建MSAA帧缓冲
glGenFramebuffers(1, &msaaFBO);
glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);

// 创建多重采样纹理
glGenTextures(1, &msaaTexture);
glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msaaTexture);
glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGB, width, height, GL_TRUE);
glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, msaaTexture, 0);

// 渲染到MSAA缓冲
glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);
// ... 渲染

// Blit到屏幕
glBindFramebuffer(GL_READ_FRAMEBUFFER, msaaFBO);
glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
```

#### FXAA（快速近似抗锯齿）

```glsl
#version 460 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform vec2 inverseScreenSize;

#define FXAA_SPAN_MAX 8.0
#define FXAA_REDUCE_MUL (1.0 / FXAA_SPAN_MAX)
#define FXAA_REDUCE_MIN (1.0 / 128.0)
#define FXAA_SUBPIX_SHIFT (1.0 / 4.0)

vec3 FxaaPixelShader(vec2 pos, sampler2D tex, vec2 rcpFrame) {
    vec3 rgbNW = texture(tex, pos + vec2(-1.0, -1.0) * rcpFrame).xyz;
    vec3 rgbNE = texture(tex, pos + vec2(1.0, -1.0) * rcpFrame).xyz;
    vec3 rgbSW = texture(tex, pos + vec2(-1.0, 1.0) * rcpFrame).xyz;
    vec3 rgbSE = texture(tex, pos + vec2(1.0, 1.0) * rcpFrame).xyz;
    vec3 rgbM = texture(tex, pos).xyz;

    vec3 luma = vec3(0.299, 0.587, 0.114);
    float lumaNW = dot(rgbNW, luma);
    float lumaNE = dot(rgbNE, luma);
    float lumaSW = dot(rgbSW, luma);
    float lumaSE = dot(rgbSE, luma);
    float lumaM = dot(rgbM, luma);

    float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
    float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));

    vec2 dir;
    dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
    dir.y = ((lumaNW + lumaSW) - (lumaNE + lumaSE));

    float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 * FXAA_REDUCE_MUL), FXAA_REDUCE_MIN);
    float rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);

    dir = min(vec2(FXAA_SPAN_MAX), max(vec2(-FXAA_SPAN_MAX), dir * rcpDirMin)) * rcpFrame;

    vec3 rgbA = 0.5 * (
        texture(tex, pos + dir * (1.0/3.0 - 0.5)).xyz +
        texture(tex, pos + dir * (2.0/3.0 - 0.5)).xyz);

    vec3 rgbB = rgbA * 0.5 + 0.25 * (
        texture(tex, pos + dir * -0.5).xyz +
        texture(tex, pos + dir * 0.5).xyz);

    float lumaB = dot(rgbB, luma);

    if((lumaB < lumaMin) || (lumaB > lumaMax)) {
        return rgbA;
    } else {
        return rgbB;
    }
}

void main() {
    FragColor = vec4(FxaaPixelShader(TexCoords, screenTexture, inverseScreenSize), 1.0);
}
```

### 6. 性能分析与调试

#### GPU 计时查询

```cpp
class GPUTimer {
public:
    GPUTimer() {
        glGenQueries(2, m_Queries);
    }

    ~GPUTimer() {
        glDeleteQueries(2, m_Queries);
    }

    void Begin() {
        glQueryCounter(m_Queries[0], GL_TIMESTAMP);
    }

    void End() {
        glQueryCounter(m_Queries[1], GL_TIMESTAMP);
    }

    float GetTimeMS() {
        GLuint64 startTime, endTime;
        glGetQueryObjectui64v(m_Queries[0], GL_QUERY_RESULT, &startTime);
        glGetQueryObjectui64v(m_Queries[1], GL_QUERY_RESULT, &endTime);
        return (endTime - startTime) / 1000000.0f;  // 转换为毫秒
    }

private:
    unsigned int m_Queries[2];
};

// 使用
GPUTimer timer;
timer.Begin();
// ... 渲染代码
timer.End();
std::cout << "GPU Time: " << timer.GetTimeMS() << "ms" << std::endl;
```

#### 性能分析器集成

```cpp
class Profiler {
public:
    struct ProfileResult {
        std::string name;
        float timeMS;
    };

    void BeginProfile(const std::string& name) {
        m_CurrentName = name;
        m_StartTime = std::chrono::high_resolution_clock::now();
    }

    void EndProfile() {
        auto endTime = std::chrono::high_resolution_clock::now();
        float duration = std::chrono::duration<float, std::milli>(endTime - m_StartTime).count();
        m_Results[m_CurrentName] = duration;
    }

    void DisplayResults() {
        for (const auto& [name, time] : m_Results) {
            std::cout << name << ": " << time << "ms" << std::endl;
        }
    }

private:
    std::string m_CurrentName;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_StartTime;
    std::unordered_map<std::string, float> m_Results;
};

// 使用RAII自动分析
class ScopedProfile {
public:
    ScopedProfile(Profiler& profiler, const std::string& name)
        : m_Profiler(profiler) {
        m_Profiler.BeginProfile(name);
    }

    ~ScopedProfile() {
        m_Profiler.EndProfile();
    }

private:
    Profiler& m_Profiler;
};

// 宏简化使用
#define PROFILE_SCOPE(name) ScopedProfile profiler##__LINE__(g_Profiler, name)

// 使用
Profiler g_Profiler;

void RenderScene() {
    PROFILE_SCOPE("Scene Rendering");
    // ... 渲染代码
}
```

## 💡 常见问题解答

### Q1: 延迟渲染不支持透明物体怎么办？

**答:** 先用延迟渲染渲染不透明物体，再用前向渲染渲染透明物体。

### Q2: 如何选择合适的阴影技术？

- **移动端**: 简单 Shadow Mapping
- **PC**: CSM + PCF/PCSS
- **高端**: Ray-traced shadows（需要 RTX）

### Q3: 性能优化的优先级？

1. 算法优化（剔除、LOD）
2. 减少绘制调用（批处理、实例化）
3. 着色器优化
4. 纹理压缩

## 📝 学习检查清单

- [ ] 实现延迟渲染系统
- [ ] 掌握 Shadow Mapping 和 CSM
- [ ] 实现屏幕空间效果（SSR、SSAO）
- [ ] 实现视锥和遮挡剔除
- [ ] 掌握 LOD 系统
- [ ] 了解各种抗锯齿技术
- [ ] 使用性能分析工具

## 🎓 课程总结

恭喜完成全部 6 个阶段的学习！你已经掌握了：

1. **基础知识**: OpenGL API、渲染管线、着色器
2. **渲染技术**: 光照、纹理、变换、模型加载
3. **高级特性**: PBR、后处理、环境映射
4. **引擎架构**: 资源管理、ECS、渲染队列
5. **性能优化**: 延迟渲染、阴影、剔除、LOD

## 🚀 下一步方向

### 继续深入

- 学习现代图形 API（Vulkan、DirectX 12）
- 研究实时光线追踪
- 探索虚拟现实渲染
- 学习计算机图形学理论

### 实践项目

- 开发完整的游戏引擎
- 实现高级渲染效果（体积雾、全局光照）
- 参与开源图形项目
- 制作个人作品集

---

**祝贺你完成 OpenGL 学习之旅！** 🎉🎊
