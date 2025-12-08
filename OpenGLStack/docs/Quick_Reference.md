# OpenGL 快速参考手册

## 🚀 常用 API 速查

### 窗口与上下文

```cpp
// GLFW初始化
glfwInit();
glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

// 创建窗口
GLFWwindow* window = glfwCreateWindow(800, 600, "Title", nullptr, nullptr);
glfwMakeContextCurrent(window);

// GLAD加载
gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

// 主循环
while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    glfwSwapBuffers(window);
}

// 清理
glfwTerminate();
```

### 缓冲对象

```cpp
// VBO (顶点缓冲对象)
unsigned int VBO;
glGenBuffers(1, &VBO);
glBindBuffer(GL_ARRAY_BUFFER, VBO);
glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);

// VAO (顶点数组对象)
unsigned int VAO;
glGenVertexArrays(1, &VAO);
glBindVertexArray(VAO);

// 配置顶点属性
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)offset);
glEnableVertexAttribArray(0);

// EBO (索引缓冲对象)
unsigned int EBO;
glGenBuffers(1, &EBO);
glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, indices, GL_STATIC_DRAW);

// UBO (统一缓冲对象)
unsigned int UBO;
glGenBuffers(1, &UBO);
glBindBuffer(GL_UNIFORM_BUFFER, UBO);
glBufferData(GL_UNIFORM_BUFFER, size, data, GL_STATIC_DRAW);
glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, UBO);

// 清理
glDeleteBuffers(1, &VBO);
glDeleteVertexArrays(1, &VAO);
```

### 着色器

```cpp
// 编译着色器
unsigned int shader = glCreateShader(GL_VERTEX_SHADER);
glShaderSource(shader, 1, &source, nullptr);
glCompileShader(shader);

// 检查编译错误
int success;
char infoLog[512];
glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
if (!success) {
    glGetShaderInfoLog(shader, 512, nullptr, infoLog);
}

// 链接程序
unsigned int program = glCreateProgram();
glAttachShader(program, vertexShader);
glAttachShader(program, fragmentShader);
glLinkProgram(program);

// 使用程序
glUseProgram(program);

// 设置Uniform
glUniform1i(location, value);           // int
glUniform1f(location, value);           // float
glUniform3f(location, x, y, z);         // vec3
glUniform3fv(location, 1, &vec[0]);     // vec3 (数组)
glUniformMatrix4fv(location, 1, GL_FALSE, &mat[0][0]);  // mat4

// 获取Uniform位置
int location = glGetUniformLocation(program, "uniformName");

// 清理
glDeleteShader(shader);
glDeleteProgram(program);
```

### 纹理

```cpp
// 2D纹理
unsigned int texture;
glGenTextures(1, &texture);
glBindTexture(GL_TEXTURE_2D, texture);

// 加载图像数据
glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
glGenerateMipmap(GL_TEXTURE_2D);

// 纹理参数
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

// 激活纹理单元
glActiveTexture(GL_TEXTURE0);
glBindTexture(GL_TEXTURE_2D, texture);

// 立方体贴图
glGenTextures(1, &cubemap);
glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);
for (int i = 0; i < 6; i++) {
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB,
                 width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data[i]);
}

// 清理
glDeleteTextures(1, &texture);
```

### 帧缓冲

```cpp
// 创建帧缓冲
unsigned int FBO;
glGenFramebuffers(1, &FBO);
glBindFramebuffer(GL_FRAMEBUFFER, FBO);

// 纹理附件
unsigned int texColorBuffer;
glGenTextures(1, &texColorBuffer);
glBindTexture(GL_TEXTURE_2D, texColorBuffer);
glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texColorBuffer, 0);

// 渲染缓冲对象（深度+模板）
unsigned int RBO;
glGenRenderbuffers(1, &RBO);
glBindRenderbuffer(GL_RENDERBUFFER, RBO);
glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);

// 检查完整性
if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    // 错误处理
}

// 绑定
glBindFramebuffer(GL_FRAMEBUFFER, FBO);  // 渲染到FBO
glBindFramebuffer(GL_FRAMEBUFFER, 0);     // 渲染到默认帧缓冲

// 清理
glDeleteFramebuffers(1, &FBO);
glDeleteRenderbuffers(1, &RBO);
```

### 绘制命令

```cpp
// 清屏
glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

// 绘制数组
glDrawArrays(GL_TRIANGLES, 0, vertexCount);

// 绘制索引
glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);

// 实例化绘制
glDrawArraysInstanced(GL_TRIANGLES, 0, vertexCount, instanceCount);
glDrawElementsInstanced(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0, instanceCount);

// 图元类型
GL_POINTS, GL_LINES, GL_LINE_STRIP, GL_LINE_LOOP,
GL_TRIANGLES, GL_TRIANGLE_STRIP, GL_TRIANGLE_FAN
```

### 状态设置

```cpp
// 深度测试
glEnable(GL_DEPTH_TEST);
glDepthFunc(GL_LESS);  // GL_ALWAYS, GL_NEVER, GL_LESS, GL_EQUAL, GL_LEQUAL, GL_GREATER, GL_NOTEQUAL, GL_GEQUAL
glDepthMask(GL_TRUE);  // 启用深度写入

// 模板测试
glEnable(GL_STENCIL_TEST);
glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
glStencilMask(0xFF);

// 混合
glEnable(GL_BLEND);
glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
glBlendEquation(GL_FUNC_ADD);

// 面剔除
glEnable(GL_CULL_FACE);
glCullFace(GL_BACK);  // GL_FRONT, GL_BACK, GL_FRONT_AND_BACK
glFrontFace(GL_CCW);  // GL_CCW (逆时针), GL_CW (顺时针)

// 视口
glViewport(0, 0, width, height);

// 裁剪测试
glEnable(GL_SCISSOR_TEST);
glScissor(x, y, width, height);
```

## 🎨 GLSL 着色器速查

### 数据类型

```glsl
// 基本类型
int, uint, float, double, bool

// 向量
vec2, vec3, vec4      // float向量
ivec2, ivec3, ivec4   // int向量
uvec2, uvec3, uvec4   // uint向量
bvec2, bvec3, bvec4   // bool向量
dvec2, dvec3, dvec4   // double向量

// 矩阵
mat2, mat3, mat4      // 方阵
mat2x3, mat3x2        // 非方阵

// 采样器
sampler2D, sampler3D, samplerCube
sampler2DShadow, samplerCubeShadow
```

### 内置变量

```glsl
// 顶点着色器
in int gl_VertexID;
in int gl_InstanceID;
out vec4 gl_Position;
out float gl_PointSize;

// 几何着色器
in gl_PerVertex {
    vec4 gl_Position;
    float gl_PointSize;
} gl_in[];

// 片段着色器
in vec4 gl_FragCoord;
in bool gl_FrontFacing;
in vec2 gl_PointCoord;
out float gl_FragDepth;
```

### 限定符

```glsl
// 存储限定符
in        // 输入
out       // 输出
uniform   // 统一变量
const     // 常量

// 精度限定符
highp     // 高精度
mediump   // 中精度
lowp      // 低精度

// 插值限定符
flat      // 无插值
smooth    // 平滑插值（默认）
noperspective  // 线性插值

// 布局限定符
layout(location = 0) in vec3 aPos;
layout(location = 0) out vec4 FragColor;
layout(std140) uniform Matrices { ... };
```

### 内置函数

```glsl
// 数学函数
abs(x), sign(x), floor(x), ceil(x), fract(x)
mod(x, y), min(x, y), max(x, y), clamp(x, min, max)
mix(x, y, a), step(edge, x), smoothstep(edge0, edge1, x)

// 三角函数
sin(x), cos(x), tan(x)
asin(x), acos(x), atan(y, x)
radians(degrees), degrees(radians)

// 指数函数
pow(x, y), exp(x), log(x), exp2(x), log2(x)
sqrt(x), inversesqrt(x)

// 向量函数
length(v), distance(v1, v2), dot(v1, v2), cross(v1, v2)
normalize(v), reflect(I, N), refract(I, N, eta)

// 矩阵函数
matrixCompMult(m1, m2), transpose(m), determinant(m), inverse(m)

// 纹理函数
texture(sampler, coord)
texelFetch(sampler, ivec2, lod)
textureSize(sampler, lod)
```

### 常用片段着色器模板

```glsl
#version 460 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform sampler2D texture_diffuse1;
uniform vec3 lightPos;
uniform vec3 viewPos;

void main() {
    // 环境光
    vec3 ambient = 0.1 * texture(texture_diffuse1, TexCoords).rgb;

    // 漫反射
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * texture(texture_diffuse1, TexCoords).rgb;

    // 镜面反射
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specular = 0.5 * spec * vec3(1.0);

    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}
```

## 📐 GLM 数学库速查

### 向量操作

```cpp
#include <glm/glm.hpp>

// 创建向量
glm::vec3 v1(1.0f, 2.0f, 3.0f);
glm::vec3 v2 = glm::vec3(0.0f);

// 向量运算
glm::vec3 sum = v1 + v2;
glm::vec3 diff = v1 - v2;
glm::vec3 scaled = v1 * 2.0f;
float dotProduct = glm::dot(v1, v2);
glm::vec3 crossProduct = glm::cross(v1, v2);

// 向量函数
float len = glm::length(v1);
glm::vec3 normalized = glm::normalize(v1);
float dist = glm::distance(v1, v2);
```

### 矩阵操作

```cpp
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// 创建矩阵
glm::mat4 model = glm::mat4(1.0f);  // 单位矩阵

// 变换
model = glm::translate(model, glm::vec3(1.0f, 2.0f, 3.0f));
model = glm::rotate(model, glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
model = glm::scale(model, glm::vec3(2.0f, 2.0f, 2.0f));

// 视图矩阵
glm::mat4 view = glm::lookAt(
    glm::vec3(0.0f, 0.0f, 3.0f),  // 相机位置
    glm::vec3(0.0f, 0.0f, 0.0f),  // 目标位置
    glm::vec3(0.0f, 1.0f, 0.0f)   // 上向量
);

// 投影矩阵
glm::mat4 projection = glm::perspective(
    glm::radians(45.0f),  // FOV
    800.0f / 600.0f,      // 宽高比
    0.1f,                 // 近平面
    100.0f                // 远平面
);

glm::mat4 ortho = glm::ortho(
    0.0f, 800.0f,   // 左右
    0.0f, 600.0f,   // 下上
    0.1f, 100.0f    // 近远
);

// 获取指针（传递给OpenGL）
glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(model));

// 矩阵运算
glm::mat4 result = projection * view * model;
glm::mat4 inverted = glm::inverse(model);
glm::mat4 transposed = glm::transpose(model);
```

## 🔧 常见错误码

```cpp
GL_NO_ERROR                      0x0000  // 无错误
GL_INVALID_ENUM                  0x0500  // 无效枚举
GL_INVALID_VALUE                 0x0501  // 无效值
GL_INVALID_OPERATION             0x0502  // 无效操作
GL_STACK_OVERFLOW                0x0503  // 栈溢出
GL_STACK_UNDERFLOW               0x0504  // 栈下溢
GL_OUT_OF_MEMORY                 0x0505  // 内存不足
GL_INVALID_FRAMEBUFFER_OPERATION 0x0506  // 帧缓冲操作无效

// 错误检查
GLenum err;
while ((err = glGetError()) != GL_NO_ERROR) {
    std::cerr << "OpenGL error: " << err << std::endl;
}
```

## 📊 性能优化检查清单

```
□ 使用VAO减少状态切换
□ 批处理相同材质的物体
□ 使用实例化渲染大量相同物体
□ 启用面剔除
□ 使用视锥剔除
□ 实现LOD系统
□ 压缩纹理（DXT、ETC）
□ 使用mipmaps
□ 避免频繁的glUniform调用
□ 使用UBO共享数据
□ 合理使用深度测试
□ 减少overdraw
□ 优化着色器代码
□ 避免在着色器中使用分支
□ 使用纹理图集
```

## 🎯 调试技巧

### 基础调试

```cpp
// 1. 启用调试输出
void GLAPIENTRY MessageCallback(GLenum source, GLenum type, GLuint id,
                                 GLenum severity, GLsizei length,
                                 const GLchar* message, const void* userParam) {
    fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
            (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
            type, severity, message);
}

glEnable(GL_DEBUG_OUTPUT);
glDebugMessageCallback(MessageCallback, 0);

// 2. 标记调试区域
glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Scene Rendering");
// 渲染代码
glPopDebugGroup();

// 3. 对象标签
glObjectLabel(GL_BUFFER, VBO, -1, "Vertex Buffer");
glObjectLabel(GL_TEXTURE, texture, -1, "Diffuse Texture");
```

### 着色器输出可视化

```glsl
// 可视化法线
FragColor = vec4(Normal * 0.5 + 0.5, 1.0);

// 可视化深度
float depth = gl_FragCoord.z;
FragColor = vec4(vec3(depth), 1.0);

// 可视化UV
FragColor = vec4(TexCoords, 0.0, 1.0);
```

## 📱 移动端特殊考虑

```cpp
// OpenGL ES版本
#ifdef __ANDROID__
    #define GLSL_VERSION "#version 300 es\n"
#else
    #define GLSL_VERSION "#version 460 core\n"
#endif

// 精度限定符（ES必需）
#ifdef GL_ES
precision mediump float;
#endif

// 扩展检查
if (GLAD_GL_ARB_texture_compression) {
    // 使用纹理压缩
}
```

## 🔗 有用的宏定义

```cpp
#define GL_CHECK(stmt) do { \
    stmt; \
    GLenum err = glGetError(); \
    if (err != GL_NO_ERROR) { \
        fprintf(stderr, "OpenGL error %d at %s:%d\n", err, __FILE__, __LINE__); \
    } \
} while (0)

#define ASSERT_GL_ERROR() do { \
    GLenum err = glGetError(); \
    assert(err == GL_NO_ERROR); \
} while (0)

#ifdef _DEBUG
    #define GL_CALL(x) GL_CHECK(x)
#else
    #define GL_CALL(x) x
#endif
```

---

**本手册持续更新中，欢迎补充！** 📚
