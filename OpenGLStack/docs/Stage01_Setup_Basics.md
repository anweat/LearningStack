# 阶段 1: 环境搭建与 OpenGL 基础

## 🎯 学习目标

- 搭建完整的 OpenGL 开发环境
- 理解 OpenGL 渲染管线
- 创建第一个 OpenGL 窗口
- 绘制基本图形（三角形、矩形）
- 掌握 OpenGL 的状态机制

## 📋 框架性问题指引

### 核心问题 1: OpenGL 是什么？

**问题分解:**

- OpenGL 的作用和定位是什么？
- OpenGL 与 DirectX、Vulkan 的区别？
- 现代 OpenGL（Core Profile）vs 传统 OpenGL（Immediate Mode）？

**答案要点:**

- OpenGL 是跨平台的图形 API 规范
- 采用客户端-服务端模型（CPU-GPU）
- 现代 OpenGL 强制使用着色器，更接近硬件

### 核心问题 2: 渲染管线是如何工作的？

**问题分解:**

- 顶点着色器的作用是什么？
- 图元装配和光栅化做了什么？
- 片段着色器处理什么数据？

**关键流程:**

```
顶点数据 → 顶点着色器 → 图元装配 → 光栅化 → 片段着色器 → 测试混合 → 帧缓冲
```

### 核心问题 3: 如何创建 OpenGL 上下文？

**问题分解:**

- 为什么需要 GLFW？
- GLAD 的作用是什么？
- 上下文创建的完整流程？

## 💻 环境搭建

### 1. 安装依赖库

#### 使用 vcpkg（推荐）

```bash
# 安装vcpkg
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
bootstrap-vcpkg.bat

# 安装OpenGL相关库
vcpkg install glfw3:x64-windows
vcpkg install glad:x64-windows
vcpkg install glm:x64-windows
```

#### CMakeLists.txt 配置

```cmake
cmake_minimum_required(VERSION 3.15)
project(OpenGLLearning)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置vcpkg工具链
set(CMAKE_TOOLCHAIN_FILE "C:/vcpkg/scripts/buildsystems/vcpkg.cmake")

find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

add_executable(HelloTriangle src/01_hello_triangle.cpp)
target_link_libraries(HelloTriangle PRIVATE glfw glad::glad glm::glm)
```

### 2. 验证环境

```cpp
// test_environment.cpp
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

int main() {
    if (!glfwInit()) {
        std::cout << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(800, 600, "Test", nullptr, nullptr);
    if (!window) {
        std::cout << "Failed to create window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "Environment setup successful!" << std::endl;

    glfwTerminate();
    return 0;
}
```

## 📚 核心概念详解

### 1. OpenGL 状态机

OpenGL 是一个巨大的状态机，通过设置各种状态来控制渲染行为。

```cpp
// 状态机示例
glEnable(GL_DEPTH_TEST);     // 启用深度测试
glDepthFunc(GL_LESS);         // 设置深度测试函数
glClearColor(0.2f, 0.3f, 0.3f, 1.0f);  // 设置清屏颜色

// 这些状态会一直保持，直到被改变
while (!glfwWindowShouldClose(window)) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // 渲染代码...
}
```

### 2. 顶点缓冲对象（VBO）

VBO 用于存储顶点数据，是 GPU 内存中的缓冲区。

```cpp
// 顶点数据（3D坐标）
float vertices[] = {
    -0.5f, -0.5f, 0.0f,  // 左下
     0.5f, -0.5f, 0.0f,  // 右下
     0.0f,  0.5f, 0.0f   // 顶部
};

unsigned int VBO;
glGenBuffers(1, &VBO);                    // 生成缓冲对象
glBindBuffer(GL_ARRAY_BUFFER, VBO);       // 绑定为顶点缓冲
glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
```

**问题: GL_STATIC_DRAW 是什么意思？**

- `GL_STATIC_DRAW`: 数据不会改变，适合静态模型
- `GL_DYNAMIC_DRAW`: 数据会频繁改变
- `GL_STREAM_DRAW`: 数据每次绘制都会改变

### 3. 顶点数组对象（VAO）

VAO 存储顶点属性配置，简化顶点数据的管理。

```cpp
unsigned int VAO;
glGenVertexArrays(1, &VAO);
glBindVertexArray(VAO);

// 配置顶点属性
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
glEnableVertexAttribArray(0);

// 参数解释:
// 0: 属性位置（layout location = 0）
// 3: 每个顶点有3个分量（x, y, z）
// GL_FLOAT: 数据类型
// GL_FALSE: 不归一化
// 3 * sizeof(float): 步长（下一个顶点数据的间隔）
// (void*)0: 偏移量
```

### 4. 着色器基础

#### 顶点着色器（vertex.glsl）

```glsl
#version 460 core
layout (location = 0) in vec3 aPos;  // 顶点位置属性

void main() {
    gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
}
```

#### 片段着色器（fragment.glsl）

```glsl
#version 460 core
out vec4 FragColor;  // 输出颜色

void main() {
    FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);  // 橙色
}
```

#### 着色器编译与链接

```cpp
class Shader {
public:
    unsigned int ID;  // 着色器程序ID

    Shader(const char* vertexPath, const char* fragmentPath) {
        // 1. 读取着色器源代码
        std::string vertexCode = readFile(vertexPath);
        std::string fragmentCode = readFile(fragmentPath);
        const char* vShaderCode = vertexCode.c_str();
        const char* fShaderCode = fragmentCode.c_str();

        // 2. 编译着色器
        unsigned int vertex = compileShader(GL_VERTEX_SHADER, vShaderCode);
        unsigned int fragment = compileShader(GL_FRAGMENT_SHADER, fShaderCode);

        // 3. 链接着色器程序
        ID = glCreateProgram();
        glAttachShader(ID, vertex);
        glAttachShader(ID, fragment);
        glLinkProgram(ID);
        checkLinkErrors(ID);

        // 4. 删除着色器对象
        glDeleteShader(vertex);
        glDeleteShader(fragment);
    }

    void use() {
        glUseProgram(ID);
    }

private:
    unsigned int compileShader(GLenum type, const char* source) {
        unsigned int shader = glCreateShader(type);
        glShaderSource(shader, 1, &source, nullptr);
        glCompileShader(shader);
        checkCompileErrors(shader, type);
        return shader;
    }

    void checkCompileErrors(unsigned int shader, GLenum type) {
        int success;
        char infoLog[512];
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 512, nullptr, infoLog);
            std::cout << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
        }
    }
};
```

## 🎨 实践项目：Hello Triangle

### 完整代码实现

```cpp
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

// 顶点着色器源码
const char* vertexShaderSource = R"(
#version 460 core
layout (location = 0) in vec3 aPos;
void main() {
    gl_Position = vec4(aPos, 1.0);
}
)";

// 片段着色器源码
const char* fragmentShaderSource = R"(
#version 460 core
out vec4 FragColor;
void main() {
    FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
}
)";

// 窗口大小改变回调
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// 处理输入
void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

int main() {
    // 1. 初始化GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 2. 创建窗口
    GLFWwindow* window = glfwCreateWindow(800, 600, "Hello Triangle", nullptr, nullptr);
    if (!window) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // 3. 加载OpenGL函数
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // 4. 编译着色器
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // 5. 准备顶点数据
    float vertices[] = {
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
         0.0f,  0.5f, 0.0f
    };

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // 6. 渲染循环
    while (!glfwWindowShouldClose(window)) {
        processInput(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // 7. 清理资源
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}
```

### 扩展练习：绘制矩形

**问题: 如何用两个三角形绘制矩形？**

```cpp
// 方法1: 使用索引缓冲（EBO）
float vertices[] = {
     0.5f,  0.5f, 0.0f,  // 右上
     0.5f, -0.5f, 0.0f,  // 右下
    -0.5f, -0.5f, 0.0f,  // 左下
    -0.5f,  0.5f, 0.0f   // 左上
};

unsigned int indices[] = {
    0, 1, 3,  // 第一个三角形
    1, 2, 3   // 第二个三角形
};

unsigned int EBO;
glGenBuffers(1, &EBO);
glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

// 渲染
glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
```

## 🔍 调试技巧

### 1. OpenGL 错误检查

```cpp
void checkGLError(const char* stmt, const char* file, int line) {
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        std::cerr << "OpenGL error " << err << " at " << file << ":" << line
                  << " - for " << stmt << std::endl;
    }
}

#define GL_CHECK(stmt) do { \
    stmt; \
    checkGLError(#stmt, __FILE__, __LINE__); \
} while (0)

// 使用
GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, 3));
```

### 2. 着色器编译错误检查

```cpp
void checkShaderCompilation(unsigned int shader) {
    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cout << "Shader compilation failed:\n" << infoLog << std::endl;
    }
}
```

## 💡 常见问题解答

### Q1: 为什么窗口是黑屏？

**可能原因:**

1. 忘记调用 `glClear()`
2. 着色器编译失败
3. 顶点数据错误
4. 没有绑定 VAO

### Q2: 什么是 NDC（标准化设备坐标）？

**答:**

- NDC 范围是 [-1, 1]
- 超出范围的顶点会被裁剪
- 顶点着色器输出的坐标会自动转换到 NDC

### Q3: Core Profile vs Compatibility Profile？

**答:**

- Core Profile: 移除了所有废弃功能，强制现代方式
- Compatibility Profile: 保留旧 API，但不推荐

## 📝 学习检查清单

- [ ] 成功搭建 OpenGL 开发环境
- [ ] 理解渲染管线的各个阶段
- [ ] 能够编译和链接着色器
- [ ] 理解 VBO、VAO、EBO 的作用
- [ ] 成功绘制三角形和矩形
- [ ] 掌握基本的调试方法

## 🚀 下一步

完成本阶段后，进入 [阶段 2: 渲染基础](./Stage02_Rendering_Basics.md)，学习：

- 着色器数据传递（Uniform）
- 纹理映射
- 变换矩阵（模型、视图、投影）
- 3D 场景构建

---

**记住:** OpenGL 学习重在理解概念，不要死记 API！ 💪
