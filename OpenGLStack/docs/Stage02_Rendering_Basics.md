# 阶段 2: 渲染基础

## 🎯 学习目标

- 掌握着色器数据传递（Uniform、Attribute）
- 实现纹理映射和多重纹理
- 理解并应用坐标变换（MVP 矩阵）
- 构建基本的 3D 场景
- 实现摄像机系统

## 📋 框架性问题指引

### 核心问题 1: 如何向着色器传递数据？

**问题分解:**

- Attribute vs Uniform 的区别？
- Uniform 的生命周期是什么？
- 如何传递复杂数据类型（矩阵、结构体）？

**答案要点:**

- **Attribute**: 每个顶点不同（位置、颜色、纹理坐标）
- **Uniform**: 所有顶点共享（变换矩阵、光照参数）
- **Uniform Buffer Object**: 多着色器共享数据

### 核心问题 2: 纹理是如何映射到模型上的？

**问题分解:**

- 纹理坐标系统是什么样的？
- 纹理过滤和环绕模式有什么作用？
- Mipmap 解决什么问题？

### 核心问题 3: MVP 矩阵如何工作？

**问题分解:**

- Model 矩阵做什么变换？
- View 矩阵如何构建？
- Projection 矩阵的作用是什么？

**变换流程:**

```
局部空间 --[Model]--> 世界空间 --[View]--> 观察空间 --[Projection]--> 裁剪空间
```

## 💻 核心概念详解

### 1. Uniform 变量

Uniform 是从 CPU 传递到 GPU 的全局变量，在一次绘制调用中保持不变。

```cpp
// C++端设置Uniform
class Shader {
public:
    void setBool(const std::string& name, bool value) const {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
    }

    void setInt(const std::string& name, int value) const {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
    }

    void setFloat(const std::string& name, float value) const {
        glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
    }

    void setVec3(const std::string& name, const glm::vec3& value) const {
        glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
    }

    void setMat4(const std::string& name, const glm::mat4& mat) const {
        glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }
};

// 使用示例
shader.use();
shader.setFloat("ourAlpha", 0.5f);
shader.setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
```

```glsl
// 着色器端接收Uniform
#version 460 core
uniform float ourAlpha;
uniform vec3 lightColor;
uniform mat4 transform;

void main() {
    // 使用uniform变量
}
```

### 2. 纹理映射

#### 纹理加载（使用 stb_image）

```cpp
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

unsigned int loadTexture(const char* path) {
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrChannels;
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);

    if (data) {
        GLenum format;
        if (nrChannels == 1)
            format = GL_RED;
        else if (nrChannels == 3)
            format = GL_RGB;
        else if (nrChannels == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        // 设置纹理参数
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    } else {
        std::cout << "Failed to load texture" << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}
```

#### 纹理参数详解

**环绕模式（Wrapping）:**

```cpp
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);      // 重复
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // 边缘拉伸
// GL_MIRRORED_REPEAT: 镜像重复
// GL_CLAMP_TO_BORDER: 超出部分为边框颜色
```

**过滤模式（Filtering）:**

```cpp
// 放大过滤
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);  // 线性插值
// GL_NEAREST: 最近邻（像素化效果）

// 缩小过滤
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
// GL_NEAREST_MIPMAP_NEAREST: 使用最近的mipmap
// GL_LINEAR_MIPMAP_LINEAR: 三线性过滤（最佳质量）
```

#### 多重纹理

```cpp
// 顶点数据（包含纹理坐标）
float vertices[] = {
    // 位置            // 纹理坐标
    -0.5f, -0.5f, 0.0f,  0.0f, 0.0f,
     0.5f, -0.5f, 0.0f,  1.0f, 0.0f,
     0.5f,  0.5f, 0.0f,  1.0f, 1.0f,
    -0.5f,  0.5f, 0.0f,  0.0f, 1.0f
};

// 配置顶点属性
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
glEnableVertexAttribArray(0);
glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
glEnableVertexAttribArray(1);

// 激活并绑定纹理单元
glActiveTexture(GL_TEXTURE0);
glBindTexture(GL_TEXTURE_2D, texture1);
glActiveTexture(GL_TEXTURE1);
glBindTexture(GL_TEXTURE_2D, texture2);

shader.use();
shader.setInt("texture1", 0);  // 对应GL_TEXTURE0
shader.setInt("texture2", 1);  // 对应GL_TEXTURE1
```

```glsl
// 顶点着色器
#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

void main() {
    gl_Position = vec4(aPos, 1.0);
    TexCoord = aTexCoord;
}

// 片段着色器
#version 460 core
out vec4 FragColor;
in vec2 TexCoord;

uniform sampler2D texture1;
uniform sampler2D texture2;

void main() {
    FragColor = mix(texture(texture1, TexCoord), texture(texture2, TexCoord), 0.2);
}
```

### 3. 坐标变换与 MVP 矩阵

#### GLM 数学库使用

```cpp
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// 创建变换矩阵
glm::mat4 trans = glm::mat4(1.0f);  // 单位矩阵
trans = glm::translate(trans, glm::vec3(1.0f, 1.0f, 0.0f));  // 平移
trans = glm::rotate(trans, glm::radians(90.0f), glm::vec3(0.0, 0.0, 1.0));  // 旋转
trans = glm::scale(trans, glm::vec3(0.5f, 0.5f, 0.5f));  // 缩放

// 传递给着色器
shader.setMat4("transform", trans);
```

#### MVP 矩阵详解

**Model 矩阵（模型变换）:**

```cpp
// 将物体从局部空间变换到世界空间
glm::mat4 model = glm::mat4(1.0f);
model = glm::rotate(model, glm::radians(-55.0f), glm::vec3(1.0f, 0.0f, 0.0f));
model = glm::translate(model, glm::vec3(0.0f, 0.0f, -3.0f));
```

**View 矩阵（观察变换）:**

```cpp
// 将世界空间变换到观察空间（摄像机视角）
glm::mat4 view = glm::mat4(1.0f);
// 方法1: 简单移动
view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));

// 方法2: LookAt矩阵（推荐）
glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f, 0.0f);
view = glm::lookAt(cameraPos, cameraTarget, cameraUp);
```

**Projection 矩阵（投影变换）:**

```cpp
// 方法1: 透视投影（3D场景）
glm::mat4 projection = glm::perspective(
    glm::radians(45.0f),  // FOV（视野）
    800.0f / 600.0f,       // 宽高比
    0.1f,                  // 近平面
    100.0f                 // 远平面
);

// 方法2: 正交投影（2D场景或UI）
glm::mat4 projection = glm::ortho(
    0.0f, 800.0f,   // 左右边界
    0.0f, 600.0f,   // 底部顶部边界
    0.1f, 100.0f    // 近远平面
);
```

#### 完整 MVP 示例

```glsl
// 顶点着色器
#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    TexCoord = aTexCoord;
}
```

```cpp
// C++端设置矩阵
shader.use();

glm::mat4 model = glm::rotate(glm::mat4(1.0f), (float)glfwGetTime(), glm::vec3(0.5f, 1.0f, 0.0f));
glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.0f));
glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

shader.setMat4("model", model);
shader.setMat4("view", view);
shader.setMat4("projection", projection);
```

### 4. 摄像机系统

#### 基础摄像机类

```cpp
class Camera {
public:
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;

    float Yaw;    // 偏航角
    float Pitch;  // 俯仰角

    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;

    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
           glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
           float yaw = -90.0f, float pitch = 0.0f)
        : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(2.5f),
          MouseSensitivity(0.1f), Zoom(45.0f) {
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }

    glm::mat4 GetViewMatrix() {
        return glm::lookAt(Position, Position + Front, Up);
    }

    void ProcessKeyboard(int direction, float deltaTime) {
        float velocity = MovementSpeed * deltaTime;
        if (direction == 0)  // FORWARD
            Position += Front * velocity;
        if (direction == 1)  // BACKWARD
            Position -= Front * velocity;
        if (direction == 2)  // LEFT
            Position -= Right * velocity;
        if (direction == 3)  // RIGHT
            Position += Right * velocity;
    }

    void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true) {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        Yaw += xoffset;
        Pitch += yoffset;

        if (constrainPitch) {
            if (Pitch > 89.0f)
                Pitch = 89.0f;
            if (Pitch < -89.0f)
                Pitch = -89.0f;
        }

        updateCameraVectors();
    }

    void ProcessMouseScroll(float yoffset) {
        Zoom -= yoffset;
        if (Zoom < 1.0f)
            Zoom = 1.0f;
        if (Zoom > 45.0f)
            Zoom = 45.0f;
    }

private:
    void updateCameraVectors() {
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);

        Right = glm::normalize(glm::cross(Front, WorldUp));
        Up = glm::normalize(glm::cross(Right, Front));
    }
};
```

#### 输入处理

```cpp
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = 400, lastY = 300;
bool firstMouse = true;
float deltaTime = 0.0f;
float lastFrame = 0.0f;

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;  // Y坐标是反的
    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll(yoffset);
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(0, deltaTime);  // FORWARD
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(1, deltaTime);  // BACKWARD
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(2, deltaTime);  // LEFT
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(3, deltaTime);  // RIGHT
}

// 主循环
while (!glfwWindowShouldClose(window)) {
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    processInput(window);

    // 渲染
    shader.use();
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), 800.0f / 600.0f, 0.1f, 100.0f);
    shader.setMat4("view", view);
    shader.setMat4("projection", projection);

    // ... 绘制代码
}
```

## 🎨 实践项目：旋转的 3D 立方体

### 完整实现

```cpp
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shader.h"
#include "camera.h"
#include <iostream>

// 立方体顶点数据（位置 + 纹理坐标）
float vertices[] = {
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
};

// 多个立方体位置
glm::vec3 cubePositions[] = {
    glm::vec3( 0.0f,  0.0f,  0.0f),
    glm::vec3( 2.0f,  5.0f, -15.0f),
    glm::vec3(-1.5f, -2.2f, -2.5f),
    glm::vec3(-3.8f, -2.0f, -12.3f),
    glm::vec3( 2.4f, -0.4f, -3.5f),
    glm::vec3(-1.7f,  3.0f, -7.5f),
    glm::vec3( 1.3f, -2.0f, -2.5f),
    glm::vec3( 1.5f,  2.0f, -2.5f),
    glm::vec3( 1.5f,  0.2f, -1.5f),
    glm::vec3(-1.3f,  1.0f, -1.5f)
};

int main() {
    // 初始化和窗口创建...

    // 启用深度测试
    glEnable(GL_DEPTH_TEST);

    // 创建VAO、VBO
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // 加载纹理和着色器...

    // 渲染循环
    while (!glfwWindowShouldClose(window)) {
        // 时间计算...
        processInput(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();

        // 创建变换矩阵
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), 800.0f / 600.0f, 0.1f, 100.0f);
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);

        glBindVertexArray(VAO);
        for (unsigned int i = 0; i < 10; i++) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, cubePositions[i]);
            float angle = 20.0f * i + (float)glfwGetTime() * 25.0f;
            model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
            shader.setMat4("model", model);

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // 清理...
    return 0;
}
```

## 💡 常见问题解答

### Q1: 为什么启用深度测试？

**答:** 深度测试确保离摄像机近的物体遮挡远处的物体。不启用会导致渲染顺序错误。

```cpp
glEnable(GL_DEPTH_TEST);
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  // 清除深度缓冲
```

### Q2: 透视投影 vs 正交投影的应用场景？

- **透视投影**: 3D 场景，物体有近大远小的效果
- **正交投影**: 2D UI、工程制图、等距游戏

### Q3: 变换矩阵的顺序为什么重要？

**答:** 矩阵乘法不满足交换律。正确顺序：先缩放 → 再旋转 → 最后平移

```cpp
glm::mat4 transform = glm::mat4(1.0f);
transform = glm::translate(transform, position);  // 3. 平移
transform = glm::rotate(transform, angle, axis);  // 2. 旋转
transform = glm::scale(transform, scale);         // 1. 缩放
```

## 📝 学习检查清单

- [ ] 理解 Uniform 和 Attribute 的区别
- [ ] 能够加载和应用纹理
- [ ] 掌握纹理过滤和环绕模式
- [ ] 理解 MVP 矩阵的作用
- [ ] 实现可交互的摄像机系统
- [ ] 成功渲染 3D 立方体场景
- [ ] 理解深度测试的必要性

## 🚀 下一步

完成本阶段后，进入 [阶段 3: 高级渲染技术](./Stage03_Advanced_Rendering.md)，学习：

- 冯氏光照模型（环境光、漫反射、镜面反射）
- 多光源系统
- 模型加载（Assimp）
- 面剔除和深度测试优化

---

**继续加油！** 🚀
